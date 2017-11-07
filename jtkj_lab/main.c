// turha kommentti
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

// Note: if you edit STACKSIZEs, remember to edit them in main too
#define STACKSIZE_commTask 2048
Char commTaskStack[STACKSIZE_commTask];

#define STACKSIZE_displayTask 2048
Char displayTaskStack[STACKSIZE_displayTask];

#define STACKSIZE_MPU9250Task 4096
Char MPU9250TaskStack[STACKSIZE_MPU9250Task];

//Char labTaskStack[STACKSIZE];

// State machine
enum state { STARTUP=0, MENU, GAME, CALIBRATE };
enum state myState;

// MPU GLOBAL VARIABLES
static PIN_Handle hMpuPin;
static PIN_State MpuPinState;
static PIN_Config MpuPinConfig[] = {
    Board_MPU_POWER  | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

static float ax, ay, az, gx, gy, gz;
static uint8_t trackBuffer[5];

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

/* jtkj: Handle power button */
Void powerButtonFxn(PIN_Handle handle, PIN_Id pinId) {

    Display_clear(hDisplay);
    Display_close(hDisplay);
    Task_sleep(100000 / Clock_tickPeriod);

	PIN_close(hPowerButton);

    PINCC26XX_setWakeup(cPowerWake);
	Power_shutdown(NULL,0);
}

/* JTKJ: HERE YOUR HANDLER FOR BUTTON0 PRESS */

Void Button0Fxn(PIN_Handle handle, PIN_Id pinId) {
    PIN_setOutputValue(hLed, Board_LED0, !PIN_getOutputValue(Board_LED0) );
    myState = CALIBRATE;
}

//Void Int2Binary(uint8_t num, char *str) {
//    int len = strlen(str) - 1;
//    int i;
//    for (i = len; i >= 0; i--) {
//        if (num % 2 == 0) {
//            *(str + i) = "0";
//        }
//        else {
//            *(str + i) = "1";
//        }
//        num = num / 2;
//    }
//}

//jtkj: Communication Task
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
	char track[9];
    while (1) {
        if (GetRXFlag() == true) {
            memset(payload, 0, 16);
            Receive6LoWPAN(&SenderAddr, payload, 16);
            trackBuffer[4] = trackBuffer[3];
            trackBuffer[3] = trackBuffer[2];
            trackBuffer[2] = trackBuffer[1];
            trackBuffer[1] = trackBuffer[0];
            trackBuffer[0] = payload[0];
            System_printf("%d\n", trackBuffer[0]);
            System_printf("%d\n", trackBuffer[1]);
            System_printf("%d\n", trackBuffer[2]);
            System_printf("%d\n", trackBuffer[3]);
            System_printf("%d\n", trackBuffer[4]);
            System_flush();

            //Int2Binary(payload[0], &track);
            //System_printf("%s\n", track);
            //System_flush();
            // no sleep due to lowest priority of all tasks
        }
    }
}

////JTKJ: laboratory exercise task
//Void labTask(UArg arg0, UArg arg1) {
//
//	I2C_Handle      i2c;
//	I2C_Params      i2cParams;
//
////	jtkj: Create I2C for usage
//	I2C_Params_init(&i2cParams);
//	i2cParams.bitRate = I2C_400kHz;
//	i2c = I2C_open(Board_I2C0, &i2cParams);
//	if (i2c == NULL) {
//		System_abort("Error Initializing I2C\n");
//	}
//
////	JTKJ: SETUP BMP280 SENSOR HERE
//	bmp280_setup(&i2c);
//
////	jtkj: Init Display
//	Display_Params displayParams;
//	displayParams.lineClearMode = DISPLAY_CLEAR_BOTH;
//	Display_Params_init(&displayParams);
//
//	hDisplay = Display_open(Display_Type_LCD, &displayParams);
//	if (hDisplay == NULL) {
//		System_abort("Error initializing Display\n");
//	}
//
////	jtkj: Check that Display works
//	Display_clear(hDisplay);
//	char str[3];
//	sprintf(str, "%d", IEEE80154_MY_ADDR);
//	Display_print0(hDisplay, 7, 7, str);
//
//	double pres;
//	double temp;
//	char t[16];
//	char p[16];
////	jtkj: main loop
//	while (1) {
////		JTKJ: READ SENSOR DATA
//		bmp280_get_data(&i2c, &pres, &temp);
//
//		pres /= 100;
//
//		sprintf(p, "%.3f hPa", pres);
//		sprintf(t, "%.3f F", temp);
//		Display_clear(hDisplay);
//		Display_print0(hDisplay, 0, 0, p);
//		Display_print0(hDisplay, 1, 0, t);
//
////		jtkj: Do not remove sleep-call from here!
//		Task_sleep(1000000 / Clock_tickPeriod);
//	}
//}


