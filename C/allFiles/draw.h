#ifndef DRAW_H
#define DRAW_H

#include "library.h"

void DrawScreen(void);
void blitToScreen();

void drawTriangle(int tris[3][2], int shade);
void drawLineFast(int x0, int y0, int x1, int y1, uint8_t color, int thickness);
void plotPixel(int x, int y, uint8_t color);
void drawImg(int screenX, int screenY, float depth, int tX, int tY, int tW, int tH, int8_t* texture, int texW, int texH, float projDist);
void drawImgNoScale(int x, int y, int tX, int tY, int tW, int tH, int8_t* texture, int texW, int texH);

#endif