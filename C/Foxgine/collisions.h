#ifndef COLLISIONS_H
#define COLLISIONS_H

#include "../allFiles/library.h"

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
    Vector3f v0, v1, v2;
    Vector3f normal;
    Vector3f center;
    SurfaceType type;

    int minX, minY, minZ;
    int maxX, maxY, maxZ;

    float v0x, v0y, v0z;
    float v1x, v1y, v1z;

    float dot00, dot01, dot11, invDenom;
} CollisionSurface;

typedef struct {
    CollisionSurface* collisions;
    Vector3f pos;
    int amt;
} CollisionChunks;

typedef struct {
    MinMax2f* lineMinMax;
    MinMax3f* BoxMinMax;
} OOBArea;

void resetTriggers();
void resetCollisionSurface();
void fixSurfaces(Mesh_t mapArray);
void collisionChunks();
void addTriggers(Vector3f pos, Vector3f size, int type, int id);
void addCollisionSurface(Vector3f v0, Vector3f v1, Vector3f v2, Vector3f normal, SurfaceType type);
VectMf cylinderInTriangle(Vector3f pos, float radius, float height);
Triggers cylinderInTrigger(Vector3f pos, float radius, float height);

#endif