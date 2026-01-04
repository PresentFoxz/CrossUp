#ifndef CAM_H
#define CAM_H
#include "library.h"

Camera_t createCamera(float x, float y, float z, float rotX, float rotY, float rotZ, float fov, float near, float far);
EntStruct createEntity(float x, float y, float z, float rotX, float rotY, float rotZ, float sizeX, float sizeY, float sizeZ, float radius, float height, float frict, float fallFrict, int type, int* joints, int jointCount);
ObjStruct createObject(float x, float y, float z, float rotX, float rotY, float rotZ, float sizeX, float sizeY, float sizeZ, int type, int timer);

void moveCamera(Camera_t* cam, float dx, float dy, float dz);
void rotateCamera(Camera_t* cam, float rx, float ry, float rz);
void destroyCamera(Camera_t* cam);

void moveEntity(EntStruct* p, float dx, float dy, float dz);
void rotateEntity(EntStruct* p, float rx, float ry, float rz);
void destroyEntity(EntStruct* p);


#endif