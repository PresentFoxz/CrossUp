#ifndef MESH_H
#define MESH_H
#include "library.h"

typedef struct {
    int v0, v1;
    int tri0, tri1;
} Edge;

typedef struct {
    int x0, y0, x1, y1;
    int tri0, tri1;
} EdgeDraw;

typedef struct {
    int idx;
    float dist;
} TriangleOrdering;

typedef struct {
    Dimentions dimentions;
    
    Vertex verts[3];
    EdgeDraw edges[3];
    float distMod;
    uint8_t color;
    int textID;
} worldTris;

typedef struct {
    Vector3f* verts;
    int vertCount;

    int (*tris)[3];
    int triCount;

    Vector3f* normal;

    Edge* edges;
    int edgeCount;
    
    uint8_t* color;
    int* bfc;
} Mesh_t;

typedef struct {
    Mesh_t map;
    Vector3f pos;
    Vector3f whd;
} Mesh_Chunks;

typedef struct {
    Mesh_t* meshModel;
    int frames;
} AnimFrames;

typedef struct {
    AnimFrames** anims;
    int count;
} VertAnims;

typedef struct {
    int8_t* pixels;
    int w;
    int h;
} textAtlas;

typedef struct {
    textAtlas animData;
    int frames;
} textAtlasFrames;

typedef struct {
    textAtlasFrames** animation;
    int count;
} textAnimsAtlas;

typedef struct {
    const char*** animNames;
    const int* animFrameCounts;
    int totalAnims;
} entDataModel_t;

typedef struct {
    const char** animNames;
    const int* animFrameCounts;
    int totalAnims;
} entDataAtlas_t;

#endif