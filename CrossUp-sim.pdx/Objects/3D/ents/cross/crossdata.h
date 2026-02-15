#ifndef CROSSDATA_H
#define CROSSDATA_H

static const char* cross_idleAnim[] = {
    "Objects/3D/ents/cross/idle/idle0001.obj"
};

static const char* cross_walkAnim[] = {
    "Objects/3D/ents/cross/walk/walk0001.obj",
    "Objects/3D/ents/cross/walk/walk0002.obj",
    "Objects/3D/ents/cross/walk/walk0003.obj",
    "Objects/3D/ents/cross/walk/walk0004.obj"
};

static const char** cross_animNames[2] = { cross_idleAnim, cross_walkAnim };
static const int cross_animFrameCounts[2] = { 1, 4 };
static const int cross_totalAnims = 2;

#endif