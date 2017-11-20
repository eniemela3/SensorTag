/*
 *  ======== main.c ========
 */
 #include <stdio.h>

// XDCtools Header files
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>

// TI-RTOS Header files
#include <ti/drivers/I2C.h>
#include <ti/drivers/i2c/I2CCC26XX.h>
#include <ti/drivers/PIN.h>
#include <ti/drivers/pin/PINCC26XX.h>
#include <ti/mw/display/Display.h>
#include <ti/mw/display/DisplayExt.h>

// Board Header files
#include "Board.h"

// jtkj Header files
#include "wireless/comm_lib.h"
#include "sensors/bmp280.h"
#include "sensors/mpu9250.h"

#include <inttypes.h>

// Obstacle graphics
#include <graphics.h>

// Functions used by displayTask
#include <display_functions.h>

// Functions used to run the game
//#include <game.h>

#define STACKSIZE_commTask 2048
Char commTaskStack[STACKSIZE_commTask];

#define STACKSIZE_displayTask 2048
Char displayTaskStack[STACKSIZE_displayTask];

#define STACKSIZE_MPU9250Task 4096
Char MPU9250TaskStack[STACKSIZE_MPU9250Task];








//// Coordinates for drawing rectangle
//struct rect {
//   struct point max;
//   struct point min;
//} displayRect;



// DIY boolean
enum diyBoolean {BOOLEAN_0=0, BOOLEAN_1};
enum diyBoolean button0AllowExec = BOOLEAN_1;

// Main state machine


// Clock handle
Clock_Handle clockHandle;



// MPU global variables
static PIN_Handle hMpuPin;
static PIN_State MpuPinState;
static PIN_Config MpuPinConfig[] = {
    Board_MPU_POWER  | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};


// MPU9250 uses its own I2C interface
static const I2CCC26XX_I2CPinCfg i2cMPUCfg = {
    .pinSDA = Board_I2C0_SDA1,
    .pinSCL = Board_I2C0_SCL1
};

// Global display handle
Display_Handle hDisplay;

// Pin Button1 configured as power button
static PIN_Handle hPowerButton;
static PIN_State sPowerButton;
PIN_Config cPowerButton[] = {
    Board_BUTTON1 | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
    PIN_TERMINATE
};
PIN_Config cPowerWake[] = {
    Board_BUTTON1 | PIN_INPUT_EN | PIN_PULLUP | PINCC26XX_WAKEUP_NEGEDGE,
    PIN_TERMINATE
};

// Pin Button0 configured as input
static PIN_Handle hButton0;
static PIN_State sButton0;
PIN_Config cButton0[] = {
    Board_BUTTON0 | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
    PIN_TERMINATE
};
PIN_Config buttonConfig[] = {
    Board_BUTTON0 | PIN_INPUT_EN | PIN_PULLUP | PINCC26XX_WAKEUP_NEGEDGE,
    PIN_TERMINATE
};

