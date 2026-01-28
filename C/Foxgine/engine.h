#ifndef ENGINE_H
#define ENGINE_H
#include "../allFiles/library.h"
#include "../allFiles/mesh.h"
#include "../allFiles/structs.h"
#include "../textures/allTexts.h"

void generateMap(Mesh_t mapArray);
void generateTriggers(Vect3f pos, Vect3f size);
void generateTextures(textAtlas** textAtlasMem, int memArea);

void addObjectToWorld(Vect3f pos, Vect3f rot, Vect3f size, Camera_t cCam, float depthOffset, int triCount, Vect3f* data, int* backFace, int* colorArray, int lineDraw, int distMod);
void shootRender(float CamYDirSin, float CamYDirCos, float CamXDirSin, float CamXDirCos, float CamZDirSin, float CamZDirCos, Camera_t cam, textAtlas* textAtlasMem);
void resetAllVariables();

void addEnt(Vect3f pos, Vect3f rot, Vect3f size, float radius, float height, float frict, float fallFrict, int type, ModelType objType, VertAnims* entArray, Objects* allEnts);

#endif