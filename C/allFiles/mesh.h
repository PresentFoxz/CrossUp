#ifndef MESH_H
#define MESH_H
#include "library.h"

typedef struct {
    int v0;
    int v1;

    int tri0;
    int tri1;
} Edge;

typedef struct {
    Dimentions dimentions;
    
    Vertex verts[3];
    Edge edges[3];
    float distMod;
    int32_t dist;
    int color;
    int lines;
    int textID;
} worldTris;

typedef struct {
    Vect3f* verts;
    int vertCount;

    int (*tris)[3];
    int triCount;

    float (*uvs)[2];
    int uvCount;

    Edge* edges;
    int edgeCount;

    int* color;
    int* bfc;
} Mesh_t;

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