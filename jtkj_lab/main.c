#include <stdio.h>
/*
 *  ======== main.c ========
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>

/* TI-RTOS Header files */
#include <ti/drivers/I2C.h>
#include <ti/drivers/i2c/I2CCC26XX.h>
#include <ti/drivers/PIN.h>
#include <ti/drivers/pin/PINCC26XX.h>
#include <ti/mw/display/Display.h>
#include <ti/mw/display/DisplayExt.h>

/* Board Header files */
#include "Board.h"

/* jtkj Header files */
#include "wireless/comm_lib.h"
#include "sensors/bmp280.h"
#include "sensors/mpu9250.h"

#define STACKSIZE_commTask 2048
Char commTaskStack[STACKSIZE_commTask];

#define STACKSIZE_displayTask 2048
Char displayTaskStack[STACKSIZE_displayTask];

#define STACKSIZE_MPU9250Task 4096
Char MPU9250TaskStack[STACKSIZE_MPU9250Task];

//#define STACKSIZE_checkTask 1024
//Char checkTaskStack[STACKSIZE_checkTask];

// pixel graphics
#define OBSTACLE_W 12
#define OBSTACLE_H 12
#define TRACK_H 86
#define TRACK_W 30
#define MID_R_COORD 47
#define MID_L_COORD 46
#define BALL_R_FLYING 6
#define BALL_R 4
static uint8_t ball_r = BALL_R;
static flyCounter = 0;

// xy-coordinate on the display
struct point {
   uint8_t x; // x = [0, 95]
   uint8_t y; // y = [0, 95]
};

// Coordinates for drawing rectangle
struct rect {
   struct point max;
   struct point min;
} displayRect;

// Coordinates for drawing obstacles and ball
struct trackCoordinates {
    struct point ballR;
    struct point ballL;
    struct rect obstR[6];
    struct rect obstL[6];
    struct rect obstRR[6];
    struct rect obstLL[6];
    uint8_t trackMaxX;
    uint8_t trackMinX;
    uint8_t trackMaxY;
};

// Main state machine
enum mainState { STARTUP=0, MENU, GAME, CALIBRATE };
enum mainState myState;

// Flying state machine
enum flyingState { CAN_FLY=0, FLYING, CANT_FLY };
enum flyingState flyState;

// Object's position on the track
enum trackPos { LEFT=0, RIGHT };
enum trackPos BallPos;

// Obstacle's type and position on track
enum obstacle {RIGHTSIDE_MOVING=2, RIGHTLANE_MOVING=4, RIGHTLANE_STATIC=8, LEFTLANE_STATIC=16, LEFTLANE_MOVING=32, LEFTSIDE_MOVING=64};
enum obstacle obstaclePos;

// Default acceleration thresholds if calibration is never run:
static float calLeft = -0.5;
static float calRight = 0.5;
static float calFly = -1.5;

