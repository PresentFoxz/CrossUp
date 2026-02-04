#ifndef MESH_H
#define MESH_H
#include "library.h"

typedef struct {
    Vect3f* verts;
    int vertCount;

    int (*tris)[3];
    int triCount;

    float (*uvs)[2];
    int uvCount;

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