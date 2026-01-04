#ifndef TESTANIM_H
#define TESTANIM_H
#include "../mesh.h"
#include "testModels.h"

static const AnimMesh idleTri = {
    .meshModel = (const Mesh_t*[]) { &test2 },
    .animOrientation = (VectB[]) {
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 6, 0},
        {{0.0f, 0.0f, 0.0f}, {20.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 6, 0},
    },
    .count = 2,
};

static const AnimMesh idleCube = {
    .meshModel = (const Mesh_t*[]) { &test1 },
    .animOrientation = (VectB[]) {
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 12, 0},
    },
    .count = 1,
};

static const AnimMesh moveTri = {
    .meshModel = (const Mesh_t*[]) { &test2, &test3 },
    .animOrientation = (VectB[]) {
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 6, 0},
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 10, 1},
        {{20.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 17, 1},
    },
    .count = 3,
};

static const AnimMesh moveCube = {
    .meshModel = (const Mesh_t*[]) { &test1 },
    .animOrientation = (VectB[]) {
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 2, 0},
        {{0.0f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, -0.5f, 0.0f}, 4, 0},
        {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.5f, 0.0f}, 8, 0},
        {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 12, 0},
        {{0.0f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, -0.5f, 0.0f}, 15, 0},
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 18, 0},
    },
    .count = 6,
};

#endif