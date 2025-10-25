#ifndef MESH_H
#define MESH_H
typedef struct {
    float x, y, z;
} Vect3m;

typedef struct {
    Vect3m* data;
    int* color;
    int* bfc;
    int count;
} Mesh_t;

typedef struct {
    Vect3f pos;
    Vect3f rot;
    Vect3f size;
    int frameSwap;
    int modelUsed;
} VectB;

typedef struct {
    const Mesh_t** meshModel;
    VectB* animOrientation;
    int count;
} AnimMesh;

typedef struct { 
    const AnimMesh** animations;
} ModelAnimations;

typedef struct {
    const ModelAnimations** animations;
    const int* maxFrames;
    int joints;
    int count;
} PlayerModel_t;

#endif