// MPU GLOBAL VARIABLES
static PIN_Handle hMpuPin;
static PIN_State MpuPinState;
static PIN_Config MpuPinConfig[] = {
    Board_MPU_POWER  | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

static float ax, ay, az, gx, gy, gz;
static uint8_t trackBuffer[6];

// MPU9250 uses its own I2C interface
static const I2CCC26XX_I2CPinCfg i2cMPUCfg = {
    .pinSDA = Board_I2C0_SDA1,
    .pinSCL = Board_I2C0_SCL1
};

/* jtkj: Display */
Display_Handle hDisplay;

/* jtkj: Pin Button1 configured as power button */
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

/* jtkj: Pin Button0 configured as input */
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

/* jtkj: Leds */
static PIN_Handle hLed;
static PIN_State sLed;
PIN_Config cLed[] = {
    Board_LED0 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    Board_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

// Power (bottom) button press handler
Void powerButtonFxn(PIN_Handle handle, PIN_Id pinId) {

//    Display_clear(hDisplay);
//    Display_close(hDisplay);
//    Task_sleep(100000 / Clock_tickPeriod);
//
//	PIN_close(hPowerButton);
//
//    PINCC26XX_setWakeup(cPowerWake);
//	Power_shutdown(NULL,0);
}

// Top button press handler
Void Button0Fxn(PIN_Handle handle, PIN_Id pinId) {
    PIN_setOutputValue(hLed, Board_LED0, !PIN_getOutputValue(Board_LED0) );
    switch (myState) {
    case MENU:
    	myState = CALIBRATE;
    	break;
    case CALIBRATE:
    	myState = GAME;
    	break;
    case GAME:
    	myState = MENU;
    	break;
    default:
    	myState = MENU;
    	break;
    }
}

// Communication Task
Void commTask(UArg arg0, UArg arg1) {
	while ((myState == STARTUP) || (myState == CALIBRATE)) {
		// prevent data receive during startup or sensor calibration
	}
    char payload[16];
    uint16_t SenderAddr;
    // Radio to receive mode
	int32_t result = StartReceive6LoWPAN();
	if(result != true) {
		System_abort("Wireless receive mode failed");
	}

    while (1) {
        if (GetRXFlag() == true) {
            memset(payload, 0, 16);
            Receive6LoWPAN(&SenderAddr, payload, 16);
            trackBuffer[5] = trackBuffer[4];
            trackBuffer[4] = trackBuffer[3];
            trackBuffer[3] = trackBuffer[2];
            trackBuffer[2] = trackBuffer[1];
            trackBuffer[1] = trackBuffer[0];
            trackBuffer[0] = payload[0];
        }
    }
}

Void moveBall() {
    // Moves ball left / right / up
    if (ax < calLeft) {
        BallPos = LEFT;
    } else if (ax > calRight) {
        BallPos = RIGHT;
    }
    if (az < calFly && flyState == CAN_FLY) {
        flyState = FLYING;
    }
}

//Void checkTask(UArg arg0, UArg arg1) {
//    // Checks whether the ball and an obstacle are in the same position
////    uint8_t bottomLine = trackBuffer[5];
////
////    uint8_t bitMaskLeft = 0b00110000;
////    uint8_t bitMaskRight = 0b00001100;
////    if ((bottomline & bitMaskLeft) && (BallPos))
//
//    while (1) {
////        if (flyState == FLYING) {////            ball_r = BALL_R_FLYING;////            flyState = CANT_FLY;////        } else if (flyState == CANT_FLY) {////            ball_r = BALL_R;////            flyState = CAN_FLY;
////        }//        Task_sleep(100000 / Clock_tickPeriod);
//    }
//}

Void MPU9250Task(UArg arg0, UArg arg1) {
//	I2C_Handle      i2c;
//	I2C_Params      i2cParams;
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

	if (myState == STARTUP) {
		myState = MENU;
	}
	else {
		System_abort("Error: incorrect state\n");
	}

	// Creating calWait variable to ensure calibration task waits for sensor setup
	uint8_t calWait = 0; // how to make a boolean??
	uint8_t calIndex = 0;
    float maxX = 0;
    float minX = 0;
    float minZ = 0;
	while (1) {
		mpu9250_get_data(&i2cMPU, &ax, &ay, &az, &gx, &gy, &gz);
		if (myState == MENU) {
	        calWait = 0;
		    // use data to move within menu
		} else if (myState == GAME) {
			moveBall();
			Task_sleep(10000 / Clock_tickPeriod);
		} else if (myState == CALIBRATE) {
		    if (calWait == 0) {
		        // Waiting for display task to print instructions to the user
		        Task_sleep(500000 / Clock_tickPeriod);
		        calWait = 1;
		        calLeft = 0;
		        calRight = 0;
		        calFly = 0;
		        calIndex = 0;
		        minX = 0;
		        maxX = 0;
		        minZ = 0;
		    }
		    // Pick largest and smallest ax & smallest az value for R, L & Fly (up) thresholds
		    if (maxX < ax) {
		        maxX = ax;
		    } else if (minX > ax) {
		        minX = ax;
		    }
		    if (minZ > az) {
		        minZ = az;
		    }
		    calIndex++;
		    if (calIndex == 200) {
		        calLeft = minX;
		        calRight = maxX;
		        calFly = minZ;
		    }
		}
	}
}

Void drawTrack(tContext *pContext) {
    Display_clear(hDisplay);
    Display_print0(hDisplay, 11, 0, "MSG: ");
    GrLineDrawH(pContext,0,95,TRACK_H);
    GrLineDraw(pContext, MID_L_COORD-TRACK_W/2, 0, MID_L_COORD-TRACK_W/2, TRACK_H);
    GrLineDraw(pContext, MID_R_COORD+TRACK_W/2, 0, MID_R_COORD+TRACK_W/2, TRACK_H);
}

Void drawObstacles(tContext *pContext, tRectangle ObstacleRect) {
    int i;
    for (i=0; i < 6; i++) {
        // Covering 5 seconds worth of trackBuffer
        ObstacleRect.sYMin = TRACK_H/12 + i*TRACK_H/6 - OBSTACLE_W/2;
        ObstacleRect.sYMax = TRACK_H/12 + i*TRACK_H/6 + OBSTACLE_W/2;

        if (trackBuffer[i] & LEFTSIDE_MOVING) {
            // Moving obstacle on left side of track
            ObstacleRect.sXMin = MID_L_COORD - 3*TRACK_W/4 - OBSTACLE_W/2;
            ObstacleRect.sXMax = MID_L_COORD - 3*TRACK_W/4 + OBSTACLE_W/2;
            GrRectDraw(pContext, &ObstacleRect);
        }
        if (trackBuffer[i] & LEFTLANE_MOVING) {
            // Moving obstacle in left lane
            ObstacleRect.sXMin = MID_L_COORD - TRACK_W/4 - OBSTACLE_W/2;
            ObstacleRect.sXMax = MID_L_COORD - TRACK_W/4 + OBSTACLE_W/2;
            GrRectDraw(pContext, &ObstacleRect);
        }
        if (trackBuffer[i] & LEFTLANE_STATIC) {
            // Static obstacle in left lane
            ObstacleRect.sXMin = MID_L_COORD - TRACK_W/4 - OBSTACLE_W/2;
            ObstacleRect.sXMax = MID_L_COORD - TRACK_W/4 + OBSTACLE_W/2;
            GrRectFill(pContext, &ObstacleRect);
        }
        if (trackBuffer[i] & RIGHTLANE_STATIC) {
            // Static obstacle in right lane
            ObstacleRect.sXMin = MID_R_COORD + TRACK_W/4 - OBSTACLE_W/2;
            ObstacleRect.sXMax = MID_R_COORD + TRACK_W/4 + OBSTACLE_W/2;
            GrRectFill(pContext, &ObstacleRect);
        }
        if (trackBuffer[i] & RIGHTLANE_MOVING) {
            // Moving obstacle in right lane
            ObstacleRect.sXMin = MID_R_COORD + TRACK_W/4 - OBSTACLE_W/2;
            ObstacleRect.sXMax = MID_R_COORD + TRACK_W/4 + OBSTACLE_W/2;
            GrRectDraw(pContext, &ObstacleRect);
        }
        if (trackBuffer[i] & RIGHTSIDE_MOVING) {
            // MOving obstacle on right side of track
            ObstacleRect.sXMin = MID_R_COORD + 3*TRACK_W/4 - OBSTACLE_W/2;
            ObstacleRect.sXMax = MID_R_COORD + 3*TRACK_W/4 + OBSTACLE_W/2;
            GrRectDraw(pContext, &ObstacleRect);
        }
    }
}

Void drawBall(tContext *pContext) {
    if (flyState == FLYING) {
        ball_r = BALL_R_FLYING;
        flyState = CANT_FLY;
    } else if (flyState == CANT_FLY) {
        ball_r = BALL_R;
        flyState = CAN_FLY;
    }
    if (BallPos == LEFT) {
        GrCircleFill(pContext, MID_L_COORD - TRACK_W/4, TRACK_H/12 + 5*TRACK_H/6, ball_r);
    } else {
        GrCircleFill(pContext, MID_R_COORD + TRACK_W/4, TRACK_H/12 + 5*TRACK_H/6, ball_r);
    }
}

Void showMenu(Display_Params displayParams) {
    displayParams.lineClearMode = DISPLAY_CLEAR_RIGHT;
    char text[17];
    sprintf(text, "ax = %.3f", ax);
    Display_print0(hDisplay, 0, 0, text);
    sprintf(text, "ay = %.3f", ay);
    Display_print0(hDisplay, 1, 0, text);
    sprintf(text, "az = %.3f", az);
    Display_print0(hDisplay, 2, 0, text);
    sprintf(text, "gx = %.3f", gx);
    Display_print0(hDisplay, 3, 0, text);
    sprintf(text, "gy = %.3f", gy);
    Display_print0(hDisplay, 4, 0, text);
    sprintf(text, "gz = %.3f", gz);
    Display_print0(hDisplay, 5, 0, text);

}

Void showCalibration(Display_Params displayParams) {
    Display_clear(hDisplay);
    char line1[16], line2[16];

    sprintf(line1, "CALIBRATING");
    sprintf(line2, "Tilt L & R");
    Display_print0(hDisplay, 1, 0, line1);
    Display_print0(hDisplay, 4, 0, line2);

    // Wait here until calibration is done:
    while (myState == CALIBRATE) {
        // Check state every 0.1 seconds
        // System_printf("Still calibrating display\n");
        Task_sleep(100000 / Clock_tickPeriod);
    }
    displayParams.lineClearMode = DISPLAY_CLEAR_BOTH;
    sprintf(line1, "R = %.1f", calRight);
    Display_print0(hDisplay, 8, 0, line1);
    sprintf(line1, "L = %.1f", calLeft);
    Display_print0(hDisplay, 9, 0, line1);
    sprintf(line2, "DONE");
    Display_print0(hDisplay, 6, 0, line2);
    Task_sleep(1000000 / Clock_tickPeriod);
    Display_clear(hDisplay);

    Task_sleep(1000000 / Clock_tickPeriod);
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

	// Define graphics positions
	/*    struct trackCoordinates trackCoord;
	    trackCoord.trackMinX = MID_L_COORD-TRACK_W/2;
	    trackCoord.trackMaxX = MID_R_COORD+TRACK_W/2;
	    trackCoord.trackMaxY = TRACK_H;
	    trackCoord.ballR.x = MID_L_COORD - TRACK_W/4;
	    trackCoord.ballR.y = TRACK_H/12 + 5*TRACK_H/6;
	    trackCoord.ballL.x = MID_R_COORD + TRACK_W/4;
	    trackCoord.ballL.y = TRACK_H/12 + 5*TRACK_H/6;
	    uint8_t i;
	    for (i=0; i < 6; i++) {
	        trackCoord.obstR[i].min.x = MID_R_COORD + TRACK_W/4 - OBSTACLE_W/2;
	        trackCoord.obstR[i].min.y = TRACK_H/12 + i*TRACK_H/6 - OBSTACLE_W/2;
	        trackCoord.obstR[i].max.x = MID_R_COORD + TRACK_W/4 + OBSTACLE_W/2;
	        trackCoord.obstR[i].max.y = TRACK_H/12 + i*TRACK_H/6 + OBSTACLE_W/2;

	        trackCoord.obstL[i].min.x = MID_L_COORD - TRACK_W/4 - OBSTACLE_W/2;
	        trackCoord.obstL[i].min.y = TRACK_H/12 + i*TRACK_H/6 - OBSTACLE_W/2;
	        trackCoord.obstL[i].max.x = MID_L_COORD - TRACK_W/4 + OBSTACLE_W/2;
	        trackCoord.obstL[i].max.y = TRACK_H/12 + i*TRACK_H/6 + OBSTACLE_W/2;

	        trackCoord.obstRR[i].min.x = MID_R_COORD + 3*TRACK_W/4 - OBSTACLE_W/2;
	        trackCoord.obstRR[i].min.y = TRACK_H/12 + i*TRACK_H/6 - OBSTACLE_W/2;
	        trackCoord.obstRR[i].max.x = MID_R_COORD + 3*TRACK_W/4 + OBSTACLE_W/2;
	        trackCoord.obstRR[i].max.y = TRACK_H/12 + i*TRACK_H/6 + OBSTACLE_W/2;

	        trackCoord.obstLL[i].min.x = MID_L_COORD - 3*TRACK_W/4 - OBSTACLE_W/2;
	        trackCoord.obstLL[i].min.y = TRACK_H/12 + i*TRACK_H/6 - OBSTACLE_W/2;
	        trackCoord.obstLL[i].max.x = MID_L_COORD - 3*TRACK_W/4 + OBSTACLE_W/2;
	        trackCoord.obstLL[i].max.y = TRACK_H/12 + i*TRACK_H/6 + OBSTACLE_W/2;
	    }
	    displayRect.min.x = 0;
	    displayRect.min.y = 0;
	    displayRect.max.x = GrContextDpyWidthGet(pContext) - 1;
	    displayRect.max.y = GrContextDpyHeightGet(pContext) - 1;
*/

	tRectangle ObstacleRect;

	while (1) {
		Display_clear(hDisplay);
		while (myState == MENU) {
			showMenu(displayParams);
			Task_sleep(1000000 / Clock_tickPeriod); // 1 sec
		}
		while (myState == CALIBRATE) {
		    showCalibration(displayParams);
		    // Task_sleeps built into showCalibration
		}
		while (myState == GAME) {
			drawTrack(pContext);
			drawBall(pContext);
			drawObstacles(pContext, ObstacleRect);

			GrFlush(pContext);

			Task_sleep(1000000 / Clock_tickPeriod); // 1sec
		}
	}
}
Int main(void) {
    //  Task variables
    Task_Handle hMPU9250Task;
    Task_Params MPU9250TaskParams;
    Task_Handle hdisplayTask;
    Task_Params displayTaskParams;
    Task_Handle hCommTask;
    Task_Params commTaskParams;
//    Task_Handle hCheckTask;
//    Task_Params checkTaskParams;

    //  Initialize board
    Board_initGeneral();
    Board_initI2C();

    // Power Button
    hPowerButton = PIN_open(&sPowerButton, cPowerButton);
    if(!hPowerButton) {
        System_abort("Error initializing power button shut pins\n");
    }
    if (PIN_registerIntCb(hPowerButton, &powerButtonFxn) != 0) {
        System_abort("Error registering power button callback function");
    }

    // INITIALIZE BUTTON0
    hButton0 = PIN_open(&sButton0, cButton0);
    if(!hButton0) {
        System_abort("Error initializing led button shut pins\n");
    }
    if (PIN_registerIntCb(hButton0, &Button0Fxn) != 0) {
        System_abort("Error registering led button callback function");
    }

    // Init Leds
    hLed = PIN_open(&sLed, cLed);
    if(!hLed) {
        System_abort("Error initializing LED pin\n");
    }

    // Init displayTask
    Task_Params_init(&displayTaskParams);
    displayTaskParams.stackSize = STACKSIZE_displayTask;
    displayTaskParams.stack     = &displayTaskStack;
    displayTaskParams.priority  = 3; // cant set to 4??

    hdisplayTask = Task_create(displayTask, &displayTaskParams, NULL);
    if (hdisplayTask == NULL) {
        System_abort("displayTask create failed!");
    }

//    // Init check task
//    Task_Params_init(&checkTaskParams);
//    checkTaskParams.stackSize = STACKSIZE_checkTask;
//    checkTaskParams.stack     = &checkTaskStack;
//    checkTaskParams.priority  = 3;
//
//    hCheckTask = Task_create(checkTask, &checkTaskParams, NULL);
//    if (hCheckTask == NULL) {
//        System_abort("CheckTask create failed!");
//    }

    // OPEN MPU POWER PIN
    hMpuPin = PIN_open(&MpuPinState, MpuPinConfig);
    if (hMpuPin == NULL) {
        System_abort("Pin open failed!");
    }
    //  Init MPU9250Task
    Task_Params_init(&MPU9250TaskParams);
    MPU9250TaskParams.stackSize = STACKSIZE_MPU9250Task;
    MPU9250TaskParams.stack     = &MPU9250TaskStack;
    MPU9250TaskParams.priority  = 2;

    hMPU9250Task = Task_create(MPU9250Task, &MPU9250TaskParams, NULL);
    if (hMPU9250Task == NULL) {
        System_abort("MPU9250Task create failed!");
    }

    // Init communication task
    Task_Params_init(&commTaskParams);
    commTaskParams.stackSize    = STACKSIZE_commTask;
    commTaskParams.stack        = &commTaskStack;
    commTaskParams.priority     = 1;
    Init6LoWPAN();
    hCommTask = Task_create(commTask, &commTaskParams, NULL);
    if (hCommTask == NULL) {
        System_abort("CommTask create failed!");
    }

    System_printf("before bios");
    BIOS_start();
    System_printf("after bios");
    return (0);
}


