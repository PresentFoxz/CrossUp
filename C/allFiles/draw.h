#ifndef DRAW_H
#define DRAW_H

#include "library.h"

void blitToScreen();
void changeLacing(int l0, int l1, bool bType);
void drawTriangle(int tris[3][2], int shade);
void drawImg(int screenX, int screenY, float depth, int tX, int tY, int tW, int tH, int8_t* texture, int texW, int texH, float projDist);
void drawImgNoScale(int x, int y, int tX, int tY, int tW, int tH, int8_t* texture, int texW, int texH);

#endif