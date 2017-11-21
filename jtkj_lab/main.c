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
#include <Board.h>

// Include audio
#include "audio.h"

// jtkj Header files
#include "wireless/comm_lib.h"
#include "sensors/bmp280.h"
#include "sensors/mpu9250.h"

#include <inttypes.h>

// Obstacle graphics
#include "graphics.h"

// Functions used by displayTask
#include "display_functions.h"

// Functions used to run the game
// #include "game.h"

#define STACKSIZE_audioTask 1024
Char audioTaskStack[STACKSIZE_audioTask];

#define STACKSIZE_commTask 2048
Char commTaskStack[STACKSIZE_commTask];

#define STACKSIZE_displayTask 2048
Char displayTaskStack[STACKSIZE_displayTask];

#define STACKSIZE_MPU9250Task 4096
Char MPU9250TaskStack[STACKSIZE_MPU9250Task];

// How many samples are averaged for level calibration, make sure double can hold the sum of samples
#define CALIBRATE_LEVEL_SAMPLES 5

//// Coordinates for drawing rectangle
//struct rect {
//   struct point max;
//   struct point min;
//} displayRect;

// Clock handle
Clock_Handle clockHandleBtn0;
Clock_Handle clockHandleBtn1;

// audioTask variables
static Task_Handle hAudioTask;
static Task_Params audioTaskParams;

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

// Buzzer configuration
static PIN_Handle hBuzzer;

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
enum diyBoolean button0AllowExec = BOOLEAN_1;
enum diyBoolean button1AllowExec = BOOLEAN_1;

// Leds configuraion
static PIN_Handle hLed;
static PIN_State sLed;
PIN_Config cLed[] = {
    Board_LED0 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    Board_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};
static unsigned char inGameTXMessage[8] = "Tere";

Void audioTaskRestart();

Void debounceTimerBtn0(UArg arg0) {
	// Sets flag to allow Button0 to function after button pressed
    button0AllowExec = BOOLEAN_1;
}

Void debounceTimerBtn1(UArg arg0) {
	// Sets flag to allow Button1 to function after button pressed
    button1AllowExec = BOOLEAN_1;
}

Clock_Handle createTimer(uint8_t period, Clock_FuncPtr clkFxn) {
	// Function to create a new timer that runs once using Clock_start()
    // Period in tenths of a second

    // RTOS clock variables
    Clock_Handle clkHandle;
    Clock_Params clkParams;

    // Initialization
    Clock_Params_init(&clkParams);
    clkParams.period = 0; // To launch only when called
    clkParams.startFlag = FALSE; // FALSE == not started with Clock_create()

    clkHandle = Clock_create((Clock_FuncPtr)clkFxn, 100000 * period / Clock_tickPeriod, &clkParams, NULL);
    if (clkHandle == NULL) {
    	System_abort("Clock creat failed");
    }
    return clkHandle;
}

Void powerButtonFxn(PIN_Handle handle, PIN_Id pinId) {
	// Button1 (bottom) button press handler
	if (button1AllowExec == BOOLEAN_1) {
		button1AllowExec = BOOLEAN_0;
		switch (myState) {
		case MENU:
		    Display_clear(hDisplay);
		    Display_close(hDisplay);
		    Task_sleep(100000 / Clock_tickPeriod);
			PIN_close(hPowerButton);
		    PINCC26XX_setWakeup(cPowerWake);
			Power_shutdown(NULL,0);
			break;
		case CALIBRATE:
			if (volume == ANNOYING_AF) {
				volume = MUTE;
			} else {
				volume = ANNOYING_AF;
			}
			break;
	    case GAME:
		    Send6LoWPAN(IEEE80154_SERVER_ADDR, inGameTXMessage, 8);
		    PIN_setOutputValue(hLed, Board_LED0, !PIN_getOutputValue(Board_LED0));
			break;
	    case HIGHSCORES:
			break;
	    default:
			break;
		}
	    Clock_start(clockHandleBtn1); // Start debounce timer
	}
}

