#include "entities.h"

Camera createCamera(float x, float y, float z, float rotX, float rotY, float rotZ, float fov, float near, float far) {
    Camera cam;
    cam.position.x = TO_FIXED32(x);
    cam.position.y = TO_FIXED32(y);
    cam.position.z = TO_FIXED32(z);

    cam.rotation.x = TO_FIXED32(rotX);
    cam.rotation.y = TO_FIXED32(rotY);
    cam.rotation.z = TO_FIXED32(rotZ);

    cam.fov =        TO_FIXED32(fov);
    cam.nearPlane =  TO_FIXED32(near);
    cam.farPlane =   TO_FIXED32(far);
    return cam;
}

void moveCamera(Camera* cam, float dx, float dy, float dz) { if (cam) { cam->position.x += TO_FIXED32(dx); cam->position.y += TO_FIXED32(dy); cam->position.z += TO_FIXED32(dz); } }
void rotateCamera(Camera* cam, float rx, float ry, float rz) { if (cam) { cam->rotation.x += TO_FIXED32(rx); cam->rotation.y += TO_FIXED32(ry); cam->rotation.z += TO_FIXED32(rz); } }
void destroyCamera(Camera* cam) { if (cam) { cam = pd->system->realloc(cam, 0); } }

EntStruct createEntity(float x, float y, float z, float rotX, float rotY, float rotZ, float sizeX, float sizeY, float sizeZ, float radius, float height, float frict, float fallFrict, int type) {
    EntStruct p;
    p.position.x = TO_FIXED32(x);
    p.position.y = TO_FIXED32(y);
    p.position.z = TO_FIXED32(z);
    
    p.rotation.x = TO_FIXED32(rotX);
    p.rotation.y = TO_FIXED32(rotY);
    p.rotation.z = TO_FIXED32(rotZ);

    p.size.x = TO_FIXED32(sizeX);
    p.size.y = TO_FIXED32(sizeY);
    p.size.z = TO_FIXED32(sizeZ);

    p.velocity.x = 0.0f;
    p.velocity.y = 0.0f;
    p.velocity.z = 0.0f;

    p.frict = frict;
    p.fallFrict = fallFrict;

    p.surfRot =    TO_FIXED32(rotY);

    p.radius =     TO_FIXED32(radius);
    p.height =     TO_FIXED32(height);

    p.type = type;
    p.grounded = 0;
    p.coyote = 0;
    p.ifMove = 0;
    p.crouch = 0;

    p.countdown = 0;
    p.rotDir = 0;
    return p;
}

void moveEntity(EntStruct* p, float dx, float dy, float dz) { if (p) { p->position.x += TO_FIXED32(dx); p->position.y += TO_FIXED32(dy); p->position.z += TO_FIXED32(dz); } }
void rotateEntity(EntStruct* p, float rx, float ry, float rz) { if (p) { p->rotation.x += TO_FIXED32(rx); p->rotation.y += TO_FIXED32(ry); p->rotation.z += TO_FIXED32(rz); } }
void destroyEntity(EntStruct* p) { if (p) { p = pd->system->realloc(p, 0); } }

EntStruct createPlayer(float x, float y, float z, float rotX, float rotY, float rotZ, float sizeX, float sizeY, float sizeZ, float radius, float height, float frict, float fallFrict, int type) {
    EntStruct p;
    p.position.x = TO_FIXED32(x);
    p.position.y = TO_FIXED32(y);
    p.position.z = TO_FIXED32(z);
    
    p.rotation.x = TO_FIXED32(rotX);
    p.rotation.y = TO_FIXED32(rotY);
    p.rotation.z = TO_FIXED32(rotZ);

    p.size.x = TO_FIXED32(sizeX);
    p.size.y = TO_FIXED32(sizeY);
    p.size.z = TO_FIXED32(sizeZ);

    p.velocity.x = 0.0f;
    p.velocity.y = 0.0f;
    p.velocity.z = 0.0f;

    p.frict = frict;
    p.fallFrict = fallFrict;

    p.surfRot =    TO_FIXED32(rotY);

    p.radius =     TO_FIXED32(radius);
    p.height =     TO_FIXED32(height);
    
    p.type = type;
    p.grounded = 0;
    p.coyote = 0;
    p.ifMove = 0;
    p.crouch = 0;

    p.countdown = 0;
    p.rotDir = 0;
    return p;
}

void movePlayer(EntStruct* p, float dx, float dy, float dz) { if (p) { p->position.x = TO_FIXED32(dx); p->position.y = TO_FIXED32(dy); p->position.z = TO_FIXED32(dz); } }
void rotatePlayer(EntStruct* p, float rx, float ry, float rz) { if (p) { p->rotation.x += TO_FIXED32(rx); p->rotation.y += TO_FIXED32(ry); p->rotation.z += TO_FIXED32(rz); } }
void destroyPlayer(EntStruct* p) { if (p) { p = pd->system->realloc(p, 0); } }