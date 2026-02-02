#include "movement.h"

const int detectDist = 15;

static float wrapFloat(float value, float min, float max) {
    float range = max - min;
    if (range == 0.0f) return min;

    float result = fmodf(value - min, range);
    if (result < 0.0f) result += range;
    return min + result;
}

static void wrapPositionFloat(float* x, float* y, float* z) {
    *x = wrapFloat(*x, MINCOLX, MAXCOLX);
    *y = wrapFloat(*y, MINCOLY, MAXCOLY);
    *z = wrapFloat(*z, MINCOLZ, MAXCOLZ);
}

static void moveEnt(EntStruct* p, float mainYaw, float secondaryYaw, float secondaryStrength, float frict, float groundDelta, float airDelta, int type) {
    if (type == 0){
        if (p->grounded) {
            p->velocity.x += groundDelta * sinf(secondaryYaw);
            p->velocity.z += groundDelta * cosf(secondaryYaw);
        } else {
            p->velocity.x += airDelta * sinf(secondaryYaw);
            p->velocity.z += airDelta * cosf(secondaryYaw);
        }
    } else {
        if (p->grounded) {
            float landFrict = (frict + 1.0f) * 0.5f;
            
            p->velocity.x *= landFrict;
            p->velocity.z *= landFrict;
        } else {
            float forwardX = sinf(mainYaw);
            float forwardZ = cosf(mainYaw);

            float forwardVel = p->velocity.x * forwardX + p->velocity.z * forwardZ;

            if (forwardVel != 0.0f) {
                float absVel = fabsf(forwardVel);
                
                float dampingTotal = airDelta * secondaryStrength * 0.3f;
                if (absVel > 0.8f)       dampingTotal += airDelta * secondaryStrength * 1.6f;
                else if (absVel > 0.5f)  dampingTotal += airDelta * secondaryStrength * 1.0f;
                else if (absVel > 0.2f)  dampingTotal += airDelta * secondaryStrength * 0.5f;
                
                if (absVel <= dampingTotal) {
                    forwardVel = 0.0f;
                } else {
                    forwardVel -= (forwardVel / absVel) * dampingTotal;
                }
            }

            p->velocity.x = forwardVel * forwardX;
            p->velocity.z = forwardVel * forwardZ;
        }
    }
}

static void rotateTowards(EntStruct* p, float rot, float step){
    float current = FROM_FIXED32(p->surfRot);
    float delta = rot - current;
    
    while (delta < -M_PI) delta += degToRad(360.0f);
    while (delta > M_PI)  delta -= degToRad(360.0f);
    
    if (fabsf(delta) > degToRad(120.0f)) {
        p->surfRot = TO_FIXED32(rot);
    } else {
        if (delta > step)  delta = step;
        if (delta < -step) delta = -step;
        
        p->surfRot = TO_FIXED32(current + delta);
    }
    
    while (p->surfRot < 0) p->surfRot += TO_FIXED32(degToRad(360.0f));
    while (p->surfRot >= TO_FIXED32(degToRad(360.0f))) p->surfRot -= TO_FIXED32(degToRad(360.0f));
}

static void runColl(EntStruct* p){
    Vect3f pCollisionPos = {FROM_FIXED32(p->position.x), FROM_FIXED32(p->position.y), FROM_FIXED32(p->position.z)};
    float stepX = p->velocity.x / substeps;
    float stepY = p->velocity.y / substeps;
    float stepZ = p->velocity.z / substeps;

    float pRadius = FROM_FIXED32(p->radius);
    float pHeight = FROM_FIXED32(p->height);

    Triggers hitTrig = {0};

    for (int i = 0; i < substeps; i++) {
        pCollisionPos.x = FROM_FIXED32(p->position.x) + stepX;
        pCollisionPos.y = (FROM_FIXED32(p->position.y) - 0.5f) + stepY;
        pCollisionPos.z = FROM_FIXED32(p->position.z) + stepZ;

        wrapPositionFloat( &pCollisionPos.x, &pCollisionPos.y, &pCollisionPos.z );
        VectMf movePlr = cylinderInTriangle(pCollisionPos, pRadius, pHeight);
        if (movePlr.floor == -1 && movePlr.ceiling == -1 && movePlr.wall == -1) { break; }
        if (movePlr.floor == 1 && movePlr.ceiling == 1) {
            p->velocity.y = 0.0f;
            p->grounded = 0;
            p->coyote = 20;

            continue;
        }

        moveEntity(p, pCollisionPos.x, pCollisionPos.y + 0.5f, pCollisionPos.z);

        if (movePlr.floor == 0 && movePlr.ceiling == 0 && movePlr.wall == 0) { continue; }
        if (movePlr.floor == 1) {
            p->grounded = 1;
            p->coyote = 0;
            p->velocity.y = 0.0f;
            p->position.y += TO_FIXED32(movePlr.pos.y);
            stepY = p->velocity.y / substeps;
        }
        if (movePlr.ceiling == 1) {
            if (p->velocity.y < 0.0f) p->velocity.y = -0.5f;
            p->grounded = 0;
            p->coyote = 20;
            p->position.y += TO_FIXED32(movePlr.pos.y);
            stepY = p->velocity.y / substeps;
        }
        if (movePlr.wall == 1) {
            p->position.x += TO_FIXED32(movePlr.pos.x);
            p->position.z += TO_FIXED32(movePlr.pos.z);
        }

        hitTrig = cylinderInTrigger(pCollisionPos, pRadius, pHeight);
    }
}

