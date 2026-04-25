#ifndef CROSSDATA_H
#define CROSSDATA_H

static const char* cross_idleAnim3D[] = {
    "Objects/3D/ents/cross/idle/idle0001.obj"
};

static const char* cross_walkAnim3D[] = {
    "Objects/3D/ents/cross/walk/walk0001.obj",
    "Objects/3D/ents/cross/walk/walk0002.obj",
    "Objects/3D/ents/cross/walk/walk0003.obj",
    "Objects/3D/ents/cross/walk/walk0004.obj"
};

static const char** cross_animNames3D[2] = { cross_idleAnim3D, cross_walkAnim3D };
static const int cross_animFrameCounts3D[2] = { 1, 4 };
static const int cross_totalAnims3D = 2;

#endif