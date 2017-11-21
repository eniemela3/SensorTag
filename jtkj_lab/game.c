/*
 * game.c
 *
 *  Created on: Nov 20, 2017
 *      Author: eniem
 */

#include "game.h"
#include <inttypes.h>
#include "display_functions.h"

enum mainState myState;
enum gameStatus gameState;
enum obstaclePosition obstaclePos;
enum trackPosition ballPos;
enum flyingState flyState;

trackCoordinates trackCoord;
float calLeft = ACC_LO_THERSHOLD * 2;
float calRight = ACC_HI_THERSHOLD * 2;
float calFly = ACC_LO_THERSHOLD * 2;

// MPU9250 data
float ax, ay, az, gx, gy, gz; // For raw data
float ax_off = 0; // Calibration offset values, defaults for a level surface on boot
float ay_off = 0;
float az_off = -1;
float gx_off = 0;
float gy_off = 0;
float gz_off = 0;

enum diyBoolean newTrackAvailable;
enum diyBoolean calLevelReady;
uint8_t gameScore = 0; // TODO is 255 enough? :D

uint8_t highScores[10] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
uint8_t trackBuffer[5];
uint8_t ball_r = BALL_R;
void moveBall() {
    // Move ball left / right / up
    if (ax - ax_off < calLeft) { // ax_off needed?
        ballPos = LEFT;
    } else if (ax - ax_off > calRight) { // ax_off needed?
        ballPos = RIGHT;
    }
    if ((az < calFly + az_off) && (flyState == CAN_FLY)) { // az_off IS needed
        ball_r = BALL_R_FLYING;
        flyState = FLYING;
    }
}

void updateBall() {
    // This makes sure ball falls down from flying & dies when being hit
    if (gameState == ALIVE) {
        // Check if ball and obstacle overlap
        if (flyState == FLYING) {
            if (ballPos == LEFT) {
                if (trackBuffer[4] & LEFTLANE_STATIC) {
                    gameState = GAMEOVER;
                }
            } else if (ballPos == RIGHT) {
                if (trackBuffer[4] & RIGHTLANE_STATIC) {
                    gameState = GAMEOVER;
                }
            }
        } else { // flyState != FLYING
            if (ballPos == LEFT) {
                if (trackBuffer[4] & LEFTLANE_MOVING) {
                    gameState = GAMEOVER;
                }
            } else if (ballPos == RIGHT) {
                if (trackBuffer[4] & RIGHTLANE_MOVING) {
                    gameState = GAMEOVER;
                }
            }
        }
    }
    if (gameState == GAMEOVER) {
        updateHighScores(); // Not logical to execute from updateBall() TODO
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

void updateHighScores() {
    // Adds gameScore to list of high scores (if high enough)
    uint32_t i;
    uint32_t j;
    for (i = 0; i < 10; i++) {
        // System_printf("ok\n"); // debug
        if (gameScore > highScores[i]) {
            for (j = 9; j > i; j--) {
                highScores[j] = highScores[j-1];
            }
            highScores[i] = gameScore;
            return;
        }
    }
    // gameScore = 0;
}
