#ifndef ENGINE_H
#define ENGINE_H
#include "../allFiles/library.h"
#include "../allFiles/draw.h"

void generateMap(Mesh_t mapArray);
void generateTriggers(Vect3f pos, Vect3f size);

void addObjToWorld3D(Vect3f pos, Vect3f rot, Vect3f size, Camera_t cCam, float depthOffset, Mesh_t model, bool lightUse);
void addObjToWorld2D(Vect3f pos, Vect3f rot, Vect3f size, Camera_t cCam, float objDepthOffset, float sprtDepthOffset, int anim, int animFrame);

void shootRender(Camera_t cam, textAnimsAtlas* allObjArray2D);
void resetAllArrays();

void addEnt(Vect3f pos, Vect3f rot, Vect3f size, float radius, float height, float frict, float fallFrict, int type, ModelType objType, VertAnims* entArray, Objects* allEnts, Dimentions dimention);
void addLightPoint(Vect3f pos, uint8_t lightLevel, float falloff);
void precomputedFunctions(Camera_t* cam);

#endif