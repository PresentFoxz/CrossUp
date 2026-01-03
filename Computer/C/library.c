#include "library.h"

const float fovX = 2.44346f;
const float fovY = 1.41435f;

int pixSizeX = 2;
int pixSizeY = 2;

int objectCount = 0;

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

void staticLineDrawing(int p0[2], int p1[2], int color) {
    const int rand = randomInt(3, 15);
    int new[2] = {p0[0], p0[1]};
    int old[2] = {p0[0], p0[1]};

    for (int i=0; i <= rand; i++){
        int end = randomInt(3, 20);
        old[0] = new[0];
        old[1] = new[1];

        int dx = new[0] - p1[0];
        int dy = new[1] - p1[1];
        
        int dist = ((dx*dx)+(dy*dy)/2);
        if (dist < 1) { dist = 1; } else if (dist > end) { dist = end; }
        int xRand = randomInt(0, dist);
        int yRand = randomInt(0, dist);

        if (new[0] > p1[0]){
            new[0] += -xRand;
        } else {
            new[0] += xRand;
        }
        
        if (new[1] > p1[1]){
            new[1] += -yRand;
        } else {
            new[1] += yRand;
        }

        int size = randomInt(1, 4);

        if (i != rand){
            drawCustomLine(old[0], old[1], new[0], new[1], size, color);
        } else {
            drawCustomLine(new[0], new[1], p1[0], p1[1], size, color);
        }
    }
}

void drawCustomLine(int x1, int y1, int x2, int y2, int thickness, int type) {
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;

    int half = thickness / 2;

    while (1) {
        for (int oy = -half; oy <= half; oy++) {
            for (int ox = -half; ox <= half; ox++) {
                int px = x1 + ox;
                int py = y1 + oy;
                if (px < 0 || px >= sW || py < 0 || py >= sH) continue;
                DrawPixel(px, py, type ? WHITE : BLACK);
            }
        }

        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}