// Leds configuraion
static PIN_Handle hLed;
static PIN_State sLed;
PIN_Config cLed[] = {
    Board_LED0 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    Board_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

Void debounceTimer(UArg arg0) {
    button0AllowExec = BOOLEAN_1;
}

Clock_Handle createTimer(uint8_t period, Clock_FuncPtr clkFxn) {
    // Period in tenths of a second

    // RTOS:n kellomuuttujat
    Clock_Handle clkHandle;
    Clock_Params clkParams;

    // Alustetaan kello halutusti
    Clock_Params_init(&clkParams);
    clkParams.period = 0; // To launch only when called
    clkParams.startFlag = FALSE;

    // Luodaan kello
    clkHandle = Clock_create((Clock_FuncPtr)clkFxn, 100000 * period / Clock_tickPeriod, &clkParams, NULL);
    if (clkHandle == NULL) {
    System_abort("Clock creat failed");
    }
//    return clkParams;
    return clkHandle;
}

// Button1 (bottom) button press handler
Void powerButtonFxn(PIN_Handle handle, PIN_Id pinId) {
    Display_clear(hDisplay);
    Display_close(hDisplay);
    Task_sleep(100000 / Clock_tickPeriod);

	PIN_close(hPowerButton);

    PINCC26XX_setWakeup(cPowerWake);
	Power_shutdown(NULL,0);
}

// Button0 (top) button press handler
Void Button0Fxn(PIN_Handle handle, PIN_Id pinId) {
	if (button0AllowExec == BOOLEAN_1) { // For debounce
		button0AllowExec = BOOLEAN_0;
	    PIN_setOutputValue(hLed, Board_LED0, !PIN_getOutputValue(Board_LED0) );
	    switch (myState) {
	    case MENU:
	    	switch (cursorPos) {
	    	case C_NEW_GAME:
	    		myState = GAME;
	    		break;
	    	case C_CALIBRATION:
	    		myState = CALIBRATE1;
	    		break;
	    	case C_HIGH_SCORES:
	    		myState = HIGHSCORES;
	    		break;
	    	}
	    	break;
	    case CALIBRATE1:
	    	myState = CALIBRATE2;
	    	break;
	    case CALIBRATE2:
	    	myState = MENU;
	    	break;
	    case GAME:
	    	myState = MENU;
	    	break;
	    case HIGHSCORES:
	    	myState = MENU;
	    	break;
	    default:
	    	myState = MENU;
	    	break;
	    }
	    Clock_start(clockHandle);
//	    button0AllowExec = BOOLEAN_1; // This should be done somewhere else.
	}
}

// Communication Task
Void commTask(UArg arg0, UArg arg1) {
	while ((myState == STARTUP)) {
		// prevent data receive during startup or sensor calibration (Task_sleep breaks commTask functionality!)
//		Task_sleep(100000 / Clock_tickPeriod);
	}
	enum diyBoolean bufferZeroed = BOOLEAN_0;
    char payload[16];
    uint16_t SenderAddr;
    // Radio to receive mode
	int32_t result = StartReceive6LoWPAN();
	if(result != true) {
		System_abort("Wireless receive mode failed");
	}
	int i; // General for-variable
    while (1) {
        if (GetRXFlag() == true) {
            memset(payload, 0, 16);
            Receive6LoWPAN(&SenderAddr, payload, 16);
            if (myState == GAME) {
            	for (i = 0; i < 8; i++) {
            		message[i] = payload[i+1];
            	}
            	for (i = 4; i > 0; i--) {
            		trackBuffer[i] = trackBuffer[i-1];
            	}
            	trackBuffer[0] = payload[0];
                newTrackAvailable = 1;
                gameScore++;
//                System_printf("ok\n");
            } else if (myState == MENU) {
            	if (!bufferZeroed) {
            		for (i = 4; i > 0; i--) {
            			trackBuffer[i] = 0;
            		}
            		bufferZeroed = BOOLEAN_1;
            	}
            }
        } // No sleeps because of lowest priority
    }
}




// Acceleration/gyro sensor task
Void MPU9250Task(UArg arg0, UArg arg1) {
	// Following part is critical and other tasks wait for myState to change:
	I2C_Handle      i2cMPU;
	I2C_Params      i2cMPUParams;

	I2C_Params_init(&i2cMPUParams);
	i2cMPUParams.bitRate = I2C_400kHz;
	i2cMPUParams.custom = (uintptr_t)&i2cMPUCfg;
	i2cMPU = I2C_open(Board_I2C, &i2cMPUParams);
	if (i2cMPU == NULL) {
		System_abort("Error Initializing I2CMPU\n");
	}
	PIN_setOutputValue(hMpuPin,Board_MPU_POWER, Board_MPU_POWER_ON);
	Task_sleep(100000 / Clock_tickPeriod);
	System_printf("MPU9250: Power ON\n");
	System_flush();
	mpu9250_setup(&i2cMPU);
	// Critical ends

	if (myState == STARTUP) {
		myState = MENU;
	} else {
		System_abort("MPU9250Task found erroneous myState, aborting\n");
	}

	// Creating calWait variable to ensure calibration task waits for sensor setup
	uint8_t calWait = 1; // How to make a boolean??
//	uint8_t calIndex = 0;
    float maxX = 0;
    float minX = 0;
    float minZ = 0;
	while (1) {
		switch (myState) {
		case MENU:
			mpu9250_get_data(&i2cMPU, &ax, &ay, &az, &gx, &gy, &gz);
			if (ay - ay_off< ACC_LO_THERSHOLD) {
				cursorPos = C_NEW_GAME;
			} else if (ay - ay_off > ACC_HI_THERSHOLD) {
				cursorPos = C_HIGH_SCORES;
			} else {
				cursorPos = C_CALIBRATION;
			}
			calWait = 1;
			break;
		case GAME:
			mpu9250_get_data(&i2cMPU, &ax, &ay, &az, &gx, &gy, &gz);
			moveBall();
			Task_sleep(100000 / Clock_tickPeriod);
			break;
		case CALIBRATE1:
			// CALIBRATE1 is for waiting for user to lay device down
			break;
		case CALIBRATE2:
		    if (calWait) {
		        // Waiting for device to lay still after button is pressed
		        Task_sleep(1000000 / Clock_tickPeriod);
		        mpu9250_get_data(&i2cMPU, &ax, &ay, &az, &gx, &gy, &gz);
		        ax_off = ax;
		        ay_off = ay;
		        az_off = az;
		        calWait = 0;
		        calLeft = 0;
		        calRight = 0;
		        calFly = 0;
		        minX = 0;
		        maxX = 0;
		        minZ = 0;
//		        calIndex = 0;
		    }
		    mpu9250_get_data(&i2cMPU, &ax, &ay, &az, &gx, &gy, &gz);
		    // Pick largest and smallest ax & smallest az value for R, L & Fly (up) thresholds
		    if (maxX < ax - ax_off) {
		        maxX = ax - ax_off;
		    } else if (minX > ax - ax_off) {
		        minX = ax - ax_off;
		    }
		    if (minZ > az - az_off) {
		        minZ = az - az_off;
		    }
//		    calIndex++;
//		    if (calIndex == 200) {
		        calLeft = minX;
		        calRight = maxX;
		        calFly = minZ;
//		    }
		    break;
		}


//		if (myState == MENU) {
//			if (ay < ACC_LO_THERSHOLD) {
//				cursorPos = C_NEW_GAME;
//			} else if (ay > ACC_HI_THERSHOLD) {
//				cursorPos = C_HIGH_SCORES;
//			} else {
//				cursorPos = C_CALIBRATION;
//			}
//	        calWait = 0;
//		    // use data to move within menu
//		} else if (myState == GAME) {
//			moveBall();
//			Task_sleep(100000 / Clock_tickPeriod);
//		} else if (myState == CALIBRATE) {
//		    if (calWait == 0) {
//		        // Waiting for display task to print instructions to the user
//		        Task_sleep(500000 / Clock_tickPeriod);
//		        calWait = 1;
//		        calLeft = 0;
//		        calRight = 0;
//		        calFly = 0;
//		        calIndex = 0;
//		        minX = 0;
//		        maxX = 0;
//		        minZ = 0;
//		    }
//		    // Pick largest and smallest ax & smallest az value for R, L & Fly (up) thresholds
//		    if (maxX < ax) {
//		        maxX = ax;
//		    } else if (minX > ax) {
//		        minX = ax;
//		    }
//		    if (minZ > az) {
//		        minZ = az;
//		    }
//		    calIndex++;
//		    if (calIndex == 200) {
//		        calLeft = minX;
//		        calRight = maxX;
//		        calFly = minZ;
//		    }
//		}
	}
}

Void displayTask(UArg arg0, UArg arg1) {
// 96 x 96 px	16 x 12 char (char = 5 x 7 px)
	Display_Params displayParams;
	Display_Params_init(&displayParams);
	displayParams.lineClearMode = DISPLAY_CLEAR_NONE;
//		DISPLAY_CLEAR_NONE = 0,   !< Do not clear anything before writing
//	    DISPLAY_CLEAR_LEFT,       !< Clear pixels to left of text on the line
//	    DISPLAY_CLEAR_RIGHT,      !< Clear pixels to right of text on the line
//	    DISPLAY_CLEAR_BOTH        !< Clear pixels on both sides of text

	hDisplay = Display_open(Display_Type_LCD, &displayParams);
	if (hDisplay == NULL) {
		System_abort("Error initializing Display (hDisplay)\n");
	}
	tContext *pContext = DisplayExt_getGrlibContext(hDisplay);
	if (pContext == NULL) {
		System_abort("Error initializing Display (pContext)\n");
	}

	// Calculate graphics positions based on defined values
	trackCoord.trackMinX = MID_COORD - TRACK_W/2;
	trackCoord.trackMaxX = MID_COORD + TRACK_W/2;
	trackCoord.trackMaxY = TRACK_H;
	trackCoord.ballR.x = MID_COORD - TRACK_W/4;
	trackCoord.ballR.y = TRACK_H/10 + 4*TRACK_H/5;
	trackCoord.ballL.x = MID_COORD + TRACK_W/4;
	trackCoord.ballL.y = TRACK_H/10 + 4*TRACK_H/5;
	trackCoord.obstR_x = RIGHT_LANE_X;
	trackCoord.obstL_x = LEFT_LANE_X;
	trackCoord.obstRR_x = RIGHT_SIDE_X;
	trackCoord.obstLL_x = LEFT_SIDE_X;
	trackCoord.obst_y[0] = ROW_0_Y;
	trackCoord.obst_y[1] = ROW_1_Y;
	trackCoord.obst_y[2] = ROW_2_Y;
	trackCoord.obst_y[3] = ROW_3_Y;
	trackCoord.obst_y[4] = ROW_4_Y;

	while (myState == STARTUP) {
		// prevent refreshing display during startup
		Task_sleep(100000 / Clock_tickPeriod);
	}
	while (1) {
//		Display_clear(hDisplay);
		switch (myState) {
		case MENU:
			Display_clear(hDisplay);
			showMenu(hDisplay);
			Task_sleep(1000000 / Clock_tickPeriod); // 1 sec
			break;
		case CALIBRATE1:
			showCalibration1(hDisplay);
			// Task_sleeps built into showCalibration1
			break;
		case CALIBRATE2:
			showCalibration2(hDisplay);
			// Task_sleeps built into showCalibration2
			break;
		case GAME:
			if (gameState == ALIVE) {
				while (newTrackAvailable == 0) {
					Task_sleep(50000 / Clock_tickPeriod);
				}
				newTrackAvailable = 0;
				Display_clear(hDisplay); // Do this as late as possible
				drawObstacles(pContext);
				drawTrack(pContext, hDisplay); // after obstacles so bmps won't undo track lines
				updateBall();
				drawBall(pContext);
				GrFlush(pContext);
				Task_sleep(900000 / Clock_tickPeriod);
			} else if (gameState == GAMEOVER) { // Should work with just "else"
				showGameOver(hDisplay);
				GrFlush(pContext);
				Task_sleep(1000000 / Clock_tickPeriod); // 1sec
				myState = MENU;
				gameState = ALIVE;
			} else {
				System_abort("displayTask found erroneous gameState, aborting");
			}
			break;
		case HIGHSCORES:
			showHighScores(hDisplay);
			break;
		default:
			System_abort("displayTask found erroneous myState, aborting");
		}
	}
}
Int main(void) {
    // Task variables
    Task_Handle hMPU9250Task;
    Task_Params MPU9250TaskParams;
    Task_Handle hdisplayTask;
    Task_Params displayTaskParams;
    Task_Handle hCommTask;
    Task_Params commTaskParams;
	
//	Clock_Params debounceTimerParams = CreateTimer(7, debounceTimer);
	clockHandle = createTimer(7, debounceTimer);

    // Initialize board
    Board_initGeneral();
    Board_initI2C();

    // Initialize Power Button
    hPowerButton = PIN_open(&sPowerButton, cPowerButton);
    if(!hPowerButton) {
        System_abort("Error initializing power button shut pins\n");
    }
    if (PIN_registerIntCb(hPowerButton, &powerButtonFxn) != 0) {
        System_abort("Error registering power button callback function");
    }

    // Initialize Button0
    hButton0 = PIN_open(&sButton0, cButton0);
    if(!hButton0) {
        System_abort("Error initializing led button shut pins\n");
    }
    if (PIN_registerIntCb(hButton0, &Button0Fxn) != 0) {
        System_abort("Error registering led button callback function");
    }

    // Initialize Leds
    hLed = PIN_open(&sLed, cLed);
    if(!hLed) {
        System_abort("Error initializing LED pin\n");
    }

    // Initialize displayTask
    Task_Params_init(&displayTaskParams);
    displayTaskParams.stackSize = STACKSIZE_displayTask;
    displayTaskParams.stack     = &displayTaskStack;
    displayTaskParams.priority  = 3; // cant set to 4?

    hdisplayTask = Task_create(displayTask, &displayTaskParams, NULL);
    if (hdisplayTask == NULL) {
        System_abort("displayTask create failed!");
    }

    // Open MPU power pin
    hMpuPin = PIN_open(&MpuPinState, MpuPinConfig);
    if (hMpuPin == NULL) {
        System_abort("Pin open failed!");
    }
    //  Initialize MPU9250Task
    Task_Params_init(&MPU9250TaskParams);
    MPU9250TaskParams.stackSize = STACKSIZE_MPU9250Task;
    MPU9250TaskParams.stack     = &MPU9250TaskStack;
    MPU9250TaskParams.priority  = 2;

    hMPU9250Task = Task_create(MPU9250Task, &MPU9250TaskParams, NULL);
    if (hMPU9250Task == NULL) {
        System_abort("MPU9250Task create failed!");
    }

    // Initialize commTask
    Task_Params_init(&commTaskParams);
    commTaskParams.stackSize    = STACKSIZE_commTask;
    commTaskParams.stack        = &commTaskStack;
    commTaskParams.priority     = 1; // Might want to add sleeps if priority changed
    Init6LoWPAN();
    hCommTask = Task_create(commTask, &commTaskParams, NULL);
    if (hCommTask == NULL) {
        System_abort("commTask create failed!");
    }

    System_printf("before bios\n");
    BIOS_start();
    System_printf("after bios\n");
    return (0);
}


