#include "library.h"

int objectCount = 0;
int8_t* scnBuf;
InputBuffer inpBuf = {0};

int inside(int x, int y) { return x >= sX && x < sX + sW && y >= sY && y < sY + sH; }

int randomInt(int a, int b) { return a + rand() % (b - a + 1); }
float randomFloat(float a, float b) { return a + (b - a) * ((float)rand() / (float)RAND_MAX); }
float degToRad(float deg) { return deg * (M_PI / 180.0f); }
float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
float lerp(float t, float a, float b) { return a + t * (b - a); }

float unitToMeter(float x) { return x / worldUnit; }
float meterToUnit(float x) { return x * worldUnit; }

void swapInt(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}
void swapInt2(int* a, int* b) {
    int tmp0 = a[0], tmp1 = a[1];
    a[0] = b[0];
    a[1] = b[1];
    b[0] = tmp0;
    b[1] = tmp1;
}

void swapFloat(float* a, float* b) {
    float tmp = *a;
    *a = *b;
    *b = tmp;
}
void swapFloat2(float* a, float* b) {
    float tmp0 = a[0], tmp1 = a[1];
    a[0] = b[0];
    a[1] = b[1];
    b[0] = tmp0;
    b[1] = tmp1;
}

float dot(Vect3f a, Vect3f b) { return a.x*b.x + a.y*b.y + a.z*b.z; }

float fastInvSqrt(float x) {
    union { float f; uint32_t i; } conv = { x };
    conv.i = 0x5f3759df - (conv.i >> 1);
    float y = conv.f;
    return y * (1.5f - 0.5f * x * y * y);
}

int intSqrt(int x) {
    int res = 0;
    int bit = 1 << 30; // The second-to-top bit
    while (bit > x) bit >>= 2;

    while (bit != 0) {
        if (x >= res + bit) {
            x -= res + bit;
            res = (res >> 1) + bit;
        } else {
            res >>= 1;
        }
        bit >>= 2;
    }
    return res;
}