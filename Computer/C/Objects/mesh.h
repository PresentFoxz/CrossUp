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
    Vect3f pos, rot, size;
    int frameCount, modelUsed;
} VectB;

typedef struct {
    const Mesh_t** meshModel;
    VectB* animOrientation;
} AnimMesh;

typedef struct { 
    const AnimMesh** animations;
} ModelAnimations;

typedef struct {
    const ModelAnimations** animations;
    const int** maxFrames;
    int joints, count;
} PlayerModel_t;

#endif