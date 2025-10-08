#include "library.h"

#define PERMUTATION_SIZE 256
static int perm[PERMUTATION_SIZE * 2];

const float fovX = 2.44346f;
const float fovY = 1.41435f;

float deltaTime = 1.0f;
uint64_t lastTime = 0;

int inside(int x, int y) { return x >= sX && x < sX + sW && y >= sY && y < sY + sH; }

int randomInt(int a, int b) { return a + rand() % (b - a + 1); }
float randomFloat(float a, float b) { return a + (b - a) * ((float)rand() / (float)RAND_MAX); }
float degToRad(float deg) { return deg * (M_PI / 180.0f); }
float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
float lerp(float t, float a, float b) { return a + t * (b - a); }

void setPixelRaw(uint x, uint8_t* row, int color){
    int bitIndex = 7 - (x % 8);
    uint8_t mask = 1 << bitIndex;

    if (color)
        row[x / 8] |= mask;
    else
        row[x / 8] &= ~mask;
}

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