#ifndef MESHCONVERT_H
#define MESHCONVERT_H
#include "library.h"
#include "mesh.h"

void allocateMeshes(VertAnims* mesh, int maxAnims, const int* framesPerAnim);
void convertFileToMesh(const char* filename, Mesh_t* meshOut, int color, int invert);
int allocAnimModel(VertAnims* mesh, int maxAnims, const int* framesPerAnim, const char** names[]);

#endif