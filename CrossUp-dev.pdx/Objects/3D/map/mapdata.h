#ifndef MAPDATA_H
#define MAPDATA_H

static const char* mapObjs[] = {
    "Objects/3D/map/Castle.obj",
    "Objects/3D/map/teestplate.obj",
};

static const int mapData[][2] = {
    {0, 0},
    {0, 1},
};

static const Vect3f mapSize[] = {
    {4.0f, 4.0f, 4.0f},
    {2.0f, 2.0f, 2.0f},
};

#endif