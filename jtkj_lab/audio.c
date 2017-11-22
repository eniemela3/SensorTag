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
#include "audio.h"
#include "game.h"


void playNote(uint16_t note, uint16_t ms) {
    buzzerSetFrequency(note);
    Task_sleep(ms * 1000 / Clock_tickPeriod);
}

void endNote() {
    buzzerSetFrequency(0);
}

void playGonnaFlyNow() { // 94 bpm -> 160 ms/8th note
	enum mainState state = myState;
	playNote(NOTE_C5, 300); endNote();
	Task_sleep(20000 / Clock_tickPeriod);

	playNote(NOTE_C5, 300); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_C5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_C5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_C5, 300); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_C5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_C5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);

	playNote(NOTE_E5, 300); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_C5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_C5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_C5, 300); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_E5, 300); endNote();
	Task_sleep(20000 / Clock_tickPeriod);

	playNote(NOTE_E5, 300); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_E5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_E5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_E5, 300); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_E5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_E5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);

	playNote(NOTE_G5, 300); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_E5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_E5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_E5, 620); endNote();
	Task_sleep(20000 / Clock_tickPeriod);

	if (myState != state) {
		return;
	}

	playNote(NOTE_D5, 300); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_D5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_D5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_D5, 300); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_D5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_D5, 300); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_D5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_D5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	Task_sleep(160000 / Clock_tickPeriod);
	playNote(NOTE_F4, 620); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_E4, 1280); endNote();
	Task_sleep(640000 / Clock_tickPeriod);

	if (myState != state) {
		return;
	}

	playNote(NOTE_E5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_G5, 460); endNote();
	Task_sleep(20000 / Clock_tickPeriod);

	playNote(NOTE_A5, 1900); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_A5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_B5, 460); endNote();
	Task_sleep(20000 / Clock_tickPeriod);

	playNote(NOTE_E5, 1900); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_E5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_G5, 460); endNote();
	Task_sleep(20000 / Clock_tickPeriod);

	playNote(NOTE_A5, 1900); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_A5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_B5, 460); endNote();
	Task_sleep(20000 / Clock_tickPeriod);

	playNote(NOTE_E5, 2560); endNote();

	if (myState != state) {
		return;
	}

	Task_sleep(320000 / Clock_tickPeriod);
	playNote(NOTE_D5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_C5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_D5, 460); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_C5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);

	playNote(NOTE_D5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_E5, 1120); endNote();

	Task_sleep(320000 / Clock_tickPeriod);
	playNote(NOTE_C5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_C5, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_B4, 300); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_B4, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_A4, 300); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_A4, 140); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_G4, 620); endNote();
	Task_sleep(20000 / Clock_tickPeriod);
	playNote(NOTE_F5, 300); endNote();
	Task_sleep(20000 / Clock_tickPeriod);

	playNote(NOTE_E5, 2560); endNote();
	Task_sleep(2240000 / Clock_tickPeriod);
}

void playRiverside1() {
	playNote(NOTE_C5, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_C5, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_C5, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_C5, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);

	playNote(NOTE_F5, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_C5, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_DS5, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_C5, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);

	playNote(NOTE_D5, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_DS5, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_C5, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_D5, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);

	playNote(NOTE_GS4, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_G4, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_FS4, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);
}

void playRiverside2() {
	playNote(NOTE_C6, 120);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_C6, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_C6, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_C6, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);

	playNote(NOTE_F6, 120); endNote();
	playNote(NOTE_F6, 120); endNote();
	playNote(NOTE_C6, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_DS6, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_C6, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);

	playNote(NOTE_C6, 120); endNote();
	playNote(NOTE_D6, 120); endNote();
	playNote(NOTE_DS6, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_C6, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_D6, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);

	playNote(NOTE_GS5, 120); endNote();
	playNote(NOTE_GS5, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_G5, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_FS5, 120); endNote();
	Task_sleep(120000 / Clock_tickPeriod);
}

