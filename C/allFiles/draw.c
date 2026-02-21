#include "draw.h"

static ALIGNED_32 uint8_t _screen[sW * sH] = {0};
static const uint8_t TRI_SHADES[4] = {0, 85, 170, 255};
static bool anythingImaged = false;
InputBuffer inpBuf = {0};

uint8_t* src;
uint8_t* hwscreen;

const uint32_t ordered_dither4x4[] = {
    0xa52d870f,
    0x69e14bc3,
    0x961eb43c,
    0x5ad278f0,
};

static void vline(int x, int y1, int y2, uint8_t color) {
    if (y1 < 0) y1 = 0;
    if (y2 >= sH) y2 = sH - 1;
    if (y2 < y1) return;

    uint8_t* dst = _screen + x * sH + y1;
    memset(dst, color, y2 - y1 + 1);

    anythingImaged = true;
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

void blitToScreen() {
    src = _screen;
    hwscreen = pd->graphics->getFrame();

    for (int j = 0; j < sW / 8; j++, src += 8 * sH) {
        uint8_t* dst = hwscreen + j;

        uint8_t* src0 = src + 0 * sH;
        uint8_t* src1 = src + 1 * sH;
        uint8_t* src2 = src + 2 * sH;
        uint8_t* src3 = src + 3 * sH;
        uint8_t* src4 = src + 4 * sH;
        uint8_t* src5 = src + 5 * sH;
        uint8_t* src6 = src + 6 * sH;
        uint8_t* src7 = src + 7 * sH;

        for (int i = 0; i < sH; i++, dst += LCD_ROWSIZE) {
            const uint32_t threshold0 = ordered_dither4x4[i & 3];

            uint32_t pixels0 = (*src0) | (*src1) << 8 | (*src2) << 16 | (*src3) << 24;
            uint32_t pixels1 = (*src4) | (*src5) << 8 | (*src6) << 16 | (*src7) << 24;

            *dst = __SUBTEST_DUAL(pixels0, threshold0, pixels1, threshold0);

            src0++; src1++; src2++; src3++;
            src4++; src5++; src6++; src7++;
        }
    }

    pd->graphics->markUpdatedRows(0, LCD_ROWS - 1);
    memset(_screen, 0, sW * sH);
}

void drawTriangle(int tris[3][2], int shade) {
    uint8_t color = TRI_SHADES[shade];

    int x0 = tris[0][0], y0 = tris[0][1];
    int x1 = tris[1][0], y1 = tris[1][1];
    int x2 = tris[2][0], y2 = tris[2][1];

    if (x1 < x0) { int tx=x0, ty=y0; x0=x1; y0=y1; x1=tx; y1=ty; }
    if (x2 < x0) { int tx=x0, ty=y0; x0=x2; y0=y2; x2=tx; y2=ty; }
    if (x2 < x1) { int tx=x1, ty=y1; x1=x2; y1=y2; x2=tx; y2=ty; }

    int dx01 = x1 - x0;
    int dx02 = x2 - x0;
    int dx12 = x2 - x1;

    for (int x = x0; x <= x2; x++) {
        if (x < 0 || x >= sW) continue;

        float tA = dx02 ? (float)(x - x0) / dx02 : 0.0f;
        float yA = y0 + (y2 - y0) * tA;

        float yB;
        if (x < x1) {
            float tB = dx01 ? (float)(x - x0) / dx01 : 0.0f;
            yB = y0 + (y1 - y0) * tB;
        } else {
            float tB = dx12 ? (float)(x - x1) / dx12 : 0.0f;
            yB = y1 + (y2 - y1) * tB;
        }
        
        int yTop = (int)(yA < yB ? yA + 0.5f : yB + 0.5f);
        int yBot = (int)(yA > yB ? yA + 0.5f : yB + 0.5f);

        if (yBot >= yTop) { 
            vline(x, yTop, yBot, color); 
        }
    }
}

// void drawImg(int screenX, int screenY, float depth, int tX, int tY, int tW, int tH, int8_t* texture, int texW, int texH, float projDist) {
//     int minX = tW < tX ? tW : tX;
//     int maxX = tW > tX ? tW : tX;
//     int minY = tH < tY ? tH : tY;
//     int maxY = tH > tY ? tH : tY;

//     if (minX >= texW || minY >= texH) return;
//     if (maxX >= texW) maxX = texW - 1;
//     if (maxY >= texH) maxY = texH - 1;

//     if (depth < 0.001f) depth = 0.001f;
//     float scale = (projDist / depth);

//     int spriteW = maxX - minX + 1;
//     int spriteH = maxY - minY + 1;

//     if (spriteW <= 0 || spriteH <= 0) return;

//     int scaledW = (int)(spriteW * scale);
//     int scaledH = (int)(spriteH * scale);

//     if (scaledW <= 0 || scaledH <= 0) return;

//     int drawX = screenX - scaledW / 2.0f;
//     int drawY = screenY - scaledH / 1.2f;

//     for (int j = 0; j < scaledH; j++) {
//         for (int i = 0; i < scaledW; i++) {
//             int gx = (drawX + i) / resolution;
//             int gy = (drawY + j) / resolution;

//             if (gx < 0 || gy < 0 || gx >= sW_L || gy >= sH_L) continue;

//             int texX = minX + (i * spriteW) / scaledW;
//             int texY = minY + (j * spriteH) / scaledH;

//             int8_t color = texture[texY * texW + texX];
//             if (color != -1) setPixScnBuf(gx, gy, color);
//         }
//     }
// }

// void drawImgNoScale(int x, int y, int tX, int tY, int tW, int tH, int8_t* texture, int texW, int texH) {
//     if (tX >= texW || tY >= texH) return;
//     if (tW >= texW) tW = texW - 1;
//     if (tH >= texH) tH = texH - 1;

//     int regionW = tW - tX;
//     int regionH = tH - tY;

//     int drawX = x - regionW / 2;
//     int drawY = y - regionH / 2;
    
//     for (int j = 0; j < regionH; j++) {
//         for (int i = 0; i < regionW; i++) {
//             int texX = (tX + i);
//             int texY = (tY + j);

//             int8_t color = texture[texY * texW + texX];
//             int gx = drawX + i;
//             int gy = drawY + j;

//             if (gx < 0 || gy < 0 || gx >= sW_L || gy >= sH_L) continue;
//             setPixScnBuf(gx / resolution, gy / resolution, color);
//         }
//     }
// }