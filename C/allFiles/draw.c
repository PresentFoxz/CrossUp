#include "draw.h"

static ALIGNED_32 uint8_t _screen[sW * sH] = {0};
static bool _scnDirty[sH] = {false};
static uint8_t* hwscreen;

static int interlaceFrame   = 0;
static int interlaceHeight  = 1;
static int frameInterlacing = 0;

const uint32_t ordered_dither4x4[] = {
    0xa52d870f,
    0x69e14bc3,
    0x961eb43c,
    0x5ad278f0,
};

static void hline(int x1, int x2, int y, uint8_t color) {
    if (x1 < 0) x1 = 0;
    if (x2 >= sW) x2 = sW - 1;
    if (x2 < x1) return;

    uint8_t* dst = _screen + y * sW + x1;
    int len = x2 - x1 + 1;
    while (len--) *dst++ = color;
    _scnDirty[y] = true;
}

static void drawPixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= sW) return;
    if (y < 0 || y >= sH) return;

    uint8_t brightness = color + ((interlaceFrame & 1) << 3);

    _screen[y * sW + x] = brightness;
    _scnDirty[y] = true;
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
    hwscreen = pd->graphics->getFrame();
    const int rowBytes = sW;
    const int totalRows = sH;

    if (frameInterlacing) {
        for (int y = interlaceFrame; y < totalRows; y += interlaceHeight) {
            uint8_t* dst = hwscreen + y * LCD_ROWSIZE;
            uint8_t* src = _screen + y * rowBytes;
            const uint32_t threshold = ordered_dither4x4[y & 3];
            
            for (int xbyte = 0; xbyte <= rowBytes - 8; xbyte += 8, dst++, src += 8) {
                uint32_t pixels0, pixels1;

                memcpy(&pixels0, src, 4);
                memcpy(&pixels1, src + 4, 4);

                *dst = __SUBTEST_DUAL(pixels0, threshold, pixels1, threshold);
            }
        }

        for (int y = interlaceFrame; y < totalRows; y += interlaceHeight) {
            if (_scnDirty[y]) {
                memset(_screen + y * rowBytes, 0, rowBytes);
                _scnDirty[y] = false;
            }
        }

        pd->graphics->markUpdatedRows(interlaceFrame, totalRows - 1);
        interlaceFrame = (interlaceFrame + 1) % interlaceHeight;
    } else {
        for (int y = 0; y < totalRows; y++) {
            uint8_t* dst = hwscreen + y * LCD_ROWSIZE;
            uint8_t* src = _screen + y * rowBytes;
            const uint32_t threshold = ordered_dither4x4[y & 3];

            for (int xbyte = 0; xbyte <= rowBytes - 8; xbyte += 8, dst++, src += 8) {
                uint32_t pixels0, pixels1;

                memcpy(&pixels0, src, 4);
                memcpy(&pixels1, src + 4, 4);

                *dst = __SUBTEST_DUAL(pixels0, threshold, pixels1, threshold);
            }
        }
        
        memset(_screen, 0, sW * sH);
        pd->graphics->markUpdatedRows(0, totalRows - 1);
        interlaceFrame = 0;
    }
}

void changeLacing(int l0, int l1, bool bType) {
    if (l1 <= 0) l1 = 1;

    interlaceFrame   = l0;
    interlaceHeight  = l1;
    frameInterlacing = bType;
}

void drawRect(int x, int y, int w, int h, uint8_t color) {
    if (x > sW) return;
    if (y > sH) return;
    if (x + w < 0) return;
    if (y + h < 0) return;
    for (int y_=0; y_ < h; y_++) {
        hline(x, (x + w), y + y_, color);
    }
}