void stateMachine(EntStruct* p){
    p->currentAnim = 0;

    if (p->grounded == 1 && ((p->velocity.x > 0.02 || p->velocity.x < -0.02) || (p->velocity.z > 0.02 || p->velocity.z < -0.02))) { p->currentAnim = 1; }
    else if (p->actions.plr.spin.actionUsed > 0) { p->currentAnim = 1; }
}

void movePlayerObj(EntStruct* p, Camera_t* c){
    PDButtons tapped, held;
    pd->system->getButtonState(&held, &tapped, NULL);

    float yawCam = FROM_FIXED32(c->rotation.y);
    float mainYaw = FROM_FIXED32(p->rotation.y);
    float secondaryStrength = 0.5f;
    float jumpFrict = 0.54f;

    if (p->grounded == 1 && p->actions.plr.spin.timer <= 0) { p->actions.plr.spin.actionUsed = 0; p->actions.plr.spin.timer = -1; }

    // === Movement ===
    if (tapped & kButtonB && p->actions.plr.spin.timer <= 0 && p->actions.plr.spin.actionUsed == 0) {
        if (p->grounded == 0) { if (p->velocity.y < 0.0f) { p->velocity.y = 0.54f; } else { p->velocity.y += 0.65f; } }
        p->actions.plr.spin.actionUsed = 1; p->actions.plr.spin.timer = 15;
    }
    if (p->actions.plr.spin.timer > 0 && p->actions.plr.spin.actionUsed == 1) { p->coyote = 11; p->fallFrict = 0.05f; jumpFrict = 1.0f; }

    if (held & kButtonA && (p->grounded == 1 || p->coyote <= 10)) {
        p->grounded = 0;
        p->velocity.y = jumpFrict;
    }

    // === Compute input vector ===
    float inputX = 0.0f;
    float inputZ = 0.0f;
    
    if (held & kButtonUp) inputZ += 1.0f;
    if (held & kButtonDown) inputZ -= 1.0f;
    if (held & kButtonLeft) inputX -= 1.0f;
    if (held & kButtonRight) inputX += 1.0f;

    // === Map input to camera-relative movement ===
    float dirX = inputX * cosf(yawCam) + inputZ * sinf(yawCam);
    float dirZ = inputZ * cosf(yawCam) - inputX * sinf(yawCam);
    
    if (dirX != 0.0f || dirZ != 0.0f) {
        float targetYaw = atan2f(dirX, dirZ);

        if (p->grounded == 1) {
            p->rotation.y = TO_FIXED32(targetYaw);
            rotateTowards(p, FROM_FIXED32(p->rotation.y), 0.45f);
        } else { rotateTowards(p, targetYaw, 0.3f); }

        p->ifMove++;
    } else {
        p->ifMove = 0;
    }

    if (p->ifMove > 0) { moveEnt(p, FROM_FIXED32(p->rotation.y), FROM_FIXED32(p->surfRot), secondaryStrength, p->frict, 0.22f, 0.05f, 0); }

    p->velocity.y -= p->fallFrict;
    if (p->velocity.y < -5.0f){ p->velocity.y = -5.0f; }

    runColl(p);
    moveEnt(p, FROM_FIXED32(p->rotation.y), FROM_FIXED32(p->surfRot), secondaryStrength, p->frict, 0.22f, 0.05f, 1);

    p->coyote++;
    if (p->grounded == 1) {
        p->coyote = 0;
        p->groundTimer++;

        if (p->fallFrict != 0.08f) { p->fallFrict = 0.08f; }

        if (inputX != 0.0f || inputZ != 0.0f){
            p->groundTimer = 10;
        }
    } else {
        if (inputX != 0.0f || inputZ != 0.0f){
            p->groundTimer = 10;
        } else {
            p->groundTimer = 0;
        }
    }

    if (p->actions.plr.spin.timer > 0) p->actions.plr.spin.timer--;
    stateMachine(p);
}

