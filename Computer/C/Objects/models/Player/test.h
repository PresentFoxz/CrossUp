#ifndef TEST_H
#define TEST_H
#include "../mesh.h"
#include "testAnim.h"

static const ModelAnimations triAnims = {
    .animations = (const AnimMesh*[]) { &idleTri, &moveTri }
};

static const ModelAnimations cubeAnim = {
    .animations = (const AnimMesh*[]) { &idleCube, &moveCube }
};

static const int modelCount = (test1.count + test2.count + test3.count);

static const PlayerModel_t testox = {
    .animations = (const ModelAnimations*[]) { &cubeAnim, &triAnims },
    .maxFrames = (const int[]) { 12, 20 },
    .joints = 2,
    .count = modelCount,
};

#endif
