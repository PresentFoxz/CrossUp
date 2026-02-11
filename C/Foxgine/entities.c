#include "entities.h"

Camera_t createCamera(float x, float y, float z, float rotX, float rotY, float rotZ, float fov, float near, float far) {
    Camera_t cam;
    cam.position.x = TO_FIXED24_8(x);
    cam.position.y = TO_FIXED24_8(y);
    cam.position.z = TO_FIXED24_8(z);

    cam.rotation.x = TO_FIXED24_8(degToRad(rotX));
    cam.rotation.y = TO_FIXED24_8(degToRad(rotY));
    cam.rotation.z = TO_FIXED24_8(degToRad(rotZ));

    cam.fov =        fov;
    cam.nearPlane =  near;
    cam.farPlane =   far;
    return cam;
}

void moveCamera(Camera_t* cam, float dx, float dy, float dz) { if (cam) { cam->position.x += TO_FIXED24_8(dx); cam->position.y += TO_FIXED24_8(dy); cam->position.z += TO_FIXED24_8(dz); } }
void rotateCamera(Camera_t* cam, float rx, float ry, float rz) { if (cam) { cam->rotation.x += TO_FIXED24_8(rx); cam->rotation.y += TO_FIXED24_8(ry); cam->rotation.z += TO_FIXED24_8(rz); } }
void destroyCamera(Camera_t* cam) { if (cam) { cam = pd_realloc(cam, 0); } }

EntStruct createEntity(float x, float y, float z, float rotX, float rotY, float rotZ, float sizeX, float sizeY, float sizeZ, float radius, float height, float frict, float fallFrict, int type) {
    EntStruct p;
    p.position.x = TO_FIXED24_8(x);
    p.position.y = TO_FIXED24_8(y);
    p.position.z = TO_FIXED24_8(z);
    
    p.rotation.x = TO_FIXED24_8(degToRad(rotX));
    p.rotation.y = TO_FIXED24_8(degToRad(rotY));
    p.rotation.z = TO_FIXED24_8(degToRad(rotZ));

    p.size.x = TO_FIXED24_8(sizeX);
    p.size.y = TO_FIXED24_8(sizeY);
    p.size.z = TO_FIXED24_8(sizeZ);

    p.velocity.x = 0.0f;
    p.velocity.y = 0.0f;
    p.velocity.z = 0.0f;

    p.frict = frict;
    p.fallFrict = fallFrict;

    p.surfRot =    TO_FIXED24_8(rotY);

    p.radius =     TO_FIXED24_8(radius);
    p.height =     TO_FIXED24_8(height);

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
    
    return p;
}

void moveEntity(EntStruct* p, float dx, float dy, float dz) { if (p) { p->position.x = TO_FIXED24_8(dx); p->position.y = TO_FIXED24_8(dy); p->position.z = TO_FIXED24_8(dz); } }
void rotateEntity(EntStruct* p, float rx, float ry, float rz) { if (p) { p->rotation.x = TO_FIXED24_8(rx); p->rotation.y = TO_FIXED24_8(ry); p->rotation.z = TO_FIXED24_8(rz); } }
void destroyEntity(EntStruct* p) { if (p) { p = pd_realloc(p, 0); } }

ObjStruct createObject(float x, float y, float z, float rotX, float rotY, float rotZ, float sizeX, float sizeY, float sizeZ, int type, int timer){
    ObjStruct o;
    o.position.x = TO_FIXED24_8(x);
    o.position.y = TO_FIXED24_8(y);
    o.position.z = TO_FIXED24_8(z);
    
    o.rotation.x = TO_FIXED24_8(rotX);
    o.rotation.y = TO_FIXED24_8(rotY);
    o.rotation.z = TO_FIXED24_8(rotZ);

    o.size.x = TO_FIXED24_8(sizeX);
    o.size.y = TO_FIXED24_8(sizeY);
    o.size.z = TO_FIXED24_8(sizeZ);

    o.velocity.x = 0.0f;
    o.velocity.y = 0.0f;
    o.velocity.z = 0.0f;

    o.type = type;
    o.timer = timer;

    return o;
}