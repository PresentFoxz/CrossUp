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

#endif

#include "structs.h"
#include "mesh.h"

#define M_PI 3.14159265358979323846f
#define TWO_PI 6.2831853f

#define FIXED_POINT_FRACTIONAL_BITS8 8
#define FP8_ONE (1 << FIXED_POINT_FRACTIONAL_BITS8)
#define TO_FIXED8(x) ((qfixed8_t)((x) * FP8_ONE))
#define FROM_FIXED8(x) ((float)(x) / FP8_ONE)
static inline qfixed8_t multiply8(qfixed8_t a, qfixed8_t b) { return (qfixed8_t)(((int32_t)a * (int32_t)b) >> FIXED_POINT_FRACTIONAL_BITS8); }
static inline qfixed8_t divide8(qfixed8_t a, qfixed8_t b) { return (qfixed8_t)(((int32_t)a << FIXED_POINT_FRACTIONAL_BITS8) / (int32_t)b); }

#define TO_FIXED1_15(x) ((qfixed16_t)((x) * 0x7FFF))

#define FIXED_POINT_FRACTIONAL_BITS16 16
#define FP16_ONE (1 << FIXED_POINT_FRACTIONAL_BITS16)
#define TO_FIXED16(x) ((qfixed16_t)((x) * FP16_ONE))
#define FROM_FIXED16(x) ((float)(x) / FP16_ONE)
static inline qfixed16_t multiply16(qfixed16_t a, qfixed16_t b) { return (qfixed16_t)(((int64_t)a * (int64_t)b) >> FIXED_POINT_FRACTIONAL_BITS16); }
static inline qfixed16_t divide16(qfixed16_t a, qfixed16_t b) { return (qfixed16_t)(((int64_t)a << FIXED_POINT_FRACTIONAL_BITS16) / (int64_t)b); }

#define FIXED_POINT_FRACTIONAL_BITS32 32
#define FP32_ONE ((qfixed32_t)1LL << FIXED_POINT_FRACTIONAL_BITS32)
#define TO_FIXED32(x) ((qfixed32_t)((x) * (float)FP32_ONE))
#define FROM_FIXED32(x) ((float)(x) / FP32_ONE)

#define FLT_MAX8 TO_FIXED8(256.0f)
#define FLT_MAX16 TO_FIXED16(32767.0f)

#define SIGN_MASK 0x80000000u

#define BASE_FPS 30.0f

#define MAX_ENTITIES 240
#define MAX_TRIS 0

#define substeps 4
#define worldUnit 16.0f

#define rowStride 52
#define resolution 2

#define sX  0
#define sY  0
#define sW  400
#define sH  240
#define sW_H  (sW / 2)
#define sH_H  (sH / 2)
#define sW_L  (sW / resolution)
#define sH_L  (sH / resolution)

extern int8_t* scnBuf;

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