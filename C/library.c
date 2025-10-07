#include "library.h"

#define PERMUTATION_SIZE 256
static int perm[PERMUTATION_SIZE * 2];

int inside(int x, int y) { return x >= sX && x < sX + sW && y >= sY && y < sY + sH; }
int pointsOnScreen(int tri[3][2]){
    int insideScreen = 0;
    for (int i = 0; i < 3; i++) {
        if (inside(tri[i][0], tri[i][1])) { insideScreen++; }
    }
    if (insideScreen > 0) { return 1; } else { return 0; }
}

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
void drawCustomLine(int x1, int y1, int x2, int y2, int size, int type) {
    int dx = x2 > x1 ? x2 - x1 : x1 - x2;
    int sx = x1 < x2 ? 1 : -1;
    int dy = y2 > y1 ? y2 - y1 : y1 - y2;
    int sy = y1 < y2 ? 1 : -1;

    int err = dx - dy;

    int newX = x1;
    int newY = y1;

    while (1) {
        for (int oy = -size; oy <= size; oy++) {
            uint8_t* row = buf + oy * 52;
            for (int ox = -size; ox <= size; ox++) {
                int px = newX + ox;
                int py = newY + oy;

                setPixelRaw(px, row, type ? 1 : 0);
            }
        }

        if (newX == x2 && newY == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            newX += sx;
        }
        if (e2 < dx) {
            err += dx;
            newY += sy;
        }
    }
}

float mfminf(float a, float b) { return (a < b) ? a : b; }
float mfmaxf(float a, float b) { return (a > b) ? a : b; }

int viewFrustrum3D(float x, float y, float z, float fovX, float fovY, float nearPlane, float farPlane) {
    if (z < nearPlane || z > farPlane) return 1;

    float halfWidth  = z * tanf(fovX * 0.5f);
    float halfHeight = z * tanf(fovY * 0.5f);

    if (x < -halfWidth || x > halfWidth) return 1;
    if (y < -halfHeight || y > halfHeight) return 1;

    return 0;
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

float dot3(const float a[3], const float b[3]) { return a[0]*b[0] + a[1]*b[1] + a[2]*b[2]; }
void sub3(const float a[3], const float b[3], float r[3]) {
    r[0] = a[0] - b[0];
    r[1] = a[1] - b[1];
    r[2] = a[2] - b[2];
}
void mul3(const float v[3], float s, float r[3]) {
    r[0] = v[0] * s;
    r[1] = v[1] * s;
    r[2] = v[2] * s;
}
void add3(const float a[3], const float b[3], float r[3]) {
    r[0] = a[0] + b[0];
    r[1] = a[1] + b[1];
    r[2] = a[2] + b[2];
}
float clampf(float x, float min, float max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}
void closestPointOnTriangle3(const float p[3], const float a[3], const float b[3], const float c[3], float out[3]) {
    float ab[3], ac[3], ap[3];
    float u, v, denom;
    
    sub3(b, a, ab);
    sub3(c, a, ac);
    sub3(p, a, ap);
    
    float d1 = dot3(ab, ap);
    float d2 = dot3(ac, ap);
    float d3 = dot3(ab, ab);
    float d4 = dot3(ab, ac);
    float d5 = dot3(ac, ac);
    
    denom = d3 * d5 - d4 * d4;
    u = (d5*d1 - d4*d2) / denom;
    v = (d3*d2 - d4*d1) / denom;
    
    // Clamp to triangle
    u = clampf(u, 0.0f, 1.0f);
    v = clampf(v, 0.0f, 1.0f);
    if (u + v > 1.0f) {
        float t = u + v;
        u /= t;
        v /= t;
    }
    
    float temp1[3], temp2[3];
    mul3(ab, u, temp1);
    mul3(ac, v, temp2);
    add3(temp1, temp2, temp1);
    add3(a, temp1, out);
}