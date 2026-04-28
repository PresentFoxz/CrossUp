#ifndef ENGINE_H
#define ENGINE_H
#include "../allFiles/library.h"
#include "../allFiles/draw.h"

void generateMap(Mesh_t mapArray, Vector3f pos);
void generateTriggers(Vector3f pos, Vector3f size);

void addWaves(WaterSlice* water, int index, int wAmt);
void addWaveToWorld3D(LineSlice* line, Vector2i boundMin, Vector2i boundMax, Camera_t cCam);
void addObjToWorld3D(Vector3f pos, Vector3f rot, Vector3f size, Camera_t cCam, float depthOffset, Mesh_t model, bool lightUse);
void addObjToWorld2D(Vector3f pos, Vector3f rot, Vector3f size, Camera_t cCam, float objDepthOffset, float sprtDepthOffset, int anim, int animFrame);

void shootRender(Camera_t cam, textAnimsAtlas* allObjArray2D);
void resetAllArrays();

void addEnt(Vector3f pos, Vector3f rot, Vector3f size, float radius, float height, float frict, float fallFrict, int type, ModelType objType, VertAnims* entArray, Objects* allEnts, Dimentions dimention);
void addLightPoint(Vector3f pos, uint8_t lightLevel, float falloff);
void precomputedFunctions(Camera_t* cam);

#endif