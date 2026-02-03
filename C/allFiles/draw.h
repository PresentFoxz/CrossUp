#ifndef DRAW_H
#define DRAW_H

#include "library.h"

#if defined(TARGET_PLAYDATE) || defined(PLAYDATE_SDK)
void multiPixl(uint gridX, uint gridY, int shade);
void setPixelLow(uint gridX, uint gridY, int shade);
#else
void drawScreen();
#endif

#endif