#ifndef STRUCTS_H
#define STRUCTS_H

typedef int32_t qfixed16_t;
typedef int32_t qfixed24x8_t;
typedef int64_t qfixed32_t;
typedef unsigned int uint;

typedef enum {
    ENT_NONE,
    ENT_PLAYER,
    ENT_TEST
} EntTypes;

typedef struct {
    qfixed24x8_t x, y, z;
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
    float x, y, z;
    float u, v;
} Vertex;

typedef struct {
    Vertex t1, t2, t3;
} clippedTri;

typedef struct {
    Vect3i position;
    Vect3i rotation;
    float fov, nearPlane, farPlane;
} Camera_t;

typedef struct {
    const char*** animNames;
    const int* animFrameCounts;
    int totalAnims;
} entData_t;

typedef struct {
    int actionUsed;
    int timer;
} Action;

typedef struct {
    Action spin;
} PlayerActions;

typedef struct {
    int countdown;
} EntityActions;

typedef struct {
    Vect3i position;
    Vect3i rotation;
    Vect3i size;
    Vect3f velocity;
    qfixed24x8_t surfRot;
    qfixed24x8_t radius, height;
    int type, grounded, groundTimer;
    int coyote, ifMove;
    int countdown;
    float frict, fallFrict;
    int frameCount, currentFrame;
    int currentAnim, lastAnim;
    int meshIndex;

    union {
        PlayerActions plr;
        EntityActions ent;
    } actions;
} EntStruct;

typedef struct {
    int timer;
    int type;
    Vect3i position;
    Vect3i rotation;
    Vect3i size;
    Vect3f velocity;
} ObjStruct;

typedef enum {
    PLAYER,
    ENTITY,
    OBJECT
} ModelType;

typedef struct {
    ModelType type;

    union {
        EntStruct plr;
        EntStruct ent;
        ObjStruct obj;
    } data;
} Objects;

typedef struct {
    int* pixels;
    int w;
    int h;
} textAtlas;

typedef struct {
    Vect3f pos;
    Vect3f size;
    int type, id;
} Triggers;

typedef struct {
    int** chunkIndex;
    int chunkAmt;
    int* amt;
} ChunkCount;

#endif