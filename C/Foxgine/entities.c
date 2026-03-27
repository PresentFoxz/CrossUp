#include "entities.h"

Camera_t createCamera(float x, float y, float z, float rotX, float rotY, float rotZ, float fov, float near, float far) {
    Camera_t cam;
    cam.position.x = x;
    cam.position.y = y;
    cam.position.z = z;

    cam.rotation.x = degToRad(rotX);
    cam.rotation.y = degToRad(rotY);
    cam.rotation.z = degToRad(rotZ);

    cam.fov        = fov;
    cam.nearPlane  = near;
    cam.farPlane   = far;
    cam.projDist   = 0;

    cam.fVect.x    = 0.0f;
    cam.fVect.y    = 0.0f;
    cam.fVect.z    = 0.0f;

    return cam;
}

void moveCamera(Camera_t* cam, float dx, float dy, float dz) { if (cam) { cam->position.x += dx; cam->position.y += dy; cam->position.z += dz; } }
void rotateCamera(Camera_t* cam, float rx, float ry, float rz) { if (cam) { cam->rotation.x += rx; cam->rotation.y += ry; cam->rotation.z += rz; } }
void destroyCamera(Camera_t* cam) { if (cam) { cam = pd_realloc(cam, 0); } }

EntStruct createEntity(float x, float y, float z, float rotX, float rotY, float rotZ, float sizeX, float sizeY, float sizeZ, float radius, float height, float frict, float fallFrict, int type, Dimentions dimention) {
    EntStruct p;
    p.position.x = x;
    p.position.y = y;
    p.position.z = z;
    
    p.rotation.x = degToRad(rotX);
    p.rotation.y = degToRad(rotY);
    p.rotation.z = degToRad(rotZ);

    p.size.x = sizeX;
    p.size.y = sizeY;
    p.size.z = sizeZ;

    p.velocity.x = 0.0f;
    p.velocity.y = 0.0f;
    p.velocity.z = 0.0f;

    p.frict = frict;
    p.fallFrict = fallFrict;

    p.surfRot =    rotY;

    p.radius =     radius;
    p.height =     height;

    p.type = type;
    p.grounded = 0;
    p.groundTimer = 0;
    p.coyote = 0;
    p.ifMove = 0;

    p.countdown = 0;

    p.frameCount = 0;
    p.currentFrame = 0;
    p.currentAnim = 0;
    p.lastAnim = 0;

    p.dimention = dimention;
    
    return p;
}

void moveEntity(EntStruct* p, float dx, float dy, float dz) { if (p) { p->position.x = dx; p->position.y = dy; p->position.z = dz; } }
void rotateEntity(EntStruct* p, float rx, float ry, float rz) { if (p) { p->rotation.x = rx; p->rotation.y = ry; p->rotation.z = rz; } }
void destroyEntity(EntStruct* p) { if (p) { p = pd_realloc(p, 0); } }

ObjStruct createObject(float x, float y, float z, float rotX, float rotY, float rotZ, float sizeX, float sizeY, float sizeZ, int type, int timer, Dimentions dimention){
    ObjStruct o;
    o.position.x = x;
    o.position.y = y;
    o.position.z = z;
    
    o.rotation.x = rotX;
    o.rotation.y = rotY;
    o.rotation.z = rotZ;

    o.size.x = sizeX;
    o.size.y = sizeY;
    o.size.z = sizeZ;

    o.velocity.x = 0.0f;
    o.velocity.y = 0.0f;
    o.velocity.z = 0.0f;

    o.type = type;
    o.timer = timer;

    o.dimention = dimention;

    return o;
}