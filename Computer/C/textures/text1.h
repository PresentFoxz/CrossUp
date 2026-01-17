#ifndef TEXT1_H
#define TEXT1_H

#include "library.h"

static const TextAtlas texture1 = {
    .pixels = (int[]) {
        0, 0, 0, 0,
        1, 0, 0, 1,
        1, 2, 3, 1,
        0, 3, 2, 0,
    },
    .w = 4,
    .h = 4
};

#endif