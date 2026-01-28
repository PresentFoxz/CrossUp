#include "draw.h"

static const uint8_t shadeLUT[4][2][2] = {
    {{0,1},{0,0}},
    {{0,1},{1,0}},
    {{0,0},{1,1}},
    {{1,1},{1,1}}
};

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
            
            int color;
            if (shade == -1 ) { color = 0; } else if (shadeLUT[shade][py][px] == 1) { color = 1; } else { color = 0; }
            pd->graphics->setPixel(colX, rowY, color);
        }
    }
}

void drawScreen() {
    for (int y = 0; y < sH; y += resolution) {
        for (int x = 0; x < sW; x += resolution) {
            int bx = x / resolution;
            int by = y / resolution;
            multiPixl(x, y, scnBuf[by * (sW / resolution) + bx]);
        }
    }
}