Void Button0Fxn(PIN_Handle handle, PIN_Id pinId) {
	// Button0 (top) button press handler
	if (button0AllowExec == BOOLEAN_1) { // For debounce
		button0AllowExec = BOOLEAN_0;
//	    PIN_setOutputValue(hLed, Board_LED0, !PIN_getOutputValue(Board_LED0));
	    switch (myState) {
	    case MENU:
	    	switch (cursorPos) {
	    	case C_NEW_GAME:
	    		myState = GAME;
	    		break;
	    	case C_CALIBRATION:
	    		myState = CALIBRATE;
	    		calState = CALIBRATE_HELP;
	    		break;
	    	case C_HIGH_SCORES:
	    		myState = HIGHSCORES;
	    		break;
	    	}
	    	break;
	    case CALIBRATE:
			if (calState == CALIBRATE_HELP) {
				calState = CALIBRATE_LEVEL;
			} else if (calState == CALIBRATE_MOVEMENT) {
				myState = MENU;
				calState = CALIBRATE_HELP;
			}
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
	    Clock_start(clockHandleBtn0); // Start debounce timer
	}
}

Void audioTask(UArg arg0, UArg arg1) {
    while (myState == STARTUP) {
        Task_sleep(10000 / Clock_tickPeriod);
    }
    buzzerOpen(hBuzzer);
	while (1) {
		if (volume == ANNOYING_AF) {
			switch (myState) {
			case MENU:
				playGonnaFlyNow();
				break;
			case CALIBRATE:
				playOne();
				break;
			case GAME:
				playRiverside1();
				playRiverside1();
				playRiverside2();
				playRiverside3();
				break;
			case HIGHSCORES:
				playEpicSaxGuy();
				break;
			default:
				Task_sleep(1000000 / Clock_tickPeriod);
				break;
			}
		} else {
			Task_sleep(1000000 / Clock_tickPeriod);
		}
    }
}

Void audioTaskRestart() {
	Task_delete(hAudioTask);

//	hAudioTask = Task_create(audioTask, &audioTaskParams, NULL);
//	if (hAudioTask == NULL) {
//		System_abort("audioTask create failed!");
//	}
}

Void commTask(UArg arg0, UArg arg1) {
	// Communication Task
	while (myState == STARTUP) {
		// prevent data receive during startup or sensor calibration (Task_sleep breaks commTask functionality!?)
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
	uint32_t i; // General for-variable
    while (0) {
        if (GetRXFlag() == true) {
            memset(payload, 0, 16);
            Receive6LoWPAN(&SenderAddr, payload, 16);
//            Receive6LoWPAN(IEEE80154_SERVER_ADDR, payload, 16);
            if (myState == GAME) {
            	for (i = 0; i < 8; i++) {
            		inGameRXMsg[i] = payload[i+1];
            	}
            	for (i = 4; i > 0; i--) {
            		trackBuffer[i] = trackBuffer[i-1];
            	}
            	trackBuffer[0] = payload[0];
                newTrackAvailable = BOOLEAN_1;
                gameScore++;
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

Void MPU9250Task(UArg arg0, UArg arg1) {
	// Acceleration/gyro sensor task
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
	// Critical ends, allow other tasks to resume:
	if (myState == STARTUP) {
		myState = MENU;
	} else {
		System_abort("MPU9250Task found erroneous myState, aborting\n");
	}

    double ax_sampled;
    double ay_sampled;
    double az_sampled;
    uint32_t i; // General for-variable
	while (1) {
		switch (myState) {
		case MENU:
			mpu9250_get_data(&i2cMPU, &ax, &ay, &az, &gx, &gy, &gz);
			if (ay - ay_off < ACC_LO_THERSHOLD) {
				cursorPos = C_NEW_GAME;
			} else if (ay - ay_off > ACC_HI_THERSHOLD) {
				cursorPos = C_HIGH_SCORES;
			} else {
				cursorPos = C_CALIBRATION;
			}
			break;
		case GAME:
			mpu9250_get_data(&i2cMPU, &ax, &ay, &az, &gx, &gy, &gz);
			moveBall();
			Task_sleep(100000 / Clock_tickPeriod);
			break;
		case CALIBRATE:
			switch (calState) {
			case CALIBRATE_HELP:
				calLevelReady = BOOLEAN_0;
				ax_sampled = 0;
				ay_sampled = 0;
				az_sampled = 0;
				calLeft = 0;
				calRight = 0;
				calFly = 0;
				while (calState == CALIBRATE_HELP) {
					Task_sleep(100000 / Clock_tickPeriod);
				}
				break;
			case CALIBRATE_LEVEL:
				Task_sleep(1000000 / Clock_tickPeriod); // Wait for device to lay still after button press
				for (i = 0; i < CALIBRATE_LEVEL_SAMPLES; i++) {
					mpu9250_get_data(&i2cMPU, &ax, &ay, &az, &gx, &gy, &gz);
					ax_sampled += ax;
					ay_sampled += ay;
					az_sampled += az;
					Task_sleep(50000 / Clock_tickPeriod);
				}
				ax_off = ax_sampled / CALIBRATE_LEVEL_SAMPLES;
				ay_off = ay_sampled / CALIBRATE_LEVEL_SAMPLES;
				az_off = az_sampled / CALIBRATE_LEVEL_SAMPLES;
				calLevelReady = BOOLEAN_1;
				while (calState == CALIBRATE_LEVEL) {
					Task_sleep(100000 / Clock_tickPeriod);
				}
				break;
			case CALIBRATE_MOVEMENT:
				mpu9250_get_data(&i2cMPU, &ax, &ay, &az, &gx, &gy, &gz);
				// Pick largest and smallest ax & smallest az value for R, L & Fly (up) thresholds
				if (calRight < ax - ax_off) {
					calRight = ax - ax_off;
				} else if (calLeft > ax - ax_off) {
					calLeft = ax - ax_off;
				}
				if (calFly > az - az_off) {
					calFly = az - az_off;
				}
				break;
			}
		case HIGHSCORES:
			Task_sleep(100000 / Clock_tickPeriod);
			break;
		}
	}
}

Void displayTask(UArg arg0, UArg arg1) {
	// Specs: 96 x 96 px	&	16 x 12 char (char = 5 x 7 px)
	Display_Params displayParams;
	Display_Params_init(&displayParams);
	displayParams.lineClearMode = 	DISPLAY_CLEAR_NONE;
	//	DISPLAY_CLEAR_NONE = 0,   !< Do not clear anything before writing
	//	DISPLAY_CLEAR_LEFT,       !< Clear pixels to left of text on the line
	//	DISPLAY_CLEAR_RIGHT,      !< Clear pixels to right of text on the line
	//	DISPLAY_CLEAR_BOTH        !< Clear pixels on both sides of text

	// Open display & pixel graphics drawing context
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
		switch (myState) {
		case MENU:
			showMenu(hDisplay);
			break;
		case CALIBRATE:
			switch (calState) {
			case CALIBRATE_HELP:
	//    		audioTaskRestart();
				showCalibrateHelp(hDisplay);
				while (calState == CALIBRATE_HELP) {
					Task_sleep(100000 / Clock_tickPeriod);
				}
				break;
			case CALIBRATE_LEVEL:
				showCalibrateLevel(hDisplay);
				break;
			case CALIBRATE_MOVEMENT:
				showCalibrateMovement(hDisplay);
				break;
			}
			break;
		case GAME:
			if (gameState == ALIVE) {
				while (newTrackAvailable == BOOLEAN_0) {
					Task_sleep(50000 / Clock_tickPeriod);
				}
				newTrackAvailable = BOOLEAN_0;
				Display_clear(hDisplay); // Do this as late as possible before GrFlush()
				drawObstacles(pContext);
				drawTrack(pContext, hDisplay); // After obstacles so that bmps won't undo track lines
				updateBall();
				drawBall(pContext);
				GrFlush(pContext); // Do this as soon as possible after Display_clear()
				Task_sleep(500000 / Clock_tickPeriod); // 0,5 sec
			} else if (gameState == GAMEOVER) {
				showGameOver(hDisplay);
				GrFlush(pContext);
				Task_sleep(1000000 / Clock_tickPeriod); // 1 sec
				myState = MENU;
				gameState = ALIVE;
			} else {
				System_abort("displayTask found erroneous gameState, aborting");
			}
			break;
		case HIGHSCORES:
			showHighScores(hDisplay);
			while (myState == HIGHSCORES) {
				Task_sleep(100000 / Clock_tickPeriod);
			}
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
	
    // Create debounce timer
	clockHandleBtn0 = createTimer(7, debounceTimerBtn0);
	clockHandleBtn1 = createTimer(7, debounceTimerBtn1);

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

    // Initialize audioTask
    Task_Params_init(&audioTaskParams);
    audioTaskParams.stackSize    = STACKSIZE_audioTask;
    audioTaskParams.stack        = &audioTaskStack;
    audioTaskParams.priority     = 2;
    hAudioTask = Task_create(audioTask, &audioTaskParams, NULL);
    if (hAudioTask == NULL) {
        System_abort("audioTask create failed!");
    }

    System_printf("before bios\n");
    BIOS_start();
    System_printf("after bios\n");

    return (0);
}
