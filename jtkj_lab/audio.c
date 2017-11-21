/*
 * audio.c
 *
 *  Created on: Nov 21, 2017
 *      Author: eniem
 */

// Buzzer header
#include "buzzer.h"
#include <inttypes.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>


void playNote(uint16_t note, uint16_t ms) {
    buzzerSetFrequency(note);
    Task_sleep(ms * 1000 / Clock_tickPeriod);
}

void endNote() {
    buzzerSetFrequency(0);
}
