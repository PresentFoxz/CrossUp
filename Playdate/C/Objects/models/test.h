#ifndef TEST_H
#define TEST_H
#include "../mesh.h"

static const Mesh test1 = {
    .data = (Vect3m[]) {
        {-0.8f, -0.8f, -0.8f}, {-0.8f, -0.8f, 0.8f}, {-0.8f, 0.8f, 0.8f},
        {-0.8f, -0.8f, -0.8f}, {-0.8f, 0.8f, 0.8f}, {-0.8f, 0.8f, -0.8f},
        {0.8f, 0.8f, 0.8f}, {-0.8f, 0.8f, 0.8f}, {-0.8f, -0.8f, 0.8f},
        {0.8f, 0.8f, 0.8f}, {-0.8f, -0.8f, 0.8f}, {0.8f, -0.8f, 0.8f},
        {0.8f, -0.8f, 0.8f}, {-0.8f, -0.8f, 0.8f}, {-0.8f, -0.8f, -0.8f},
        {0.8f, -0.8f, 0.8f}, {-0.8f, -0.8f, -0.8f}, {0.8f, -0.8f, -0.8f},
        {0.8f, 0.8f, -0.8f}, {-0.8f, 0.8f, -0.8f}, {-0.8f, 0.8f, 0.8f},
        {0.8f, 0.8f, -0.8f}, {-0.8f, 0.8f, 0.8f}, {0.8f, 0.8f, 0.8f},
        {0.8f, -0.8f, -0.8f}, {-0.8f, -0.8f, -0.8f}, {-0.8f, 0.8f, -0.8f},
        {0.8f, -0.8f, -0.8f}, {-0.8f, 0.8f, -0.8f}, {0.8f, 0.8f, -0.8f},
        {0.8f, 0.8f, -0.8f}, {0.8f, 0.8f, 0.8f}, {0.8f, -0.8f, 0.8f},
        {0.8f, 0.8f, -0.8f}, {0.8f, -0.8f, 0.8f}, {0.8f, -0.8f, -0.8f}
    },
    .bfc = (int[]) {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    .color = (int[]) {3, 1, 1, 2, 2, 0, 2, 0, 3, 2, 3, 1},
    .count = (int) 12,
};

static const Mesh test2 = {
    .data = (Vect3m[]) {
        {-1.5f, 0.8f, 0.0f}, {0.0f, 2.0f, 0.0f}, {1.5f, 0.8f, 0.0f}
    },
    .bfc = (int[]) {0},
    .color = (int[]) {3},
    .count = (int) 1,
};

static const AnimMesh idleTri = {
    .meshModel = (const Mesh*[]) { &test2 },
    .animOrientation = (VectB[]) {
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 6, 0},
        {{0.0f, 0.0f, 0.0f}, {20.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 6, 0}
    },
};

static const AnimMesh idleCube = {
    .meshModel = (const Mesh*[]) { &test1 },
    .animOrientation = (VectB[]) {
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 0, 0}
    },
};

static const AnimMesh moveTri = {
    .meshModel = (const Mesh*[]) { &test2 },
    .animOrientation = (VectB[]) {
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 0, 0}
    },
};

static const AnimMesh moveCube = {
    .meshModel = (const Mesh*[]) { &test1 },
    .animOrientation = (VectB[]) {
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 4, 0},
        {{0.0f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, -0.5f, 0.0f}, 4, 0},
        {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.5f, 0.0f}, 4, 0},
        {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 4, 0},
        {{0.0f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, -0.5f, 0.0f}, 4, 0}
    },
};

static const ModelAnimations triAnims = {
    .animations = (const AnimMesh*[]) { &idleTri, &moveTri }
};

static const ModelAnimations cubeAnim = {
    .animations = (const AnimMesh*[]) { &idleCube, &moveCube }
};

static const int idleFrames[] = { 1, 2 };
static const int moveFrames[] = { 5, 1 };

static const PlayerModel_t testox = {
    .animations = (const ModelAnimations*[]) { &cubeAnim, &triAnims },
    .maxFrames = (const int*[]) { idleFrames, moveFrames },
    .joints = 2,
};

#endif
