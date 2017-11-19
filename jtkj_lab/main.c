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

// Obstacle graphics
#include "graphics.h"

#define STACKSIZE_commTask 2048
Char commTaskStack[STACKSIZE_commTask];

#define STACKSIZE_displayTask 2048
Char displayTaskStack[STACKSIZE_displayTask];

#define STACKSIZE_MPU9250Task 4096
Char MPU9250TaskStack[STACKSIZE_MPU9250Task];

// Sensor thresholds (before possible calibration)
#define ACC_HI_THERSHOLD 0.25
#define ACC_LO_THERSHOLD -0.25

// Pixel graphics
#define OBSTACLE_W 16
#define OBSTACLE_H 16
#define TRACK_H 86
#define TRACK_W 32
#define MID_COORD 48
#define BALL_R_FLYING 6
#define BALL_R 4
static uint8_t ball_r = BALL_R;

// New pixel graphics
#define RIGHT_SIDE_X 70
#define RIGHT_LANE_X 50
#define LEFT_LANE_X 34
#define LEFT_SIDE_X 18
#define ROW_0_Y 8
#define ROW_1_Y 24
#define ROW_2_Y 40
#define ROW_3_Y 56
#define ROW_4_Y 72


// xy-coordinate on the display
struct point {
   uint8_t x; // x = [0, 95]
   uint8_t y; // y = [0, 95]
};

//// Coordinates for drawing rectangle
//struct rect {
//   struct point max;
//   struct point min;
//} displayRect;

// Coordinates for drawing obstacles and ball
typedef struct {
    struct point ballR;
    struct point ballL;
    uint8_t obst_y[5];
    uint8_t obstRR_x;
    uint8_t obstR_x;
    uint8_t obstL_x;
    uint8_t obstLL_x;
    uint8_t trackMaxX;
    uint8_t trackMinX;
    uint8_t trackMaxY;
} trackCoordinates;
static trackCoordinates trackCoord;

// Main state machine
enum mainState {STARTUP=0, MENU, GAME, CALIBRATE1, CALIBRATE2, HIGHSCORES};
enum mainState myState;

// Game state machine
enum gameStatus {ALIVE=1, GAMEOVER=0};
enum gameStatus gameState = ALIVE;

// Flying state machine
enum flyingState {CAN_FLY=0, FLYING, CANT_FLY};
enum flyingState flyState = CAN_FLY;

// Object's position on the track
enum trackPosition {LEFT=0, RIGHT};
enum trackPosition BallPos;

// Cursor's position in the menu
enum cursorPosition {C_NEW_GAME=4, C_CALIBRATION, C_HIGH_SCORES};
enum cursorPosition cursorPos;

// Obstacle's type and position on track as bitmask
enum obstacle {RIGHTSIDE_MOVING=2, RIGHTLANE_MOVING=4, RIGHTLANE_STATIC=8, LEFTLANE_STATIC=16, LEFTLANE_MOVING=32, LEFTSIDE_MOVING=64};
enum obstacle obstaclePos;

// Default acceleration thresholds if calibration is never run:
static float calLeft = ACC_LO_THERSHOLD*2;
static float calRight = ACC_HI_THERSHOLD*2;
static float calFly = ACC_LO_THERSHOLD*2;

// MPU global variables
static PIN_Handle hMpuPin;
static PIN_State MpuPinState;
static PIN_Config MpuPinConfig[] = {
    Board_MPU_POWER  | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};
static float ax, ay, az, gx, gy, gz; // For raw data
static float ax_off = 0; // Calibration offset values
static float ay_off = 0; // default is that device is booted on flat surface
static float az_off = 0;
static float gx_off = 0;
static float gy_off = 0;
static float gz_off = 0;

// MPU9250 uses its own I2C interface
static const I2CCC26XX_I2CPinCfg i2cMPUCfg = {
    .pinSDA = Board_I2C0_SDA1,
    .pinSCL = Board_I2C0_SCL1
};

// commTask global variables
static char message[9];
static uint8_t trackBuffer[5];
static uint8_t newTrackAvailable;


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

// Button1 (bottom) button press handler TODO
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

// Button0 (top) button press handler TODO: debounce
Void Button0Fxn(PIN_Handle handle, PIN_Id pinId) {
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
    default:
    	myState = MENU;
    	break;
    }
//    Task_sleep(700000 / Clock_tickPeriod); // This will break at least calibration, WTF?
}

// Communication Task
Void commTask(UArg arg0, UArg arg1) {
	while ((myState == STARTUP)) {
		// prevent data receive during startup or sensor calibration (Task_sleep breaks commTask functionality!)
//		Task_sleep(100000 / Clock_tickPeriod);
	}
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
            }
        } // No sleeps because of lowest priority
    }
}

// Move ball left / right / up
Void moveBall() {
    if (ax < calLeft) {
        BallPos = LEFT;
    } else if (ax > calRight) {
        BallPos = RIGHT;
    }
    if ((az < calFly) && (flyState == CAN_FLY)) {
    	ball_r = BALL_R_FLYING;
    	flyState = FLYING;
    }
}

