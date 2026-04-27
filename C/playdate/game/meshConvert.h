#ifndef MESHCONVERT_H
#define MESHCONVERT_H
#include "libRay.h"

void allocateMeshes(VertAnims* mesh, int maxAnims, const int* framesPerAnim);
void convertFileToMesh(const char* filename, Mesh_t* meshOut, int color, int invert, int outline, Vector3f size);

int allocAnimModel(VertAnims* mesh, int maxAnims, const int* framesPerAnim, const char** names[], int color, int invert, int outline, Vector3f size);
void allocAnimAtlas(textAnimsAtlas* atlas, int maxAnims, const int* framesPerAnim, const char* names[]);

Mesh_Chunks* readMapData(const char* filename, int* outSectorAmt, WaterSlice** water, int* waterAmt);

#endif