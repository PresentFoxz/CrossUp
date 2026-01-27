#ifndef MESH_H
#define MESH_H
#include "library.h"

typedef struct {
    Vect3f* data;
    int* color;
    int* bfc;
    int count;
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