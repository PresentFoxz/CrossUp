#ifndef LIBRARY_H
#define LIBRARY_H

#include "pd_api.h"
extern PlaydateAPI* pd;

#define BASE_FPS 30

#define MAX_ENTITIES 240

#define substeps 4
#define worldUnit 16.0f

#define sX  0
#define sY  0
#define sW  400
#define sH  240
#define sW_H (sW / 2)
#define sH_H (sH / 2)

#define MAX_TRIS 0

#define FP_SHIFT 16
#define FP_ONE   (1 << FP_SHIFT)
static inline void* pd_realloc(void* ptr, size_t size) { return pd->system->realloc(ptr, size); }
static inline void* pd_malloc(size_t size) { return pd->system->realloc(NULL, size); }
static inline void pd_free(void* ptr) { pd->system->realloc(ptr, 0); }

#if TARGET_PLAYDATE
#define INLINE static __attribute__((always_inline)) inline
#define ALIGNED_32 __attribute__((aligned(32)))
#define EXPECTED(t) __builtin_expect((t),1)
#define NOT_EXPECTED(t) __builtin_expect((t),0)
#else
#define INLINE static inline
#define ALIGNED_32
#define EXPECTED(t) (t)
#define NOT_EXPECTED(t) (t)
#endif

#include "structs.h"
#include "mesh.h"

extern int ambientLight;
#define M_PI 3.14159265358979323846f
#define TWO_PI 6.2831853f

static inline int min(int a, int b) { return (a < b) ? a : b; }
static inline int max(int a, int b) { return (a > b) ? a : b; }
static inline int clamp(int value, int minVal, int maxVal) { return max(minVal, min(value, maxVal)); }
static inline float fclamp(float value, float minVal, float maxVal) { return fmax(minVal, fmin(value, maxVal)); }
static inline int lerp(int a, int b, float t) { return (int)(a + t * (b - a)); }
static inline float flerp(float a, float b, float t) { return a + t * (b - a); }
static inline float sign(float value) { return (value > 0) - (value < 0); }
static inline float unitToMeter(float x) { return x / worldUnit; }
static inline float meterToUnit(float x) { return x * worldUnit; }
static inline int randomInt(int a, int b) { return a + rand() % (b - a + 1); }
static inline float randomFloat(float a, float b) { return a + (b - a) * ((float)rand() / (float)RAND_MAX); }
static inline float dot(Vect3f a, Vect3f b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline int inside(int x, int y) { return x >= sX && x < sX + sW && y >= sY && y < sY + sH; }
static inline float degToRad(float deg) { return deg * (M_PI / 180.0f); }
static inline float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }

static inline void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

static inline void fswap(float* a, float* b) {
    float temp = *a;
    *a = *b;
    *b = temp;
}

static inline void swapAdv(int* a, int* b) {
    int temp0 = a[0], temp1 = a[1];
    a[0] = b[0];
    a[1] = b[1];
    b[0] = temp0;
    b[1] = temp1;
}

static inline void fswapAdv(float* a, float* b) {
    float temp0 = a[0], temp1 = a[1];
    a[0] = b[0];
    a[1] = b[1];
    b[0] = temp0;
    b[1] = temp1;
}

static inline float fastsqrt(float x) {
    union { float f; uint32_t i; } conv = { x };
    conv.i = 0x5f3759df - (conv.i >> 1);
    float y = conv.f;
    return y * (1.5f - 0.5f * x * y * y);
}

extern int allPointsCount;
extern int entAmt;

extern InputBuffer inpBuf;
static inline void runInputBuffer() {
    PDButtons tapped, held;
    pd->system->getButtonState(&held, &tapped, NULL);

    inpBuf.UP = held & kButtonUp;
    inpBuf.DOWN = held & kButtonDown;
    inpBuf.LEFT = held & kButtonLeft;
    inpBuf.RIGHT = held & kButtonRight;

    inpBuf.A = held & kButtonA;
    inpBuf.B = held & kButtonB;
}

#endif