#ifndef MESH_H
#define MESH_H

typedef struct {
    float x, y, z;
    int boneIDX;
} Vect3m;

typedef struct {
    Vect3f pos, rot;
} VectB;

typedef struct {
    Vect3m* data;
    VectB* bones;
    int* color;
    int count;
} Mesh;

#endif