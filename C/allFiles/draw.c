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
void setPixelRaw(uint x, uint8_t* row, int color) {
    int bitIndex = 7 - (x % 8);
    uint8_t mask = 1 << bitIndex;

    if (color)
        row[x / 8] |= mask;
    else
        row[x / 8] &= ~mask;
}

void multiPixl(uint gridX, uint gridY, int shade) {
    if (gridX >= sW || gridY >= sH) return;

    uint8_t colorMask[2][2] = {
        {0x00, 0xFF},
        {0xFF, 0x00}
    };

    for (int dy = 0; dy < resolution; dy++) {
        uint y = gridY + dy;
        if (y >= sH) break;

        uint8_t* row = buf + y * rowStride;

        int py = (y & 3);

        uint x = gridX;
        uint endX = gridX + resolution;
        if (endX > sW) endX = sW;
        
        while ((x & 7) && x < endX) {
            int px = (x & 3);
            int color = (shade != -1) && shadeLUT[shade][py][px];
            setPixelRaw(x, row, color);
            x++;
        }
        
        while (x + 8 <= endX) {
            int px = (x & 3);
            uint8_t pattern = shadeLUT[shade][py][px] ? 0xFF : 0x00;
            row[x >> 3] = pattern;
            x += 8;
        }

        // Tail
        while (x < endX) {
            int px = (x & 3);
            int color = (shade != -1) && shadeLUT[shade][py][px];
            setPixelRaw(x, row, color);
            x++;
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