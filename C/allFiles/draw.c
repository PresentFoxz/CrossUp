#include "draw.h"

static const uint8_t shadeLUT[16][4][4] = {
    {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}},
    {{1,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}},
    {{1,0,1,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}},
    {{1,0,1,0},{1,0,0,0},{0,0,0,0},{0,0,0,0}},
    {{1,0,1,0},{1,0,1,0},{0,0,0,0},{0,0,0,0}},
    {{1,0,1,0},{1,0,1,0},{1,0,0,0},{0,0,0,0}},
    {{1,0,1,0},{1,0,1,0},{1,0,1,0},{0,0,0,0}},
    {{1,0,1,0},{1,0,1,0},{1,0,1,0},{1,0,0,0}},
    {{1,0,1,0},{1,0,1,0},{1,0,1,0},{1,0,1,0}},
    {{1,1,1,0},{1,0,1,0},{1,0,1,0},{1,0,1,0}},
    {{1,1,1,0},{1,1,1,0},{1,0,1,0},{1,0,1,0}},
    {{1,1,1,0},{1,1,1,0},{1,1,1,0},{1,0,1,0}},
    {{1,1,1,0},{1,1,1,0},{1,1,1,0},{1,1,1,0}},
    {{1,1,1,1},{1,1,1,0},{1,1,1,0},{1,1,1,0}},
    {{1,1,1,1},{1,1,1,1},{1,1,1,0},{1,1,1,0}},
    {{1,1,1,1},{1,1,1,1},{1,1,1,1},{1,1,1,1}}
};


#if defined(TARGET_PLAYDATE) || defined(PLAYDATE_SDK)
static uint8_t ditherByte[16][4][4];

void initDitherByteLUT() {
    for (int s = 0; s < 16; s++) {
        for (int py = 0; py < 4; py++) {
            for (int px0 = 0; px0 < 4; px0++) {
                uint8_t mask = 0;
                for (int i = 0; i < 8; i++) {
                    int px = (px0 + i) & 3;
                    if (shadeLUT[s][py][px]) {
                        mask |= (1 << (7 - i));
                    }
                }
                ditherByte[s][py][px0] = mask;
            }
        }
    }
}

void setPixelRaw(uint x, uint y, int color) {
    uint8_t* row = buf + y * rowStride;

    int bitIndex = 7 - (x % 8);
    uint8_t mask = 1 << bitIndex;

    if (color)
        row[x / 8] |= mask;
    else
        row[x / 8] &= ~mask;
}

void upscaleToScreen() {
    for (uint ly = 0; ly < sH_L; ly++) {
        uint baseY = ly * resolution;

        for (uint dy = 0; dy < resolution; dy++) {
            uint y = baseY + dy;

            uint8_t* row = buf + y * rowStride;
            uint py = y & 3;

            for (uint lx = 0; lx < sW_L; lx++) {
                int shade = scnBuf[ly * sW_L + lx];
                if (shade <= 0) continue;

                uint baseX = lx * resolution;
                uint x = baseX;
                uint endX = baseX + resolution;
                if (endX > sW) endX = sW;
                
                while ((x & 7) && x < endX) {
                    uint px = x & 3;
                    if (shadeLUT[shade][py][px]) {
                        row[x >> 3] |= (1 << (7 - (x & 7)));
                    }
                    x++;
                }
                
                uint byteEnd = endX & ~7;
                while (x < byteEnd) {
                    uint px0 = x & 3;
                    row[x >> 3] |= ditherByte[shade][py][px0];
                    x += 8;
                }
                
                while (x < endX) {
                    uint px = x & 3;
                    if (shadeLUT[shade][py][px]) {
                        row[x >> 3] |= (1 << (7 - (x & 7)));
                    }
                    x++;
                }
            }
        }
    }
}


#else
static inline void multiPixl(uint gridX, uint gridY, int shade) {
    if (gridX >= sW || gridY >= sH) return;
    
    for (int dy = 0; dy < resolution; dy++) {
        uint rowY = gridY + dy;
        if (rowY >= sH) break;

        for (int dx = 0; dx < resolution; dx++) {
            uint colX = gridX + dx;
            if (colX >= sW) break;
            
            uint px = (colX & 3);
            uint py = (rowY & 3);
            
            Color color;
            if (shadeLUT[shade][py][px] == 1) { color = (Color){255, 255, 255, 255}; } else { color = (Color){0, 0, 0, 255}; }
            
            DrawPixel(colX, rowY, color);
        }
    }
}

void drawScreen() {
    for (int y = 0; y < sH; y += resolution) {
        for (int x = 0; x < sW; x += resolution) {
            int bx = x / resolution;
            int by = y / resolution;

            int col = scnBuf[by * (sW / resolution) + bx];
            if (col >= 0) multiPixl(x, y, col);
        }
    }
}
#endif

void skybox(int col1, int col2, int count) {
    for (int y = 0; y < sH_L; y++) {
        for (int x = 0; x < sW_L; x++) {
            int checkerX = x / count;
            int checkerY = y / count;
            
            if ((checkerX + checkerY) % 2 == 0) {
                scnBuf[y * sW_L + x] = col1;
            } else {
                scnBuf[y * sW_L + x] = col2;
            }
        }
    }
}
