#ifndef LIST_MAKER_H
#define LIST_MAKER_H

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
    Vect3f pos;
    int floor, cieling, wall;
} VectMf;

typedef struct {
    float verts[3][3];
    int objType;
    float dist;
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
    int type, grounded, coyote, ifMove, crouch;
    int countdown, rotDir;
    float frict, fallFrict;
} EntStruct;

#endif