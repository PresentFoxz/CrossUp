#ifndef CAM_H
#define CAM_H
#include "library.h"

Camera createCamera(float x, float y, float z, float rotX, float rotY, float rotZ, float fov, float near, float far);
EntStruct createPlayer(float x, float y, float z, float rotX, float rotY, float rotZ, float sizeX, float sizeY, float sizeZ, float radius, float height, float frict, float fallFrict, int type);
EntStruct createEntity(float x, float y, float z, float rotX, float rotY, float rotZ, float sizeX, float sizeY, float sizeZ, float radius, float height, float frict, float fallFrict, int type);

void moveCamera(Camera* cam, float dx, float dy, float dz);
void rotateCamera(Camera* cam, float rx, float ry, float rz);
void destroyCamera(Camera* cam);

void movePlayer(EntStruct* p, float dx, float dy, float dz);
void rotatePlayer(EntStruct* p, float rx, float ry, float rz);
void destroyPlayer(EntStruct* p);

void moveEntity(EntStruct* p, float dx, float dy, float dz);
void rotateEntity(EntStruct* p, float rx, float ry, float rz);
void destroyEntity(EntStruct* p);


#endif