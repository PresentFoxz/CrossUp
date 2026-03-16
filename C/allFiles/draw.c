#include "draw.h"

static ALIGNED_32 uint8_t _screen[sW * sH] = {0};
static bool anythingImaged = false;
InputBuffer inpBuf = {0};

#define INTERLACE_HEIGHT  4
static int interlaceFrame = 0;
static int frameCount = 0;

uint8_t* src;
uint8_t* hwscreen;

const uint32_t ordered_dither4x4[] = {
    0xa52d870f,
    0x69e14bc3,
    0x961eb43c,
    0x5ad278f0,
};

static void hline(int x1, int x2, int y, uint8_t color) {
    if (y < 0 || y >= sH) return;

    if (x1 < 0) x1 = 0;
    if (x2 >= sW) x2 = sW - 1;
    if (x2 < x1) return;

    memset(_screen + y * sW + x1, color, x2 - x1 + 1);
}

INLINE uint32_t __SUBTEST_DUAL(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1) {
    #if TARGET_PLAYDATE
            uint32_t result0, result1;
            __asm volatile (
            "\
            rev     %0, %2         \n\t\
            rev     %1, %4         \n\t\
            usub8   %0, %0, %3     \n\t\
            mrs     %0, APSR       \n\t\
            usub8   %1, %1, %5     \n\t\
            lsr     %0, %0, #12    \n\t\
            mrs     %1, APSR       \n\t\
            and     %0, %0, #240   \n\t\
            ubfx    %1, %1, #16, #4 \n\t\
            "
                : "=r" (result0), "=r" (result1)
                : "r" (x0), "r" (y0), "r" (x1), "r" (y1)
                : "cc");
            return result0 | result1;
    #else
            uint32_t result = 0;
            if ((x0 & 0xff) >= (y0 & 0xff)) result |= 128;
            if ((x0 & 0xff00) >= (y0 & 0xff00)) result |= 64;
            if ((x0 & 0xff0000) >= (y0 & 0xff0000)) result |= 32;
            if ((x0 & 0xff000000) >= (y0 & 0xff000000)) result |= 16;
            if ((x1 & 0xff) >= (y1 & 0xff)) result |= 8;
            if ((x1 & 0xff00) >= (y1 & 0xff00)) result |= 4;
            if ((x1 & 0xff0000) >= (y1 & 0xff0000)) result |= 2;
            if ((x1 & 0xff000000) >= (y1 & 0xff000000)) result |= 1;
    
            return result;
    #endif
}

void plotPixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= sW || y < 0 || y >= sH) return;
    
    uint8_t brightness = color + ((interlaceFrame & 1) << 3);

    _screen[y * sW + x] = brightness;
    anythingImaged = true;
}

void blitToScreen() {
    hwscreen = pd->graphics->getFrame();
    for (int y = interlaceFrame; y < sH; y += 2) {
        for (int xbyte = 0; xbyte < sW / 8; xbyte++) {
            uint8_t* dst = hwscreen + y * LCD_ROWSIZE + xbyte;
            uint8_t* row = _screen + y * sW + xbyte * 8;

            uint32_t pixels0 = row[0] | row[1] << 8 | row[2] << 16 | row[3] << 24;
            uint32_t pixels1 = row[4] | row[5] << 8 | row[6] << 16 | row[7] << 24;

            const uint32_t threshold = ordered_dither4x4[y & 3];
            *dst = __SUBTEST_DUAL(pixels0, threshold, pixels1, threshold);
        }
    }

    pd->graphics->markUpdatedRows(interlaceFrame, LCD_ROWS - 1);
    for (int y = interlaceFrame; y < sH; y += INTERLACE_HEIGHT) { memset(_screen + y * sW, 0, sW); }
    interlaceFrame = (interlaceFrame + 1) % INTERLACE_HEIGHT;
}

void skybox(int col1, int col2, int count) {
    for (int y = 0; y < sH; y++) {
        if ((y & 1) != interlaceFrame) continue;

        for (int x = 0; x < sW; x++) {
            int checkerX = x / count;
            int checkerY = y / count;

            if ((checkerX + checkerY) % 2 == 0)
                _screen[y * sW + x] = col1;
            else
                _screen[y * sW + x] = col2;
        }
    }
}

