//turha kommentti
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
#include <ti/drivers/PIN.h>
#include <ti/drivers/pin/PINCC26XX.h>
#include <ti/mw/display/Display.h>
#include <ti/mw/display/DisplayExt.h>

/* Board Header files */
#include "Board.h"

/* jtkj Header files */
#include "wireless/comm_lib.h"
#include "sensors/bmp280.h"

/* Task Stacks */
#define STACKSIZE 4096
Char labTaskStack[STACKSIZE];
Char commTaskStack[STACKSIZE];
Char clockStack[STACKSIZE];

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

    PIN_setOutputValue( hLed, Board_LED0, !PIN_getOutputValue( Board_LED0 ) );
    char msg[8] = "Tere";

    Send6LoWPAN(IEEE80154_SERVER_ADDR, msg, 8);
}

Void clkFxn2(UArg arg0) {
    PIN_setOutputValue( hLed, Board_LED1, !PIN_getOutputValue( Board_LED1 ) );
}
// Kellokeskeytyksen käsittelijä
Void clkFxn(UArg arg0) {
    PIN_setOutputValue( hLed, Board_LED0, !PIN_getOutputValue( Board_LED0 ) );
    /*
   char str[16];
   sprintf(str,"System time: %.5fs\n", (double)Clock_getTicks() / 100000.0);
   System_printf(str);
   System_flush();
    */
   //Display_clear(hDisplay);
   //Display_print0(hDisplay, 1, 0, str);
}

/* jtkj: Communication Task */
Void commTask(UArg arg0, UArg arg1) {

    char payload[16];
    uint16_t SenderAddr;

    // Radio to receive mode
	int32_t result = StartReceive6LoWPAN();
	if(result != true) {
		System_abort("Wireless receive mode failed");
	}

    while (1) {

    	// COMMUNICATION WHILE LOOP DOES NOT USE Task_sleep
    	// (It has lower priority than main loop)
        if (GetRXFlag() == true) {
            memset(payload, 0, 16);
            Receive6LoWPAN(&SenderAddr, payload, 16);
            System_printf(payload);
            System_flush();
        }
    }
}

/* JTKJ: laboratory exercise task
Void labTask(UArg arg0, UArg arg1) {

    I2C_Handle      i2c;
    I2C_Params      i2cParams;

    jtkj: Create I2C for usage
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2c = I2C_open(Board_I2C0, &i2cParams);
    if (i2c == NULL) {
        System_abort("Error Initializing I2C\n");
    }

    // JTKJ: SETUP BMP280 SENSOR HERE

    bmp280_setup(&i2c);

     jtkj: Init Display
    Display_Params displayParams;
	displayParams.lineClearMode = DISPLAY_CLEAR_BOTH;
    Display_Params_init(&displayParams);

    hDisplay = Display_open(Display_Type_LCD, &displayParams);
    if (hDisplay == NULL) {
        System_abort("Error initializing Display\n");
    }

     jtkj: Check that Display works
    Display_clear(hDisplay);
    char str[3];
    sprintf(str, "%d", IEEE80154_MY_ADDR);
    Display_print0(hDisplay, 7, 7, str);

    double pres;
    double temp;
    char t[16];
    char p[16];
    // jtkj: main loop
    while (1) {

    	// JTKJ: READ SENSOR DATA

        Display_clear(hDisplay);

        bmp280_get_data(&i2c, &pres, &temp);

        pres /= 100;

        sprintf(p, "%.3f hPa", pres);
        sprintf(t, "%.3f F", temp);
        Display_print0(hDisplay, 0, 0, p);
        Display_print0(hDisplay, 1, 0, t);

    	// jtkj: Do not remove sleep-call from here!
    	Task_sleep(1000000 / Clock_tickPeriod);
    }
}
*/

Void MPU9250Task(UArg arg0, UArg arg1) {
    I2C_Handle      i2c;
    I2C_Params      i2cParams;

    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2c = I2C_open(Board_I2C0, &i2cParams);
    if (i2c == NULL) {
        System_abort("Error Initializing I2C\n");
    }

    mpu9250_setup(&i2c);
    unsigned short samplerate = ?;
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;

    while (1) {
        mpu9250_get_data(&i2c, &ax, &ay, &az, &gx, &gy, &gz);
        Task_sleep(1000000 / samplerate);
    }
}

Clock_Params CreateTimer(uint8_t period, Clock_FuncPtr clkFxn) {
    // RTOS:n kellomuuttujat
    Clock_Handle clkHandle;
    Clock_Params clkParams;

    // Alustetaan kello halutusti
    Clock_Params_init(&clkParams);
    clkParams.period = 1000000 * period / Clock_tickPeriod; // 1 second
    clkParams.startFlag = TRUE;

    // Luodaan kello
    clkHandle = Clock_create((Clock_FuncPtr)clkFxn, 1000000 * period / Clock_tickPeriod, &clkParams, NULL);
    if (clkHandle == NULL) {
    System_abort("Clock creat failed");
    }
    return clkParams;
}

// ERIKA todo: MPU gyro + kiihtyvyys

// ANTTI todo: pikseligrafiikka

Int main(void) {

    // Task variables
	//Task_Handle hLabTask;
	Task_Params labTaskParams;
	Task_Handle hCommTask;
	Task_Params commTaskParams;

	Clock_Params serverTimer = CreateTimer(1, clkFxn);
	Clock_Params gameTimer = CreateTimer(2, clkFxn2);

    // Initialize board
    Board_initGeneral();
    Board_initI2C();



	/* jtkj: Power Button */
	hPowerButton = PIN_open(&sPowerButton, cPowerButton);
	if(!hPowerButton) {
		System_abort("Error initializing power button shut pins\n");
	}
	if (PIN_registerIntCb(hPowerButton, &powerButtonFxn) != 0) {
		System_abort("Error registering power button callback function");
	}

    // JTKJ: INITIALIZE BUTTON0 HERE

	hButton0 = PIN_open(&sButton0, cButton0);
	if(!hButton0) {
	    System_abort("Error initializing led button shut pins\n");
	}
	if (PIN_registerIntCb(hButton0, &Button0Fxn) != 0) {
	    System_abort("Error registering led button callback function");
	}


    /* jtkj: Init Leds */
    hLed = PIN_open(&sLed, cLed);
    if(!hLed) {
        System_abort("Error initializing LED pin\n");
    }

    /* jtkj: Init Main Task
    Task_Params_init(&labTaskParams);
    labTaskParams.stackSize = STACKSIZE;
    labTaskParams.stack = &labTaskStack;
    labTaskParams.priority=2;

    hLabTask = Task_create(labTask, &labTaskParams, NULL);
    if (hLabTask == NULL) {
    	System_abort("Task create failed!");
    }
    */
    /* jtkj: Init Communication Task
    Task_Params_init(&commTaskParams);
    commTaskParams.stackSize = STACKSIZE;
    commTaskParams.stack = &commTaskStack;
    commTaskParams.priority=1;
*/
    Task_Params_init(&serverTimer);
    /*clkParams.stackSize = STACKSIZE;
    clkParams.stack = &clkStack;
    clkParams.priority=1;*/

    Init6LoWPAN();
    
/*
    hCommTask = Task_create(commTask, &commTaskParams, NULL);
    if (hCommTask == NULL) {
    	System_abort("Task create failed!");
    }
*/
    // jtkj: Send OK to console
    System_printf("Hello world!\n");
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}