// This makes sure ball falls down & dies when being hit
Void updateBall() {
	if (gameState == ALIVE) {
		// Check if ball and obstacle overlap
		if (flyState == FLYING) {
			if (BallPos == LEFT) {
				if (trackBuffer[4] & LEFTLANE_STATIC) {
					gameState = GAMEOVER;
				}
			} else if (BallPos == RIGHT) {
				if (trackBuffer[4] & RIGHTLANE_STATIC) {
					gameState = GAMEOVER;
				}
			}
		} else { // flyState != FLYING
			if (BallPos == LEFT) {
				if (trackBuffer[4] & LEFTLANE_MOVING) {
					gameState = GAMEOVER;
				}
			} else if (BallPos == RIGHT) {
				if (trackBuffer[4] & RIGHTLANE_MOVING) {
					gameState = GAMEOVER;
				}
			}
		}
	}
	// flyState must be changed after overlap-check
	if (flyState == FLYING) {
		ball_r = BALL_R_FLYING;
		flyState = CANT_FLY;
	} else if (flyState == CANT_FLY) {
		ball_r = BALL_R;
		flyState = CAN_FLY;
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
			if (ay < ACC_LO_THERSHOLD - ay_off) {
				cursorPos = C_NEW_GAME;
			} else if (ay > ACC_HI_THERSHOLD - ay_off) {
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

Void drawTrack(tContext *pContext) {
    char msg[16];
    sprintf(msg, "MSG: %s", message);
    Display_print0(hDisplay, 11, 0, msg);
    GrLineDrawH(pContext, 0, 95, trackCoord.trackMaxY);
    GrLineDrawV(pContext, trackCoord.trackMinX, 0, trackCoord.trackMaxY);
    GrLineDrawV(pContext, trackCoord.trackMaxX, 0, trackCoord.trackMaxY);
}

Void drawObstacles(tContext *pContext) {
    int i;
    for (i=0; i < 5; i++) {
        // Covering 5 seconds worth of trackBuffer
    	if (trackBuffer[i] & LEFTSIDE_MOVING) { // Moving obstacle on left side of track
    		drawDiagObstacle(trackCoord.obstLL_x, trackCoord.obst_y[i], pContext);
    	}
    	if (trackBuffer[i] & LEFTLANE_MOVING) { // Moving obstacle in left lane
    		drawDiagObstacle(trackCoord.obstL_x, trackCoord.obst_y[i], pContext);
    	}
    	if (trackBuffer[i] & LEFTLANE_STATIC) { // Static obstacle in left lane
    		drawFlyingObstacle(trackCoord.obstL_x, trackCoord.obst_y[i], pContext);
    	}
    	if (trackBuffer[i] & RIGHTLANE_STATIC) { // Static obstacle in right lane
    		drawFlyingObstacle(trackCoord.obstR_x, trackCoord.obst_y[i], pContext);
    	}
    	if (trackBuffer[i] & RIGHTLANE_MOVING) { // Moving obstacle in right lane
    		drawDiagObstacle(trackCoord.obstRR_x, trackCoord.obst_y[i], pContext);
    	}
    	if (trackBuffer[i] & RIGHTSIDE_MOVING) { // Moving obstacle on right side of track
    		drawDiagObstacle(trackCoord.obstRR_x, trackCoord.obst_y[i], pContext);
    	}
    }

//    // For testing coordinates that GrImageDraw() uses:
//    const uint8_t kuutioData[8] = {
//    		0b11111111,
//    		0b11111111,
//    		0b11111111,
//    		0b11111111,
//    		0b11111111,
//    		0b11111111,
//    		0b11111111,
//    		0b11111111
//    };
//    uint32_t Paletti[] = {0, 0xFFFFFF};
//    const tImage kuutioImage = {
//    		.BPP = IMAGE_FMT_1BPP_UNCOMP,
//    		.NumColors = 2,
//    		.XSize = 1,
//    		.YSize = 8,
//    		.pPalette = Paletti,
//    		.pPixel = kuutioData
//    };
//    GrImageDraw(pContext, &kuutioImage, 0, 0);
//    GrImageDraw(pContext, &kuutioImage, 1, 10);
//    GrImageDraw(pContext, &kuutioImage, 2, 20);
//    GrImageDraw(pContext, &kuutioImage, 3, 30);
//    GrImageDraw(pContext, &kuutioImage, 4, 40);
//    GrImageDraw(pContext, &kuutioImage, 5, 50);
//    GrImageDraw(pContext, &kuutioImage, 6, 60);
//    GrImageDraw(pContext, &kuutioImage, 7, 70);
//    GrImageDraw(pContext, &kuutioImage, 8, 80);
}

Void drawBall(tContext *pContext) {
    if (BallPos == LEFT) {
        GrCircleFill(pContext, MID_COORD - TRACK_W/4, TRACK_H/10 + 4*TRACK_H/5, ball_r);
    } else {
        GrCircleFill(pContext, MID_COORD + TRACK_W/4, TRACK_H/10 + 4*TRACK_H/5, ball_r);
    }
}

Void showMenu() {
    Display_clear(hDisplay);
    Display_print0(hDisplay, 2, 1, "RIVER SURVIVAL");
    enum cursorPosition pos;
    while (myState == MENU) {
    	pos = cursorPos;
    	switch (pos) {
    	case C_NEW_GAME:
    		break;
    	case C_CALIBRATION:
    		break;
    	case C_HIGH_SCORES:
    		break;
    	}
        Display_print0(hDisplay, 4, 3, "New game");
        Display_print0(hDisplay, 5, 3, "Calibration");
        Display_print0(hDisplay, 6, 3, "High scores");
    	Display_print0(hDisplay, pos, 1, ">");
    	Task_sleep(400000 / Clock_tickPeriod);
    	Display_print0(hDisplay, pos, 1, " ");
    	Task_sleep(400000 / Clock_tickPeriod);
    }

//    sprintf(text, "ax = %.3f", ax);
//    Display_print0(hDisplay, 0, 0, text);
//    sprintf(text, "ay = %.3f", ay);
//    Display_print0(hDisplay, 1, 0, text);
//    sprintf(text, "az = %.3f", az);
//    Display_print0(hDisplay, 2, 0, text);
//    sprintf(text, "gx = %.3f", gx);
//    Display_print0(hDisplay, 3, 0, text);
//    sprintf(text, "gy = %.3f", gy);
//    Display_print0(hDisplay, 4, 0, text);
//    sprintf(text, "gz = %.3f", gz);
//    Display_print0(hDisplay, 5, 0, text);

}

Void showGameOver() {
	char text[17];
	sprintf(text, "GAME OVER!");
	Display_print0(hDisplay, 6, 3, text);

}

// Print instructions for calibration
Void showCalibration1() {
    Display_clear(hDisplay);
    Display_print0(hDisplay, 1, 2, "CALIBRATION");
    Display_print0(hDisplay, 3, 0, "When you press");
    Display_print0(hDisplay, 4, 0, "top button, the");
    Display_print0(hDisplay, 5, 0, "device assumes");
    Display_print0(hDisplay, 6, 0, "it lays flat");
    Display_print0(hDisplay, 7, 0, "and saves data.");
    Display_print0(hDisplay, 8, 0, "Then you can");
    Display_print0(hDisplay, 9, 0, "set thresholds");
    Display_print0(hDisplay, 10, 0, "for movement.");
    while (myState == CALIBRATE1) {
    	Task_sleep(100000 / Clock_tickPeriod);
    }
}

Void showCalibration2() {
    Display_clear(hDisplay);
    char textLine[16];
    // These won't change during calibration:
    sprintf(textLine, "CALIBRATING");
    Display_print0(hDisplay, 1, 0, textLine);
    sprintf(textLine, "Tilt & jump");
    Display_print0(hDisplay, 3, 0, textLine);
    sprintf(textLine, "Right =");
    Display_print0(hDisplay, 5, 0, textLine);
    sprintf(textLine, "Left  =");
    Display_print0(hDisplay, 6, 0, textLine);
    sprintf(textLine, "Jump  =");
    Display_print0(hDisplay, 7, 0, textLine);

    // Values that will change during calibration:
    while (myState == CALIBRATE2) {
        sprintf(textLine, "%.2f", calRight);
        Display_print0(hDisplay, 5, 8, textLine);
        sprintf(textLine, "%.2f", calLeft);
        Display_print0(hDisplay, 6, 8, textLine);
        sprintf(textLine, "%.2f", calFly);
        Display_print0(hDisplay, 7, 8, textLine);
        // Update display every 0.1 seconds
        Task_sleep(100000 / Clock_tickPeriod);
    }
    // When calibration ends:
    sprintf(textLine, "DONE");
    Display_print0(hDisplay, 10, 0, textLine);
    Task_sleep(1000000 / Clock_tickPeriod);
    Display_clear(hDisplay);
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
		Display_clear(hDisplay);
		switch (myState) {
		case MENU:
			Display_clear(hDisplay);
			showMenu();
			Task_sleep(1000000 / Clock_tickPeriod); // 1 sec
			break;
		case CALIBRATE1:
			showCalibration1();
			// Task_sleeps built into showCalibration1
			break;
		case CALIBRATE2:
			showCalibration2();
			// Task_sleeps built into showCalibration2
			break;
		case GAME:
			if (gameState == ALIVE) {
				while (newTrackAvailable = 0) { // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! FOR DEBUGGING (should be "==")
					Task_sleep(50000 / Clock_tickPeriod);
				}
				newTrackAvailable = 0;
				updateBall();
				drawBall(pContext);
				drawObstacles(pContext);
				drawTrack(pContext); // after obstacles so bmps won't undo track lines
				GrFlush(pContext);
				Task_sleep(900000 / Clock_tickPeriod);
			} else if (gameState == GAMEOVER) { // Should work with just "else"
				showGameOver();
				GrFlush(pContext);
				Task_sleep(1000000 / Clock_tickPeriod); // 1sec
				myState = MENU;
				gameState = ALIVE;
			} else {
				System_abort("displayTask found erroneous gameState, aborting");
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


