#include <ti/mw/display/Display.h>
#include <ti/mw/display/DisplayExt.h>
#include <inttypes.h>

/*
 * graphics.c
 *
 *  Created on: Nov 18, 2017
 *      Author: eniem
 */

// Defining image data
// Only 8 bit datasets were allowed into pPixel in tImage, so 16 px images had to be constructed in 4 parts:
// 1 2
// 3 4

const uint8_t flyingObstacleData1[8] = {
		0b00000111,
		0b00000011,
		0b00000001,
		0b00000001,
		0b00000001,
		0b00000001,
		0b00000001,
		0b00000001
};

const uint8_t flyingObstacleData2[8] = {
		0b11110000,
		0b11100000,
		0b11000000,
		0b11000000,
		0b11000000,
		0b11000000,
		0b11000000,
		0b11000000
};	

const uint8_t flyingObstacleData3[8] = {
		0b01111111,
		0b00111111,
		0b00000011,
		0b00000001,
		0b00000001,
		0b00000001,
		0b00000001,
		0b00000000,
		0b00000000
};

const uint8_t flyingObstacleData4[8] = {
		0b11111111,
		0b11111110,
		0b11100000,
		0b11000000,
		0b11000000,
		0b11000000,
		0b11000000,
		0b10000000,
		0b00000000
};

const uint8_t diagObstacleData1[8] = {
		0b00000111,
		0b00001000,
		0b00010000,
		0b00001000,
		0b00110000,
		0b01000000,
		0b01000000,
		0b01000000
};

const uint8_t diagObstacleData2[8] = {
		0b00000000,
		0b10110000,
		0b11001000,
		0b00000100,
		0b00000100,
		0b00000100,
		0b00001000,
		0b00000110
};

const uint8_t diagObstacleData3[8] = {
		0b00100000,
		0b00100000,
		0b00011001,
		0b00000111,
		0b00000010,
		0b01001100,
		0b00110000,
		0b00000000
};									   

const uint8_t diagObstacleData4[8] = {
		0b00000001,
		0b00000001,
		0b10000001,
		0b01100010,
		0b00011100,
		0b00000000,
		0b00000000,
		0b00000000
};

uint32_t imgPalette[] = {0, 0xFFFFFF};

// Defining image components

const tImage flyingObstacle1 = {
		.BPP = IMAGE_FMT_1BPP_UNCOMP,
		.NumColors = 2,
		.XSize = 1,
		.YSize = 8,
		.pPalette = imgPalette,
		.pPixel = flyingObstacleData1
};

const tImage flyingObstacle2 = {
		.BPP = IMAGE_FMT_1BPP_UNCOMP,
		.NumColors = 2,
		.XSize = 1,
		.YSize = 8,
		.pPalette = imgPalette,
		.pPixel = flyingObstacleData2
};

const tImage flyingObstacle3 = {
		.BPP = IMAGE_FMT_1BPP_UNCOMP,
		.NumColors = 2,
		.XSize = 1,
		.YSize = 8,
		.pPalette = imgPalette,
		.pPixel = flyingObstacleData3
};

const tImage flyingObstacle4 = {
		.BPP = IMAGE_FMT_1BPP_UNCOMP,
		.NumColors = 2,
		.XSize = 1,
		.YSize = 8,
		.pPalette = imgPalette,
		.pPixel = flyingObstacleData4
};

const tImage diagObstacle1 = {
		.BPP = IMAGE_FMT_1BPP_UNCOMP,
		.NumColors = 2,
		.XSize = 1,
		.YSize = 8,
		.pPalette = imgPalette,
		.pPixel = diagObstacleData1
};

const tImage diagObstacle2 = {
		.BPP = IMAGE_FMT_1BPP_UNCOMP,
		.NumColors = 2,
		.XSize = 1,
		.YSize = 8,
		.pPalette = imgPalette,
		.pPixel = diagObstacleData2
};

const tImage diagObstacle3 = {
		.BPP = IMAGE_FMT_1BPP_UNCOMP,
		.NumColors = 2,
		.XSize = 1,
		.YSize = 8,
		.pPalette = imgPalette,
		.pPixel = diagObstacleData3
};

const tImage diagObstacle4 = {
		.BPP = IMAGE_FMT_1BPP_UNCOMP,
		.NumColors = 2,
		.XSize = 1,
		.YSize = 8,
		.pPalette = imgPalette,
		.pPixel = diagObstacleData4
};

// Draw functions

void drawFlyingObstacle(uint8_t x, uint8_t y, tContext *pContext) {
	if (pContext) {
		GrImageDraw(pContext, &flyingObstacle1, x, y);
		GrImageDraw(pContext, &flyingObstacle2, x+8, y);
		GrImageDraw(pContext, &flyingObstacle3, x, y+8);
		GrImageDraw(pContext, &flyingObstacle4, x+8, y+8);
	}
}

void drawDiagObstacle(uint8_t x, uint8_t y, tContext *pContext) {
	if (pContext) {
		GrImageDraw(pContext, &diagObstacle1, x, y);
		GrImageDraw(pContext, &diagObstacle2, x+8, y);
		GrImageDraw(pContext, &diagObstacle3, x, y+8);
		GrImageDraw(pContext, &diagObstacle4, x+8, y+8);
	}
}
