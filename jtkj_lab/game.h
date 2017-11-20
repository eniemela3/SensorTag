/*
 * game.h
 *
 *  Created on: Nov 20, 2017
 *      Author: eniem
 */

#ifndef GAME_H_
#define GAME_H_

#include <inttypes.h>
#include <display_functions.h>

// Pixel graphics
#define OBSTACLE_W 16
#define OBSTACLE_H 16
#define TRACK_H 86
#define TRACK_W 32
#define MID_COORD 48
#define BALL_R_FLYING 6
#define BALL_R 4
static uint8_t ball_r = BALL_R;

// New pixel graphics, hard coded because pixel graphics don't allow for a modular design
#define RIGHT_SIDE_X 70
#define RIGHT_LANE_X 50
#define LEFT_LANE_X 34
#define LEFT_SIDE_X 18
#define ROW_0_Y 8
#define ROW_1_Y 24
#define ROW_2_Y 40
#define ROW_3_Y 56
#define ROW_4_Y 72

// Sensor thresholds (before possible calibration)
#define ACC_HI_THERSHOLD 0.25
#define ACC_LO_THERSHOLD -0.25

// Flying state machine
enum flyingState {CAN_FLY=0, FLYING, CANT_FLY};
extern enum flyingState flyState;

// Object's position on the track
enum trackPosition {LEFT=0, RIGHT};
extern enum trackPosition BallPos;

enum mainState {STARTUP=0, MENU, GAME, CALIBRATE1, CALIBRATE2, HIGHSCORES};
extern enum mainState myState;

// Game state machine
enum gameStatus {ALIVE=1, GAMEOVER=0};
extern enum gameStatus gameState;

// Obstacle's type and position on track as bitmask
enum obstacle {RIGHTSIDE_MOVING=2, RIGHTLANE_MOVING=4, RIGHTLANE_STATIC=8, LEFTLANE_STATIC=16, LEFTLANE_MOVING=32, LEFTSIDE_MOVING=64};
extern enum obstacle obstaclePos;

// xy-coordinate on the display
struct point {
   uint8_t x; // x = [0, 95]
   uint8_t y; // y = [0, 95]
};

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
trackCoordinates trackCoord;

// Default acceleration thresholds if calibration is never run:
extern float calLeft;
extern float calRight;
extern float calFly;

extern float ax, ay, az, gx, gy, gz;
extern float ax_off, ay_off, az_off, gx_off, gy_off, gz_off;

extern uint8_t trackBuffer[5];
extern uint8_t gameScore;
extern uint8_t highScores[10];
extern uint8_t newTrackAvailable;

void updateBall();
void moveBall();
void updateHighScores();

#endif /* GAME_H_ */
