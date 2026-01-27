#ifndef TESTURE_H
#define TESTURE_H
#include "library.h"

static const textAtlas testTexture = {
    .pixels = (int[]) {
        0, 0, 0, 0,
        1, 0, 0, 1,
        0, 1, 3, 0,
        2, 2, 1, 3,
    },
    .w = 4,
    .h = 4,
};

#endif