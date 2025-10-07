#ifndef COLLISIONS_H
#define COLLISIONS_H

#include "library.h"
#include "structs.h"

#define MAX_COLLISIONS 1024

typedef enum {
    SURFACE_NONE,
    SURFACE_FLOOR,
    SURFACE_WALL,
    SURFACE_CEILING,
    SURFACE_WATER,
    OUT_OF_BOUNDS
} SurfaceType;

typedef struct {
    float x1, y1, z1;
    float x2, y2, z2;
    float x3, y3, z3;
    SurfaceType type;
    float normalX, normalY, normalZ;
} CollisionSurface;

typedef struct {
    float floorY;
    int hasFloor;

    float ceilY;
    int hasCeil;

    float pushX, pushZ;
    int hasWall;
} CollisionResult;

void resetCollisionSurface();
void addCollisionSurface(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, SurfaceType type);
VectMf cylinderInTriangle(float cx, float cy, float cz, float radius, float height);

#endif