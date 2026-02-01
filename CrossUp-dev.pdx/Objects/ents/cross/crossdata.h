#ifndef CROSSDATA_H
#define CROSSDATA_H

static const char* cross_idleAnim[] = {
    "Objects/ents/cross/idle/idle0000.obj"
};

static const char* cross_walkAnim[] = {
    "Objects/ents/cross/walk/walk0000.obj",
    "Objects/ents/cross/walk/walk0001.obj",
    "Objects/ents/cross/walk/walk0002.obj",
    "Objects/ents/cross/walk/walk0003.obj"
};

static const char** cross_animNames[2] = { cross_idleAnim, cross_walkAnim };
static const int cross_animFrameCounts[2] = { 1, 4 };
static const int cross_totalAnims = 2;

#endif