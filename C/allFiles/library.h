#ifndef LIBRARY_H
#define LIBRARY_H

#if defined(TARGET_PLAYDATE) || defined(PLAYDATE_SDK)
#include "pd_api.h"

extern PlaydateAPI* pd;
extern uint8_t* buf;

static inline void* pd_realloc(void* ptr, size_t size) {
    if (!pd) return NULL;
    return pd->system->realloc(ptr, size);
}

static inline void* pd_malloc(size_t size) {
    if (!pd) return NULL;
    return pd->system->realloc(NULL, size);
}

static inline void pd_free(void* ptr) {
    if (!pd) return;
    pd->system->realloc(ptr, 0);
}

#define resolution 2

#else
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <direct.h>
#include "raylib.h"

#define pd_realloc(ptr, size) realloc(ptr, size)
#define pd_malloc(size)       malloc(size)
#define pd_free(ptr)          free(ptr)

#define resolution 1

#endif

#include "structs.h"
#include "mesh.h"

#define M_PI 3.14159265358979323846f
#define TWO_PI 6.2831853f

#define FP24_8_ONE (1 << 8)
#define TO_FIXED24_8(x) ((qfixed24x8_t)((x) * FP24_8_ONE))
#define FROM_FIXED24_8(x) ((float)(x) / FP24_8_ONE)

#define BASE_FPS 30.0f

#define MAX_ENTITIES 240
#define MAX_TRIS 0

#define substeps 4
#define worldUnit 16.0f

#define rowStride 52

#define sX  0
#define sY  0
#define sW  400
#define sH  240
#define sW_H  (sW / 2)
#define sH_H  (sH / 2)
#define sW_L  (sW / resolution)
#define sH_L  (sH / resolution)

extern int8_t* scnBuf;
#define setPixScnBuf(x,y,c) scnBuf[(y) * sW_L + (x)] = (c)

float unitToMeter(float x);
float meterToUnit(float x);

int randomInt(int a, int b);
float randomFloat(float a, float b);
float degToRad(float deg);
float lerp(float t, float a, float b);

void swapInt(int* a, int* b);
void swapInt2(int* a, int* b);
void swapFloat(float* a, float* b);
void swapFloat2(float* a, float* b);

float dot(Vect3f a, Vect3f b);
float fastInvSqrt(float x);
void staticLineDrawing(int p0[2], int p1[2], int color);
void drawCustomLine(int x1, int y1, int x2, int y2, int thickness, int type);

extern int allPointsCount;
extern int entAmt;
extern int mapIndex;

#endif