#include "draw.h"

static const uint8_t shadeLUT[4][2][2] = {
    {{0,1},{0,0}},
    {{0,1},{1,0}},
    {{0,0},{1,1}},
    {{1,1},{1,1}}
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
    
    for (int dy = 0; dy < resolution; dy++) {
        uint rowY = gridY + dy;
        if (rowY >= sH) break;

        uint8_t* rowPtr = buf + rowY * rowStride;

        for (int dx = 0; dx < resolution; dx++) {
            uint colX = gridX + dx;
            if (colX >= sW) break;
            
            uint px = colX & 1;
            uint py = rowY & 1;
            
            int color; if (shade == -1 ) { color = 0; } else if (shadeLUT[shade][py][px] == 1) { color = 1; } else { color = 0; }
            setPixelRaw(colX, rowPtr, color);
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
            
            uint px = colX & 1;
            uint py = rowY & 1;
            
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