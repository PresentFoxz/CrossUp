#ifndef STRUCTS_H
#define STRUCTS_H

typedef int16_t qfixed8_t;
typedef int32_t qfixed16_t;
typedef int64_t qfixed32_t;
typedef unsigned int uint;

typedef enum {
    ENT_NONE,
    ENT_PLAYER,
    ENT_TEST
} EntTypes;

typedef struct {
    qfixed32_t x, y, z;
} Vect3i;

typedef struct {
    float x, y, z;
} Vect3f;

typedef struct {
    float x, y;
} Vect2f;

typedef struct {
    Vect3f pos;
    int floor, ceiling, wall;
} VectMf;

typedef struct {
    Vect3f min, max;
} MinMax3f;

typedef struct {
    Vect2f min, max;
} MinMax2f;

typedef struct {
    float verts[3][3];
    int color;
    float dist;
    int bfc;
} worldTris;

typedef struct {
    float x, y, z;
    float u, v;
} Vertex;

typedef struct {
    Vertex t1, t2, t3;
} clippedTri;

typedef struct {
    Vect3i pos;
    int joint[2][3];
    int color;
} staticPoints;

typedef struct {
    Vect3i position;
    Vect3i rotation;
    qfixed32_t fov, nearPlane, farPlane;
} Camera;

typedef struct {
    Vect3i position;
    Vect3i rotation;
    Vect3i size;
    Vect3f velocity;
    qfixed32_t surfRot;
    qfixed32_t radius, height;
    int type, grounded, groundTimer;
    int coyote, ifMove, state;
    int countdown, rotDir;
    float frict, fallFrict;

    int* frameCount;
    int* currentFrame;
    int currentAnim, lastAnim, jointCount;
} EntStruct;

typedef struct {
    int x0, y0, x1, y1, x2, y2;
    float z0, z1, z2;
    
    int32_t A01, B01, C01;
    int32_t A12, B12, C12;
    int32_t A20, B20, C20;

    int minX, maxX, minY, maxY;
    int flip;

    float invZStepX, invZStepY;
    float invZ_row;
} RasterTri;

#endif