void drawTriangle(int tris[3][2], uint8_t shade) {
    int* v0 = tris[0];
    int* v1 = tris[1];
    int* v2 = tris[2];

    int* temp;
    if (v1[1] < v0[1]) { temp=v0; v0=v1; v1=temp; }
    if (v2[1] < v0[1]) { temp=v0; v0=v2; v2=temp; }
    if (v2[1] < v1[1]) { temp=v1; v1=v2; v2=temp; }

    int dy01 = v1[1] - v0[1];
    int dy12 = v2[1] - v1[1];
    int dy02 = v2[1] - v0[1];

    if (dy02 == 0) return;

    float dx02 = (float)(v2[0] - v0[0]) / dy02;
    float dx01 = 0;
    float dx12 = 0;

    if (dy01 != 0) dx01 = (float)(v1[0] - v0[0]) / dy01;
    if (dy12 != 0) dx12 = (float)(v2[0] - v1[0]) / dy12;

    float xA = v0[0];
    float xB = v0[0];

    int y;
    for (y = v0[1]; y < v1[1]; y++) {
        if (y >= 0 && y < sH) {
            int xLeft  = (int)(xA < xB ? xA + 0.5f : xB + 0.5f);
            int xRight = (int)(xA > xB ? xA + 0.5f : xB + 0.5f);

            hline(xLeft, xRight, y, shade);
        }
        xA += dx02;
        xB += dx01;
    }

    xB = v1[0];
    for (; y <= v2[1]; y++) {
        if (y >= 0 && y < sH) {
            int xLeft  = (int)(xA < xB ? xA + 0.5f : xB + 0.5f);
            int xRight = (int)(xA > xB ? xA + 0.5f : xB + 0.5f);

            hline(xLeft, xRight, y, shade);
        }
        xA += dx02;
        xB += dx12;
    }
}

void drawTexturedTriangle(int tris[3][2], Vector2f uvs[3], float z0, float z1, float z2, int tx, int ty, int tw, int th, int* texture, int texW, int texH) {
    float x0 = tris[0][0], y0 = tris[0][1];
    float x1 = tris[1][0], y1 = tris[1][1];
    float x2 = tris[2][0], y2 = tris[2][1];

    float u0 = uvs[0].x; float v0 = uvs[0].z;
    float u1 = uvs[1].x; float v1 = uvs[1].z;
    float u2 = uvs[2].x; float v2 = uvs[2].z;

    float minX = x0, maxX = x0;
    float minY = y0, maxY = y0;

    if (x1 < minX) minX = x1;
    if (x2 < minX) minX = x2;
    if (x1 > maxX) maxX = x1;
    if (x2 > maxX) maxX = x2;

    if (y1 < minY) minY = y1;
    if (y2 < minY) minY = y2;
    if (y1 > maxY) maxY = y1;
    if (y2 > maxY) maxY = y2;

    if (minX < 0) minX = 0;
    if (minY < 0) minY = 0;
    if (maxX >= sW) maxX = sW - 1;
    if (maxY >= sH) maxY = sH - 1;
    
    float denom = (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0);
    if (denom == 0.0f) return;

    float invDenom = 1.0f / denom;

    float iz0 = 1.0f / z0;
    float iz1 = 1.0f / z1;
    float iz2 = 1.0f / z2;

    float u0p = u0 * iz0, v0p = v0 * iz0;
    float u1p = u1 * iz1, v1p = v1 * iz1;
    float u2p = u2 * iz2, v2p = v2 * iz2;

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            float w0 = ((y1 - y2)*(x - x2) + (x2 - x1)*(y - y2)) * invDenom;
            float w1 = ((y2 - y0)*(x - x2) + (x0 - x2)*(y - y2)) * invDenom;
            float w2 = 1.0f - w0 - w1;

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                float invZ = w0 * iz0 + w1 * iz1 + w2 * iz2;
                if (invZ == 0.0f) continue;

                float u = (w0 * u0p + w1 * u1p + w2 * u2p) / invZ;
                float v = (w0 * v0p + w1 * v1p + w2 * v2p) / invZ;

                if (u < 0.0f) u = 0.0f;
                if (u > 1.0f) u = 1.0f;
                if (v < 0.0f) v = 0.0f;
                if (v > 1.0f) v = 1.0f;

                int TexX = (int)(u * (texW - 1));
                int TexY = (int)(v * (texH - 1));

                int shade = texture[TexY * texW + TexX];
                if (shade != -1) drawPixel(x, y, (uint8_t)(shade));
            }
        }
    }
}

void drawImg(int screenX, int screenY, float depth, int tX, int tY, int tW, int tH, int* texture, int texW, int texH, float projDist) {
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

            int color = texture[texY * texW + texX];
            if (color != -1) {
                uint8_t brightness = color + ((interlaceFrame & 1) << 3);
                _screen[gy * sW + gx] = brightness;
            }
        }
    }
}

void drawImgNoScale(int x, int y, int tX, int tY, int tW, int tH, int* texture, int texW, int texH) {
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

            if (color != -1) {
                uint8_t brightness = color + ((interlaceFrame & 1) << 3);
                _screen[gy * sW + gx] = brightness;
            }
        }
    }
}