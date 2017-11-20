/*
 * display_functions.h
 *
 *  Created on: Nov 20, 2017
 *      Author: eniem
 */

#ifndef DISPLAY_FUNCTIONS_H_
#define DISPLAY_FUNCTIONS_H_

#include <ti/mw/display/Display.h>
#include <ti/mw/display/DisplayExt.h>
#include <inttypes.h>
#include <stdio.h>
#include <graphics.h>
#include <game.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

// Cursor's position in the menu
enum cursorPosition {C_NEW_GAME=5, C_CALIBRATION, C_HIGH_SCORES};
extern enum cursorPosition cursorPos;

extern char message[9];

void drawTrack(tContext *pContext, Display_Handle hDisplay);
void drawObstacles(tContext *pContext);
void drawBall(tContext *pContext);

void showMenu(Display_Handle hDisplay);
void showGameOver(Display_Handle hDisplay);
void showCalibration1(Display_Handle hDisplay);
void showCalibration2(Display_Handle hDisplay);
void showHighScores(Display_Handle hDisplay);

#endif /* DISPLAY_FUNCTIONS_H_ */
