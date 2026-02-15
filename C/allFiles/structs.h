#ifndef STRUCTS_H
#define STRUCTS_H

typedef int32_t qfixed16_t;
typedef int32_t qfixed24x8_t;
typedef int64_t qfixed32_t;
typedef unsigned int uint;

typedef enum {
    D_2D, D_3D
} Dimentions;

typedef enum {
    PLAYER,
    ENTITY,
    OBJECT
} ModelType;

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
    Vect3f min, max;
} MinMax3f;

typedef struct {
    Vect2f min, max;
} MinMax2f;

typedef struct {
    Vect3f pos;
    int floor, ceiling, wall;
} VectMf;
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
    int actionUsed;
    int timer;
} Action;

typedef struct {
    Action spin;
    Action dive;
    Action punch;
} PlayerActions;

typedef struct {
    int countdown;
} EntityActions;

typedef struct {
    Dimentions dimention;

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
    union {
        PlayerActions plr;
        EntityActions ent;
    } actions;
} EntStruct;

typedef struct {
    Dimentions dimention;

    int timer;
    int type;
    Vect3i position;
    Vect3i rotation;
    Vect3i size;
    Vect3f velocity;
} ObjStruct;

typedef struct {
    ModelType type;

    union {
        EntStruct plr;
        EntStruct ent;
        ObjStruct obj;
    } data;
} Objects;

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

typedef struct {
    uint8_t UP;
    uint8_t DOWN;
    uint8_t LEFT;
    uint8_t RIGHT;
    uint8_t A;
    uint8_t B;
} InputBuffer;

#endif