void drawTriangle(int tris[3][2], int shade) {
    uint8_t color = shade;

    int temp[2];
    if (tris[1][1] < tris[0][1]) { temp[0]=tris[0][0]; temp[1]=tris[0][1]; tris[0][0]=tris[1][0]; tris[0][1]=tris[1][1]; tris[1][0]=temp[0]; tris[1][1]=temp[1]; }
    if (tris[2][1] < tris[0][1]) { temp[0]=tris[0][0]; temp[1]=tris[0][1]; tris[0][0]=tris[2][0]; tris[0][1]=tris[2][1]; tris[2][0]=temp[0]; tris[2][1]=temp[1]; }
    if (tris[2][1] < tris[1][1]) { temp[0]=tris[1][0]; temp[1]=tris[1][1]; tris[1][0]=tris[2][0]; tris[1][1]=tris[2][1]; tris[2][0]=temp[0]; tris[2][1]=temp[1]; }

    int x0=tris[0][0], y0=tris[0][1];
    int x1=tris[1][0], y1=tris[1][1];
    int x2=tris[2][0], y2=tris[2][1];

    int dy01 = y1 - y0;
    int dy12 = y2 - y1;
    int dy02 = y2 - y0;

    if (dy02 == 0) return;

    float dx02 = (float)(x2 - x0) / dy02;
    float dx01 = 0;
    float dx12 = 0;

    if (dy01 != 0) dx01 = (float)(x1 - x0) / dy01;
    if (dy12 != 0) dx12 = (float)(x2 - x1) / dy12;

    float xA = x0;
    float xB = x0;

    int y;
    for (y = y0; y < y1; y++) {
        if (y >= 0 && y < sH) {
            int xLeft  = (int)(xA < xB ? xA + 0.5f : xB + 0.5f);
            int xRight = (int)(xA > xB ? xA + 0.5f : xB + 0.5f);

            hline(xLeft, xRight, y, color);
        }
        xA += dx02;
        xB += dx01;
    }

    xB = x1;

    for (; y <= y2; y++) {
        if (y >= 0 && y < sH) {
            int xLeft  = (int)(xA < xB ? xA + 0.5f : xB + 0.5f);
            int xRight = (int)(xA > xB ? xA + 0.5f : xB + 0.5f);

            hline(xLeft, xRight, y, color);
        }
        xA += dx02;
        xB += dx12;
    }
}

void drawLineFast(int x0, int y0, int x1, int y1, uint8_t color, int thickness) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    int half = thickness / 2;

    while (1) {
        for (int iy = -half; iy <= half; iy++) {
            int py = y0 + iy;
            if (py < 0 || py >= sH) continue;
            for (int ix = -half; ix <= half; ix++) {
                int px = x0 + ix;
                plotPixel(px, py, color);
            }
        }
        anythingImaged = true;

        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }
}

void drawImg(int screenX, int screenY, float depth, int tX, int tY, int tW, int tH, int8_t* texture, int texW, int texH, float projDist) {
    int minX = tW < tX ? tW : tX;
    int maxX = tW > tX ? tW : tX;
    int minY = tH < tY ? tH : tY;
    int maxY = tH > tY ? tH : tY;

    if (minX >= texW || minY >= texH) return;
    if (maxX >= texW) maxX = texW - 1;
    if (maxY >= texH) maxY = texH - 1;

    if (depth < 0.001f) depth = 0.001f;
    float scale = (projDist / depth);

    int spriteW = maxX - minX + 1;
    int spriteH = maxY - minY + 1;

    if (spriteW <= 0 || spriteH <= 0) return;

    int scaledW = (int)(spriteW * scale);
    int scaledH = (int)(spriteH * scale);

    if (scaledW <= 0 || scaledH <= 0) return;

    int drawX = screenX - scaledW / 2.0f;
    int drawY = screenY - scaledH / 1.2f;

    for (int j = 0; j < scaledH; j++) {
        for (int i = 0; i < scaledW; i++) {
            int gx = (drawX + i);
            int gy = (drawY + j);

            if (gx < 0 || gy < 0 || gx >= sW || gy >= sH) continue;

            int texX = minX + (i * spriteW) / scaledW;
            int texY = minY + (j * spriteH) / scaledH;

            int8_t color = texture[texY * texW + texX];
            if (color != -1) plotPixel(gx, gy, color);
        }
    }
}

void drawImgNoScale(int x, int y, int tX, int tY, int tW, int tH, int8_t* texture, int texW, int texH) {
    if (tX >= texW || tY >= texH) return;
    if (tW >= texW) tW = texW - 1;
    if (tH >= texH) tH = texH - 1;

    int regionW = tW - tX;
    int regionH = tH - tY;

    int drawX = x - regionW / 2;
    int drawY = y - regionH / 2;
    
    for (int j = 0; j < regionH; j++) {
        for (int i = 0; i < regionW; i++) {
            int texX = (tX + i);
            int texY = (tY + j);

            int8_t color = texture[texY * texW + texX];
            int gx = drawX + i;
            int gy = drawY + j;

            if (gx < 0 || gy < 0 || gx >= sW || gy >= sH) continue;
            plotPixel(gx, gy, color);
        }
    }
}