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
    int flipped;
    int outline;
} Mesh_t;

typedef struct {
    Mesh_t* meshModel;
    int frames;
} AnimFrames;

typedef struct {
    AnimFrames** anims;
    int count;
} VertAnims;

#endif