void updateCamera(Camera_t* cam, EntStruct* ent, float radius) {
    float pitch = FROM_FIXED32(cam->rotation.x);
    float yaw   = FROM_FIXED32(cam->rotation.y);
    float smoothOrbit = 0.1f;

    float offsetX = cosf(pitch) * sinf(yaw) * radius;
    float offsetY = sinf(pitch) * radius;
    float offsetZ = cosf(pitch) * cosf(yaw) * radius;

    float targetX = FROM_FIXED32(ent->position.x) - offsetX;
    float targetY = FROM_FIXED32(ent->position.y) + offsetY;
    float targetZ = FROM_FIXED32(ent->position.z) - offsetZ;

    cam->position.x += TO_FIXED32((targetX - FROM_FIXED32(cam->position.x)) * smoothOrbit);
    cam->position.y += TO_FIXED32((targetY - FROM_FIXED32(cam->position.y)) * smoothOrbit);
    cam->position.z += TO_FIXED32((targetZ - FROM_FIXED32(cam->position.z)) * smoothOrbit);
}

void handleCameraInput(Camera_t* cam) {
    float rotY_delta = -0.03f;
    float rotX_delta = -0.1f;
    
    float crankDelta = pd->system->getCrankChange();
    cam->rotation.y += TO_FIXED32(crankDelta * rotY_delta);
    cam->rotation.x =  TO_FIXED32(degToRad(40.0f));
    
    qfixed32_t minPitchY = TO_FIXED32(degToRad(  0.0f));
    qfixed32_t maxPitchY = TO_FIXED32(degToRad(360.0f));
    if (cam->rotation.y < minPitchY) cam->rotation.y += maxPitchY;
    if (cam->rotation.y > maxPitchY) cam->rotation.y -= maxPitchY;
    
    qfixed32_t minPitchX = TO_FIXED32(degToRad(-90.0f));
    qfixed32_t maxPitchX = TO_FIXED32(degToRad( 90.0f));
    if (cam->rotation.x < minPitchX) cam->rotation.x = minPitchX;
    if (cam->rotation.x > maxPitchX) cam->rotation.x = maxPitchX;
}

// == Entity Movements == //

void moveEntObj(EntStruct* e, EntStruct* p) {
    float mainYaw = FROM_FIXED32(e->rotation.y);
    float secondaryStrength = 0.5f;

    moveEnt(e, FROM_FIXED32(e->rotation.y), FROM_FIXED32(e->surfRot), secondaryStrength, e->frict, 0.13f, 0.0f, 0);

    e->velocity.y -= e->fallFrict;

    if (e->velocity.y < -5.0f){ e->velocity.y = -5.0f; }

    runColl(e);

    e->coyote++;
    if (e->grounded == 1) {
        e->coyote = 0;

        if (e->velocity.x > -0.001 && e->velocity.x < 0.001) { e->velocity.x = 0.0f; }
        if (e->velocity.z > -0.001 && e->velocity.z < 0.001) { e->velocity.z = 0.0f; }
    }

    float dx = FROM_FIXED32(p->position.x) - FROM_FIXED32(e->position.x);
    float dz = FROM_FIXED32(p->position.z) - FROM_FIXED32(e->position.z);
    if (dx*dx + dz*dz < detectDist*detectDist) {
        float inputX = (FROM_FIXED32(p->position.x) - FROM_FIXED32(e->position.x));
        float inputZ = (FROM_FIXED32(p->position.z) - FROM_FIXED32(e->position.z));
        float targetYaw = atan2f(inputX, inputZ);

        e->rotation.y = TO_FIXED32(targetYaw);
        e->countdown = 10;
    }

    e->countdown--;
    if (e->countdown <= 0) {
        float inputX = randomFloat(-1.0f, 1.0f);
        float inputZ = randomFloat(-1.0f, 1.0f);
        float targetYaw = atan2f(inputX, inputZ);

        e->rotation.y = TO_FIXED32(targetYaw);
        if (randomInt(0, 100) > 99 && (e->grounded == 1 || e->coyote <= 10)) { e->velocity.y = 0.94f; e->grounded = 0; }

        e->countdown = randomInt(30, 100);
    }

    moveEnt(e, FROM_FIXED32(e->rotation.y), FROM_FIXED32(e->surfRot), secondaryStrength, e->frict, 0.13f, 0.0f, 1);
    stateMachine(p);
}

static void objectTypes(ObjStruct obj){
    obj.timer--;

    obj.position.x += TO_FIXED32(obj.velocity.x);
    obj.position.y += TO_FIXED32(obj.velocity.y);
    obj.position.z += TO_FIXED32(obj.velocity.z);
}