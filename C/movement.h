#ifndef MOVEMENT_H
#define MOVEMENT_H
#include "library.h"
#include "entities.h"

#define V_LOOK_SENS 0.25f
#define H_LOOK_SENS 0.25f

#define MINCOLX -2000
#define MINCOLY -2000
#define MINCOLZ -2000

#define MAXCOLX 2000
#define MAXCOLY 2000
#define MAXCOLZ 2000

void movePlayerObj(EntStruct* p, Camera* c);
void moveEntObj(EntStruct* e, EntStruct* p);

void handleCameraInput(Camera* cam);
void updateCamera(Camera* cam, EntStruct* ent, float radius);

#endif