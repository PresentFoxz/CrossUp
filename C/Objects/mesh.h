#ifndef MESH_H
#define MESH_H

typedef struct {
    float x, y, z;
} Vect3m;

typedef struct {
    Vect3m* data;
    int* color;
    int count;
} Mesh;

#endif