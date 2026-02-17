#ifndef DRAW_H
#define DRAW_H

#include "library.h"

void skybox(int col1, int col2, int count);
#if defined(TARGET_PLAYDATE) || defined(PLAYDATE_SDK)
void setPixelRaw(uint x, uint y, int color);
void upscaleToScreen();
void initDitherByteLUT();
#else
void drawScreen();
void raylibShadeLUTCreateGrayscale();
void raylibShadeLUTCreateColor();
#endif

#endif