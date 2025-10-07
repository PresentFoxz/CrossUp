#ifndef LIBRARY_H
#define LIBRARY_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "pd_api.h"
#include "structs.h"

extern PlaydateAPI* pd;

#define M_PI 3.14159265358979323846f
#define TWO_PI 6.2831853f

#define SCREEN_W 400
#define SCREEN_H 240

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN3(a,b,c) (MIN(MIN((a),(b)),(c)))
#define MAX3(a,b,c) (MAX(MAX((a),(b)),(c)))

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

static inline int allNonNegative(float a, float b, float c) {
    uint32_t ai = *(uint32_t*)&a;
    uint32_t bi = *(uint32_t*)&b;
    uint32_t ci = *(uint32_t*)&c;
    return ((ai | bi | ci) & SIGN_MASK) == 0;
}

#define sX  0
#define sY  0
#define sW  400
#define sH  240
#define sW_H  (sW / 2)
#define sH_H  (sH / 2)

extern uint8_t* buf;
extern const int blockVerts[8][3];
extern const int blockTris[12][4];
extern float pCollisionPos[3];
extern float pColPoints[4][3];
extern int substeps;

int pointsOnScreen(int tri[3][2]);

int randomInt(int a, int b);
float randomFloat(float a, float b);
float degToRad(float deg);
float lerp(float t, float a, float b);
void setPixelRaw(uint x, uint8_t* row, int color);
int viewFrustrum3D(float x, float y, float z, float fovX, float fovY, float nearPlane, float farPlane);
float mfminf(float a, float b);
float mfmaxf(float a, float b);
void drawCustomLine(int x1, int y1, int x2, int y2, int size, int type);
void closestPointOnTriangle3(const float p[3], const float a[3], const float b[3], const float c[3], float out[3]);
float dot(Vect3f a, Vect3f b);

void swapInt(int* a, int* b);
void swapInt2(int* a, int* b);
void swapFloat(float* a, float* b);
void swapFloat2(float* a, float* b);

#endif