#ifndef DRAW_H
#define DRAW_H

#include "library.h"

void blitToScreen();
void changeLacing(int l0, int l1, bool bType);
void drawTriangle(int tris[3][2], uint8_t shade);
void drawTexturedTriangle(int tris[3][2], Vector2f uvs[3], float z0, float z1, float z2, int tx, int ty, int tw, int th, int* Texture, int TexW, int TexH);
void drawImg(int screenX, int screenY, float depth, int tX, int tY, int tW, int tH, int* texture, int texW, int texH, float projDist);
void drawImgNoScale(int x, int y, int tX, int tY, int tW, int tH, int* texture, int texW, int texH);
void drawRect(int x, int y, int w, int h, uint8_t color);
void drawLine(int x0, int y0, int x1, int y1, uint8_t color);

#endif