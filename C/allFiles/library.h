#ifndef LIBRARY_H
#define LIBRARY_H

#define BASE_FPS 30.0f

#define MAX_ENTITIES 240

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
#define MAX_TRIS 700

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

#define RAYSCREEN_WIDTH   700
#define RAYSCREEN_HEIGHT  460

#define resolution 1
#define MAX_TRIS 0

extern Texture2D screenTex;

#endif

#include "structs.h"
#include "mesh.h"

#define M_PI 3.14159265358979323846f
#define TWO_PI 6.2831853f

#define FIXED_POINT_FRACTIONAL_BITS16 16
#define FP16_ONE (1 << FIXED_POINT_FRACTIONAL_BITS16)
#define TO_FIXED16(x) ((qfixed16_t)((x) * FP16_ONE))
#define FROM_FIXED16(x) ((float)(x) / FP16_ONE)
static inline qfixed16_t multiply16(qfixed16_t a, qfixed16_t b) { return (qfixed16_t)(((int64_t)a * (int64_t)b) >> FIXED_POINT_FRACTIONAL_BITS16); }
static inline qfixed16_t divide16(qfixed16_t a, qfixed16_t b) { return (qfixed16_t)(((int64_t)a << FIXED_POINT_FRACTIONAL_BITS16) / (int64_t)b); }

#define FIXED_POINT_FRACTIONAL_BITS_24x8 8
#define FP24_8_ONE (1 << FIXED_POINT_FRACTIONAL_BITS_24x8)
#define TO_FIXED24_8(x) ((qfixed24x8_t)((x) * FP24_8_ONE))
#define FROM_FIXED24_8(x) ((float)(x) / FP24_8_ONE)
static inline qfixed24x8_t multiply24_8(qfixed24x8_t a, qfixed24x8_t b) { return (qfixed24x8_t)(((int64_t)a * (int64_t)b) >> FIXED_POINT_FRACTIONAL_BITS_24x8); }
static inline qfixed24x8_t divide24_8(qfixed24x8_t a, qfixed24x8_t b) { return (qfixed24x8_t)(((int64_t)a << FIXED_POINT_FRACTIONAL_BITS_24x8) / (int64_t)b); }

#define FIXED_POINT_FRACTIONAL_BITS32 16
#define FP32_ONE (1 << FIXED_POINT_FRACTIONAL_BITS32)
#define TO_FIXED32(x) ((qfixed32_t)((x) * FP32_ONE))
#define FROM_FIXED32(x) ((float)((x) / FP32_ONE))
static inline qfixed32_t multiply32(qfixed32_t a, qfixed32_t b) { return (qfixed32_t)(((int64_t)a * (int64_t)b) >> FIXED_POINT_FRACTIONAL_BITS32); }
static inline qfixed32_t divide32(qfixed32_t a, qfixed32_t b) { return (qfixed32_t)(((int64_t)a << FIXED_POINT_FRACTIONAL_BITS32) / (int64_t)b); }

extern int8_t* scnBuf;
#define setPixScnBuf(x,y,c) scnBuf[(y) * sW_L + (x)] = (c)

float unitToMeter(float x);
float meterToUnit(float x);

int randomInt(int a, int b);
float randomFloat(float a, float b);
float degToRad(float deg);
float lerp(float t, float a, float b);
int intSqrt(int x);

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

extern InputBuffer inpBuf;

static inline void runInputBuffer() {
    #if defined(TARGET_PLAYDATE) || defined(PLAYDATE_SDK)
    PDButtons tapped, held;
    pd->system->getButtonState(&held, &tapped, NULL);

    inpBuf.UP = held & kButtonUp;
    inpBuf.DOWN = held & kButtonDown;
    inpBuf.LEFT = held & kButtonLeft;
    inpBuf.RIGHT = held & kButtonRight;

    inpBuf.A = held & kButtonA;
    inpBuf.B = held & kButtonB;
    #else 
    inpBuf.UP = IsKeyDown(KEY_W);
    inpBuf.DOWN = IsKeyDown(KEY_S);
    inpBuf.LEFT = IsKeyDown(KEY_A);
    inpBuf.RIGHT = IsKeyDown(KEY_D);
    inpBuf.A = IsKeyDown(KEY_J);
    inpBuf.B = IsKeyDown(KEY_K);
    #endif
}

#endif