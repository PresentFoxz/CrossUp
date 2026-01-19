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
    Vect3f v0, v1, v2;
    Vect3f normal;
    Vect3f center;
    SurfaceType type;
} CollisionSurface;

typedef struct {
    float floorY;
    int hasFloor;

    float ceilY;
    int hasCeil;

    float pushX, pushZ;
    int hasWall;
} CollisionResult;

typedef struct {
    MinMax2f* lineMinMax;
    MinMax3f* BoxMinMax;
} OOBArea;

void resetCollisionSurface();
void addCollisionSurface(Vect3f v0, Vect3f v1, Vect3f v2, SurfaceType type);
VectMf cylinderInTriangle(Vect3f pos, float radius, float height);

#endif