// ERIKA todo: MPU gyro + kiihtyvyys
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

	// Variables for calibration
	float maxCal = 0;
	float minCal = 0;
	// Defaults if calibration is never run:
	float calLeft = -45;
	float calRight = 45;
	uint8_t calIndex = 0;
	uint8_t calWait = 0; // boolean??

	while (1) {
		mpu9250_get_data(&i2cMPU, &ax, &ay, &az, &gx, &gy, &gz);
		if (myState == MENU) {
		    // use data to move within menu
		} else if (myState == GAME) {
		    // use data to move ball
		} else if (myState == CALIBRATE) {
		    if (calWait == 0) {
		        Task_sleep(500000 / Clock_tickPeriod);
		        calWait = 1;
		    }
		    // Pick largest and smallest gy values for R & L thresholds
		    if (maxCal < gy) {
		        maxCal = gy;
		    } else if (minCal > gy) {
		        minCal = gy;
		    }
		    calIndex++;
		    if (calIndex == 200) {
		        myState = MENU;
		        calLeft = minCal;
		        calRight = maxCal;
		        calWait = 0;
		    }
		    //System_printf("Still calibrating mpu\n");
		    //System_printf("%d\n", calIndex);
		}

	}
}


// ANTTI todo: pikseligrafiikka
Void displayTask(UArg arg0, UArg arg1) {

//	Init Display
	Display_Params displayParams;
	Display_Params_init(&displayParams);
	displayParams.lineClearMode = DISPLAY_CLEAR_NONE;
//		DISPLAY_CLEAR_NONE = 0,   !< Do not clear anything before writing
//	    DISPLAY_CLEAR_LEFT,       !< Clear pixels to left of text on the line
//	    DISPLAY_CLEAR_RIGHT,      !< Clear pixels to right of text on the line
//	    DISPLAY_CLEAR_BOTH        !< Clear pixels on both sides of text

	hDisplay = Display_open(Display_Type_LCD, &displayParams);
	if (hDisplay == NULL) {
		System_abort("Error initializing Display\n");
	}

////	jtkj: Check that Display works
//	Display_clear(hDisplay);
//	char str[3];
//	sprintf(str, "%d", IEEE80154_MY_ADDR);
//	Display_print0(hDisplay, 7, 7, str);

	while (1) {
//		Display_clear(hDisplay);

//	näytölle viivoilla X
//		tContext *pContext = DisplayExt_getGrlibContext(hDisplay);
//		if (pContext) {
//
//			// Piirretään puskuriin kaksi linjaa näytön poikki x:n muotoon
//			GrLineDraw(pContext,0,0,96,96);
//			GrLineDraw(pContext,0,96,96,0);
//
//			// Piirto puskurista näytölle
//			GrFlush(pContext);
//		}
//
//		char text[17];
//		sprintf(text, "ax = %.3f", ax);
//		Display_print0(hDisplay, 0, 0, text);
//		sprintf(text, "ay = %.3f", ay);
//		Display_print0(hDisplay, 1, 0, text);
//		sprintf(text, "az = %.3f", az);
//		Display_print0(hDisplay, 2, 0, text);
//		sprintf(text, "gx = %.3f", gx);
//		Display_print0(hDisplay, 3, 0, text);
//		sprintf(text, "gy = %.3f", gy);
//		Display_print0(hDisplay, 4, 0, text);
//		sprintf(text, "gz = %.3f", gz);
//		Display_print0(hDisplay, 5, 0, text);

//		Display_print0(hDisplay, 11, 0, "0123456789ABCDEF");
//		int i;
//		for(i = 0; i < 16; i++) {
//			if(i>=11)
//				Display_print0(hDisplay, 11, i, "O");
//			else
//				Display_print0(hDisplay, i, i, "O");
////			char text[3];
////			sprintf(text, "%d", i);
////			Display_print0(hDisplay, i, i, text);
//		}

//		if (myState == MENU) {
		    // print out menu
		if (myState == GAME || myState == MENU) { // MENU temp
		    int i;
		    for (i=0; i < 5; i++) {
		        if (trackBuffer[i] & 0b00001000) {
		            Display_print0(hDisplay, i, 1, "O");
		        } else {
		            Display_print0(hDisplay, i, 1, "-");
		        }
		        if (trackBuffer[i] & 0b00010000) {
                    Display_print0(hDisplay, i, 0, "O");
		        } else {
		            Display_print0(hDisplay, i, 0, "-");
		        }
		    }

		} else if (myState == CALIBRATE) {
		    Display_clear(hDisplay);
		    char line1[16], line2[16];

		    sprintf(line1, "CALIBRATING");
		    sprintf(line2, "Tilt L & R");

		    Display_print0(hDisplay, 1, 0, line1);
		    Display_print0(hDisplay, 4, 0, line2);

		    // Wait here until calibration is done:
		    while (myState == CALIBRATE) {
		        // Check state every 0.1 seconds
		        System_printf("Still calibrating display\n");
		        Task_sleep(100000 / Clock_tickPeriod);
		    }
		    sprintf(line2, "DONE");
		    Display_print0(hDisplay, 6, 0, line2);
		    Task_sleep(1000000 / Clock_tickPeriod);
		    Display_clear(hDisplay);

		}
		// Refreshing display every second
		Task_sleep(1000000 / Clock_tickPeriod);
	}
}