void playRiverside3() {
	playNote(NOTE_DS6, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	playNote(NOTE_DS6, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	playNote(NOTE_DS6, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	playNote(NOTE_DS6, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_DS6, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);

	playNote(NOTE_GS6, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	playNote(NOTE_GS6, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	playNote(NOTE_DS6, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_FS6, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_DS6, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);

	playNote(NOTE_DS6, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	playNote(NOTE_F6, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	playNote(NOTE_FS6, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_DS6, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_F6, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);

	playNote(NOTE_B5, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	playNote(NOTE_B5, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_AS5, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_A5, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
}

void playRiverside() {
	enum mainState state = myState;
	playRiverside1();
	if (myState != state) {
		return;
	}
	playRiverside1();
	if (myState != state) {
		return;
	}
	playRiverside2();
	if (myState != state) {
		return;
	}
	playRiverside3();
}

void playOneA() {
	playNote(NOTE_A4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_G4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_F4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_E4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_D4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_D4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_D4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_D4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_D4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_D4, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_D4, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
}

void playOneB() {
	playNote(NOTE_A4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_G4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_F4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_E4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_F4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_F4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_F4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_F4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_F4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_F4, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_F4, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
}

void playOneC() {
	playNote(NOTE_A4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_G4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_F4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_E4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_G4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_G4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_G4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_G4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_G4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_G4, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_G4, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
}

void playOneD() {
	playNote(NOTE_A4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_B4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_C5, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_D5, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_C5, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_B4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_A4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_G4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_E4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_D4, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_E4, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
}

void playOne() {
	enum mainState state = myState;
	playOneA();
	if ((myState != state) || (volume == MUTE)) {
		return;
	}
	playOneA();
	if ((myState != state) || (volume == MUTE)) {
		return;
	}
	playOneB();
	if ((myState != state) || (volume == MUTE)) {
		return;
	}
	playOneC();
	if ((myState != state) || (volume == MUTE)) {
		return;
	}
	playOneB();
	if ((myState != state) || (volume == MUTE)) {
		return;
	}
	playOneC();
	if ((myState != state) || (volume == MUTE)) {
		return;
	}
	playOneD();
	if ((myState != state) || (volume == MUTE)) {
		return;
	}
	playOneD();
}

void playEpicSaxGuy() {
	enum mainState state = myState;
	playNote(NOTE_AS4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(720000 / Clock_tickPeriod);

	playNote(NOTE_AS4, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_AS4, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	playNote(NOTE_AS4, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	playNote(NOTE_GS4, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(10000 / Clock_tickPeriod);
	playNote(NOTE_AS4, 350); endNote();
	Task_sleep(10000 / Clock_tickPeriod);

	playNote(NOTE_AS4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(720000 / Clock_tickPeriod);

	if (myState != state) {
		return;
	}

	playNote(NOTE_AS4, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(120000 / Clock_tickPeriod);
	playNote(NOTE_AS4, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	playNote(NOTE_AS4, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	playNote(NOTE_GS4, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(10000 / Clock_tickPeriod);
	playNote(NOTE_AS4, 350); endNote();
	Task_sleep(10000 / Clock_tickPeriod);

	playNote(NOTE_AS4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(480000 / Clock_tickPeriod);

	if (myState != state) {
		return;
	}

	playNote(NOTE_CS5, 480); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	playNote(NOTE_AS4, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(360000 / Clock_tickPeriod);
	playNote(NOTE_GS4, 480); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	playNote(NOTE_FS4, 110); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	Task_sleep(360000 / Clock_tickPeriod);
	playNote(NOTE_DS4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);

	playNote(NOTE_DS4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	playNote(NOTE_F4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	playNote(NOTE_FS4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);
	playNote(NOTE_DS4, 230); endNote();
	Task_sleep(10000 / Clock_tickPeriod);

}
