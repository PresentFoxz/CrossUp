#ifndef MOVEMENT_H
#define MOVEMENT_H
#include "libRay.h"
#include "../../Foxgine/entities.h"
#include "../../Foxgine/collisions.h"

#define V_LOOK_SENS 0.25f
#define H_LOOK_SENS 0.25f

#define MINCOLX -2000
#define MINCOLY -2000
#define MINCOLZ -2000

#define MAXCOLX 2000
#define MAXCOLY 2000
#define MAXCOLZ 2000

void movePlayerObj(EntStruct* p, Camera_t* c);
void moveEntObj(EntStruct* e, EntStruct* p);
static void objectTypes(ObjStruct obj);

void handleCameraInput(Camera_t* cam);
void updateCamera(Camera_t* cam, EntStruct* ent, float radius);

void runInputBuffer();

#endif