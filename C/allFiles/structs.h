#ifndef STRUCTS_H
#define STRUCTS_H

typedef unsigned int uint;

typedef enum {
    D_2D, D_3D
} Dimentions;

typedef enum {
    PLAYER,
    ENTITY,
    OBJECT
} ModelType;

typedef enum {
    SLICE_NONE,
    SLICE_SECTOR,
    SLICE_WALL,
    SLICE_ENTITY,
    SLICE_OBJECT
} SliceType;

typedef struct {
    int x, z;
} Vector2i;
typedef struct {
    int x, y, z;
} Vector3i;

typedef struct {
    float x, z;
} Vector2f;
typedef struct {
    float x, y, z;
} Vector3f;

typedef struct {
    Vector3f min, max;
} MinMax3f;

typedef struct {
    Vector2f min, max;
} MinMax2f;

typedef struct {
    Vector3f pos;
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
    Vector3f position;
    Vector3f rotation;
    Vector3f fVect;
    float camMatrix[3][3];
    float fov, nearPlane, farPlane;
    float projDist;
} Camera_t;

typedef struct {
    int actionUsed;
    int timer;
} Action;

typedef struct {
    Action spin;
    Action dash;
} PlayerActions;

typedef struct {
    int countdown;
} EntityActions;

typedef struct {
    Dimentions dimention;

    Vector3f position;
    Vector3f rotation;
    Vector3f size;
    Vector3f velocity;
    float surfRot;
    float radius, height;
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
    Vector3f position;
    Vector3f rotation;
    Vector3f size;
    Vector3f velocity;
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
    Vector3f pos;
    Vector3f size;
    int type, id;
} Triggers;

typedef struct {
    int** chunkIndex;
    int chunkAmt;
    int* amt;
} ChunkCount;

typedef struct {
    Vector3f pos;
    uint8_t power;
    float falloff;
} Light_t;

typedef struct {
    uint8_t UP;
    uint8_t DOWN;
    uint8_t LEFT;
    uint8_t RIGHT;
    uint8_t A;
    uint8_t B;
} InputBuffer;

typedef struct {
    float y;
    Vector2i min, max;
} WaterSlice;

typedef struct {
    Vector2i* points;
    float y[2];
    int pallete;
    int count;
    int normal;
    int type;
} SectorSlice;

typedef struct {
    Vector2i points[2];
    float y[2];
    int pallete;
    int normal;
    int type;
} WallSlice;

typedef struct {
    Vector3f pos;
    Vector3f rot;
    Vector3f size;
    int type;
} ObjectSlice;

typedef struct {
    SectorSlice* sectors;
    int sectorCount;

    WallSlice* walls;
    int wallCount;

    ObjectSlice* objects;
    int objectCount;

    ObjectSlice* entities;
    int entityCount;

    Vector3f chunkPos;
    Vector3f chunkWHD;
} WorldChunks;

#endif