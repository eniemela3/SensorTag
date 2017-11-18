#include <ti/mw/display/Display.h>
#include <ti/mw/display/DisplayExt.h>
#include <inttypes.h>

/*
 * graphics.c
 *
 *  Created on: Nov 18, 2017
 *      Author: eniem
 */

// Image data
const uint16_t flyingObstacleData[16] = {
                                         0b0000011111110000,
                                         0b0000001111100000,
                                         0b0000000111000000,
                                         0b0000000111000000,
                                         0b0000000111000000,
                                         0b0000000111000000,
                                         0b0000000111000000,
                                         0b0111111111111111,
                                         0b0011111111111110,
                                         0b0000001111100000,
                                         0b0000000111000000,
                                         0b0000000111000000,
                                         0b0000000111000000,
                                         0b0000000111000000,
                                         0b0000000010000000,
                                         0b0000000000000000
};

const uint16_t diagObstacleData[16] = {
                                       0b0000011100000000,
                                       0b0000100010110000,
                                       0b0001000011001000,
                                       0b0000100000001000,
                                       0b0011000000000100,
                                       0b0100000000000100,
                                       0b0100000000001000,
                                       0b0100000000000110,
                                       0b0010000000000001,
                                       0b0010000000000001,
                                       0b0001100110000001,
                                       0b0000011101100010,
                                       0b0000001000011100,
                                       0b0100110000000000,
                                       0b0011000000000000,
                                       0b0000000000000000
};

uint32_t imgPalette[] = {0, 0xFFFFFF};

// Defining images
const tImage flyingObstacle = {
    .BPP = IMAGE_FMT_1BPP_UNCOMP,
    .NumColors = 2,
    .XSize = 2,
    .YSize = 16,
    .pPalette = imgPalette,
    .pPixel = flyingObstacleData
};

const tImage diagObstacle = {
    .BPP = IMAGE_FMT_1BPP_UNCOMP,
    .NumColors = 2,
    .XSize = 2,
    .YSize = 16,
    .pPalette = imgPalette,
    .pPixel = diagObstacleData
};


void drawFlyingObstacle(uint8_t x, uint8_t y, tContext *pContext) {
   if (pContext) {
      GrImageDraw(pContext, &flyingObstacle, x, y);
      GrFlush(pContext);
   }
}

void drawDiagObstacle(uint8_t x, uint8_t y, tContext *pContext) {
    if (pContext) {
          GrImageDraw(pContext, &diagObstacle, x, y);
          GrFlush(pContext);
    }
}
