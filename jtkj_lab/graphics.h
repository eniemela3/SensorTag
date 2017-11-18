/*
 * graphics.h
 *
 *  Created on: Nov 18, 2017
 *      Author: eniem
 */

#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <ti/mw/display/Display.h>
#include <ti/mw/display/DisplayExt.h>
#include <inttypes.h>

void drawFlyingObstacle(uint8_t x, uint8_t y, tContext *pContext);
void drawDiagonalObstacle(uint8_t x, uint8_t y, tContext *pContext);

#endif /* GRAPHICS_H_ */
