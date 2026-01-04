#ifndef MESHCONVERT_H
#define MESHCONVERT_H
#include "library.h"
#include "mesh.h"

void allocateMeshes(VertAnims* mesh, int entCount, int maxAnims, int* framesPerAnim);
void convertFileToMesh(const char* filename, Mesh_t* meshOut, int color);

#endif