Int main(void) {
    //	Task variables
	Task_Handle hMPU9250Task;
	Task_Params MPU9250TaskParams;
	Task_Handle hdisplayTask;
	Task_Params displayTaskParams;
//	Task_Handle hLabTask;
//	Task_Params labTaskParams;
	Task_Handle hCommTask;
	Task_Params commTaskParams;

	//	Initialize board
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
    displayTaskParams.stack		= &displayTaskStack;
    displayTaskParams.priority	= 3;

    hdisplayTask = Task_create(displayTask, &displayTaskParams, NULL);
    if (hdisplayTask == NULL) {
    	System_abort("displayTask create failed!");
    }


    // OPEN MPU POWER PIN
    hMpuPin = PIN_open(&MpuPinState, MpuPinConfig);
    if (hMpuPin == NULL) {
    	System_abort("Pin open failed!");
    }
    //	Init MPU9250Task
    Task_Params_init(&MPU9250TaskParams);
    MPU9250TaskParams.stackSize	= STACKSIZE_MPU9250Task;
    MPU9250TaskParams.stack		= &MPU9250TaskStack;
    MPU9250TaskParams.priority	= 2;

    hMPU9250Task = Task_create(MPU9250Task, &MPU9250TaskParams, NULL);
    if (hMPU9250Task == NULL) {
    	System_abort("MPU9250Task create failed!");
    }

////	jtkj: Init Main Task
//    Task_Params_init(&labTaskParams);
//    labTaskParams.stackSize = STACKSIZE;
//    labTaskParams.stack = &labTaskStack;
//    labTaskParams.priority=2;
//
//    hLabTask = Task_create(labTask, &labTaskParams, NULL);
//    if (hLabTask == NULL) {
//    	System_abort("Task create failed!");
//    }


    // Init communication task
    Task_Params_init(&commTaskParams);
    commTaskParams.stackSize	= STACKSIZE_commTask;
    commTaskParams.stack		= &commTaskStack;
    commTaskParams.priority		= 1;
    Init6LoWPAN();
    hCommTask = Task_create(commTask, &commTaskParams, NULL);
    if (hCommTask == NULL) {
    	System_abort("Task create failed!");
    }

    BIOS_start();

    return (0);
}

