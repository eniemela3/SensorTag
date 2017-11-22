/*
 * display_functions.c
 *
 *  Created on: Nov 20, 2017
 *      Author: eniem
 */
#include <ti/mw/display/Display.h>
#include <ti/mw/display/DisplayExt.h>
#include <display_functions.h>
#include <inttypes.h>
#include <stdio.h>
#include <graphics.h>
#include <game.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

enum cursorPosition cursorPos;
char inGameRXMsg[9];

void drawTrack(tContext *pContext, Display_Handle hDisplay) {
    // Draws the outline of the "river" track and displays a messaging area at the bottom
    char msg[17]; // 14 should be enough IF messages are 8 chars max
    sprintf(msg, "MSG: %s", inGameRXMsg);
    Display_print0(hDisplay, 11, 0, msg);
    GrLineDrawH(pContext, 0, 95, trackCoord.trackMaxY);
    GrLineDrawV(pContext, trackCoord.trackMinX, 0, trackCoord.trackMaxY);
    GrLineDrawV(pContext, trackCoord.trackMaxX, 0, trackCoord.trackMaxY);
}

void drawObstacles(tContext *pContext) {
    // Draws obstacles from received data
    uint32_t i;
    for (i = 0; i < 5; i++) {
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
            drawDiagObstacle(trackCoord.obstR_x, trackCoord.obst_y[i], pContext);
        }
        if (trackBuffer[i] & RIGHTSIDE_MOVING) { // Moving obstacle on right side of track
            drawDiagObstacle(trackCoord.obstRR_x, trackCoord.obst_y[i], pContext);
        }
    }
}

void drawBall(tContext *pContext) {
    // Draws the ball in its correct position
    if (ballPos == LEFT) {
        GrCircleFill(pContext, trackCoord.ballL.x, trackCoord.ballL.y, ball_r);
    } else {
        GrCircleFill(pContext, trackCoord.ballR.x, trackCoord.ballR.y, ball_r);
    }
}

void showMenu(Display_Handle hDisplay) {
    // Shows menu and handles cursor movement within
    Display_clear(hDisplay);
    Display_print0(hDisplay, 3, 1, "RIVER SURVIVAL");
    enum cursorPosition pos;
    while (myState == MENU) {
        pos = cursorPos;
        Display_print0(hDisplay, C_NEW_GAME, 3, "New game");
        Display_print0(hDisplay, C_CALIBRATION, 3, "Calibration");
        Display_print0(hDisplay, C_HIGH_SCORES, 3, "High scores");
        Display_print0(hDisplay, pos, 1, ">");
        Task_sleep(300000 / Clock_tickPeriod);
        Display_print0(hDisplay, pos, 1, " ");
        Task_sleep(300000 / Clock_tickPeriod); // Shouldn't be too long to avoid selecting something not shown
    }
}

void showGameOver(Display_Handle hDisplay) {
    // Displays the "game over" screen and score
    char text[8];
    Display_print0(hDisplay, 5, 2, "           ");
    Display_print0(hDisplay, 6, 2, " GAME OVER ");
    Display_print0(hDisplay, 7, 2, "   Score:  ");
    Display_print0(hDisplay, 8, 2, "           ");
    Display_print0(hDisplay, 9, 2, "           ");
    sprintf(text, "%d", gameScore);
    Display_print0(hDisplay, 8, 6, text);
    gameScore = 0;
}

void showCalibrateHelp(Display_Handle hDisplay) {
    // Print instructions for calibration
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

	while (calState == CALIBRATE_HELP) {
		Task_sleep(100000 / Clock_tickPeriod);
	}
}

Void showCalibrateLevel(Display_Handle hDisplay) {
    // Displays level calibration status
    Display_clear(hDisplay);
    // These won't change during calibration:
    Display_print0(hDisplay, 1, 2, "CALIBRATING");
    Display_print0(hDisplay, 3, 0, "Hold still,");
    Display_print0(hDisplay, 4, 0, "almost done!");

    // Values that will change during calibration:
    while (calLevelReady == BOOLEAN_0) {
        Task_sleep(100000 / Clock_tickPeriod);
    // For debugging the calibrated values:
//    char textLine[16];
//    Display_print0(hDisplay, 5, 0, "ax =");
//    Display_print0(hDisplay, 6, 0, "ay =");
//    Display_print0(hDisplay, 7, 0, "az =");
//    sprintf(textLine, "%.2f", ax_off);
//    Display_print0(hDisplay, 5, 8, textLine);
//    sprintf(textLine, "%.2f", ay_off);
//    Display_print0(hDisplay, 6, 8, textLine);
//    sprintf(textLine, "%.2f", az_off);
//    Display_print0(hDisplay, 7, 8, textLine);
    }

    // When calibration ends:
    Display_print0(hDisplay, 10, 0, "DONE");
    Task_sleep(1000000 / Clock_tickPeriod);
    calState = CALIBRATE_MOVEMENT;
}

Void showCalibrateMovement(Display_Handle hDisplay) {
    // Displays movement calibration status
    Display_clear(hDisplay);
    char textLine[16];
    // These won't change during calibration:
    Display_print0(hDisplay, 1, 2, "CALIBRATING");
    Display_print0(hDisplay, 3, 0, "Tilt & jump");
    Display_print0(hDisplay, 5, 0, "Right =");
    Display_print0(hDisplay, 6, 0, "Left  =");
    Display_print0(hDisplay, 7, 0, "Jump  =");
    Display_print0(hDisplay, 10, 0, "PRESS TO FINISH");

    // Values that will change during calibration:
    while (calState == CALIBRATE_MOVEMENT) {
        sprintf(textLine, "%.2f", calRight);
        Display_print0(hDisplay, 5, 8, textLine);
        sprintf(textLine, "%.2f", -calLeft);
        Display_print0(hDisplay, 6, 8, textLine);
        sprintf(textLine, "%.2f", -calFly);
        Display_print0(hDisplay, 7, 8, textLine);
        Task_sleep(100000 / Clock_tickPeriod); // Update display every 0.1 seconds
    }
    // When calibration ends:
    Display_print0(hDisplay, 10, 0, "DONE           "); // Spaces because of DISPLAY_CLEAR_NONE
    Task_sleep(1000000 / Clock_tickPeriod);

}

Void showHighScores(Display_Handle hDisplay) {
    // Displays high scores
    Display_clear(hDisplay);
    char text[17];
    Display_print0(hDisplay, 0, 3, "HIGH SCORES");
    uint32_t i;
    for (i = 0; i < 10; i++) {
        sprintf(text, "%d", highScores[i]);
        Display_print0(hDisplay, i+2, 4, text);
    }

	while (myState == HIGHSCORES) {
		Task_sleep(100000 / Clock_tickPeriod);
	}
}
