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
    float forwardX = sinf(mainYaw);
    float forwardZ = cosf(mainYaw);

    if (type == 0){
        if (p->grounded) {
            p->velocity.x += groundDelta * sinf(mainYaw);
            p->velocity.z += groundDelta * cosf(mainYaw);
        } else {
            p->velocity.x += airDelta * sinf(secondaryYaw);
            p->velocity.z += airDelta * cosf(secondaryYaw);
        }
    } else {
        if (p->grounded == 1 && p->groundTimer > 3) {
            float landFrict = 1.0f - frict * 0.1f;
            
            p->velocity.x *= landFrict;
            p->velocity.z *= landFrict;
        } else {
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

static void rotateSurfTowards(EntStruct* p, float rot, float step){
    float current = p->surfRot;
    float delta = rot - current;
    
    while (delta < -M_PI) delta += degToRad(360.0f);
    while (delta > M_PI)  delta -= degToRad(360.0f);
    
    if (fabsf(delta) > degToRad(120.0f)) {
        p->surfRot = rot;
    } else {
        if (delta > step)  delta = step;
        if (delta < -step) delta = -step;
        
        p->surfRot = (current + delta);
    }
    
    while (p->surfRot < 0) p->surfRot += degToRad(360.0f);
    while (p->surfRot >= degToRad(360.0f)) p->surfRot -= degToRad(360.0f);
}

static void rotatePlrTowards(EntStruct* p, float rot, float step){
    float current = p->rotation.y;
    float delta = rot - current;
    
    while (delta < -M_PI) delta += degToRad(360.0f);
    while (delta > M_PI)  delta -= degToRad(360.0f);
    
    if (fabsf(delta) > degToRad(120.0f)) {
        p->rotation.y = rot;
    } else {
        if (delta > step)  delta = step;
        if (delta < -step) delta = -step;
        
        p->rotation.y = (current + delta);
    }
    
    while (p->rotation.y < 0) p->rotation.y += degToRad(360.0f);
    while (p->rotation.y >= degToRad(360.0f)) p->rotation.y -= degToRad(360.0f);
}

static void runColl(EntStruct* p) {
    Vect3f pCollisionPos = p->position;
    float stepX = p->velocity.x / substeps;
    float stepY = p->velocity.y / substeps;
    float stepZ = p->velocity.z / substeps;

    float pRadius = p->radius;
    float pHeight = p->height;

    Triggers hitTrig = {0};

    for (int i = 0; i < substeps; i++) {
        pCollisionPos.x = (p->position.x) + stepX;
        pCollisionPos.y = (p->position.y - 0.5f) + stepY;
        pCollisionPos.z = (p->position.z) + stepZ;

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
            p->position.y += movePlr.pos.y;
            stepY = p->velocity.y / substeps;
        }
        if (movePlr.ceiling == 1) {
            if (p->velocity.y < 0.0f) p->velocity.y = -0.5f;
            p->grounded = 0;
            p->coyote = 20;
            p->position.y += movePlr.pos.y;
            stepY = p->velocity.y / substeps;
        }
        if (movePlr.wall == 1) {
            p->position.x += movePlr.pos.x;
            p->position.z += movePlr.pos.z;
        }

        hitTrig = cylinderInTrigger(pCollisionPos, pRadius, pHeight);
    }
}

void stateMachine(EntStruct* p){
    p->currentAnim = 0;

    if (p->grounded == 1 && ((p->velocity.x > 0.02 || p->velocity.x < -0.02) || (p->velocity.z > 0.02 || p->velocity.z < -0.02))) { p->currentAnim = 1; }
    else if (p->actions.plr.spin.actionUsed > 0) { p->currentAnim = 1; }
}

void movePlayerObj(EntStruct* p, Camera_t* c, int type){
    float yawCam = c->rotation.y;
    float mainYaw = p->rotation.y;
    float secondaryStrength = 0.5f;
    float jumpFrict = 0.54f;
    
    if (type == 0) {
        if (inpBuf.A && (p->grounded == 1 || p->coyote <= 10)) {
            if (p->grounded == 1) { p->velocity.x *= 1.15f; p->velocity.z *= 1.15f; }

            p->grounded = 0;
            p->velocity.y = jumpFrict;
        } if (!inpBuf.A && (p->grounded == 0 && p->coyote <= 10)) p->coyote = 11;
        
        float inputX = 0.0f;
        float inputZ = 0.0f;
        
        if (inpBuf.UP) inputZ += 1.0f;
        if (inpBuf.DOWN) inputZ -= 1.0f;
        if (inpBuf.LEFT) inputX -= 1.0f;
        if (inpBuf.RIGHT) inputX += 1.0f;
        
        float dirX = inputX * cosf(yawCam) + inputZ * sinf(yawCam);
        float dirZ = inputZ * cosf(yawCam) - inputX * sinf(yawCam);
        
        if (dirX != 0.0f || dirZ != 0.0f) {
            float targetYaw = atan2f(dirX, dirZ);
            rotateSurfTowards(p, targetYaw, 0.2f);
                
            if (p->grounded == 1 && p->groundTimer >= 3) rotatePlrTowards(p, targetYaw, 0.2f);

            p->ifMove++;
        } else {
            p->ifMove = 0;
        }

        if (p->ifMove > 0) { moveEnt(p, p->rotation.y, p->surfRot, secondaryStrength, p->frict, 0.12f, 0.05f, 0); }

        p->velocity.y -= p->fallFrict;
        if (p->velocity.y < -5.0f){ p->velocity.y = -5.0f; }

        runColl(p);
        moveEnt(p, p->rotation.y, p->surfRot, secondaryStrength, p->frict, 0.12f, 0.05f, 1);

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
    } else {
        p->velocity.y -= p->fallFrict;
        if (p->velocity.y < -5.0f){ p->velocity.y = -5.0f; }

        runColl(p);
        moveEnt(p, p->rotation.y, p->surfRot, secondaryStrength, p->frict, 0.12f, 0.05f, 1);

        if (p->actions.plr.spin.timer > 0) p->actions.plr.spin.timer--;
        stateMachine(p);
    }
}

void updateCamera(Camera_t* cam, EntStruct* ent, float radius) {
    float pitch = cam->rotation.x;
    float yaw   = cam->rotation.y;
    float smoothOrbit = 0.1f;

    float offsetX = cosf(pitch) * sinf(yaw) * radius;
    float offsetY = sinf(pitch) * radius;
    float offsetZ = cosf(pitch) * cosf(yaw) * radius;

    float targetX = ent->position.x - offsetX;
    float targetY = ent->position.y + offsetY;
    float targetZ = ent->position.z - offsetZ;

    cam->position.x += (targetX - cam->position.x) * smoothOrbit;
    cam->position.y += (targetY - cam->position.y) * smoothOrbit;
    cam->position.z += (targetZ - cam->position.z) * smoothOrbit;
}

void handleCameraInput(Camera_t* cam) {
    float rotY_delta = -0.03f;
    float rotX_delta = -0.1f;
    
    float crankDelta = pd->system->getCrankChange();
    cam->rotation.y += crankDelta * rotY_delta;
    cam->rotation.x =  degToRad(40.0f);
    
    float minPitchY = degToRad(  0.0f);
    float maxPitchY = degToRad(360.0f);
    if (cam->rotation.y < minPitchY) cam->rotation.y += maxPitchY;
    if (cam->rotation.y > maxPitchY) cam->rotation.y -= maxPitchY;
    
    float minPitchX = degToRad(-90.0f);
    float maxPitchX = degToRad( 90.0f);
    if (cam->rotation.x < minPitchX) cam->rotation.x = minPitchX;
    if (cam->rotation.x > maxPitchX) cam->rotation.x = maxPitchX;
}

void flyCameraInput(Camera_t* cam) {
    float rotY_delta = -0.03f;
    float rotX_delta = -0.1f;
    float crankDelta = pd->system->getCrankChange();
    float flyVel  = 0.008f;
    float camRotXSPD = 0.08f;
    float camRotYSPD = 0.0f;

    float yaw   = cam->rotation.y;
    float pitch = cam->rotation.x;

    if (inpBuf.UP) {
        cam->position.x += flyVel * sin(yaw);
        cam->position.z += flyVel * cos(yaw);
    }
    if (inpBuf.DOWN) {
        cam->position.x -= flyVel * sin(yaw);
        cam->position.z -= flyVel * cos(yaw);
    }

    if (inpBuf.A) { cam->position.y += 0.8f; }
    if (inpBuf.B) { cam->position.y -= 0.8f; }

    if (inpBuf.LEFT) { cam->rotation.y -= camRotXSPD; }
    else if (inpBuf.RIGHT) { cam->rotation.y += camRotXSPD; }
    cam->rotation.x += crankDelta * rotY_delta;
    
    float minPitchY = degToRad(  0.0f);
    float maxPitchY = degToRad(360.0f);
    if (cam->rotation.y < minPitchY) cam->rotation.y += maxPitchY;
    if (cam->rotation.y > maxPitchY) cam->rotation.y -= maxPitchY;
    
    float minPitchX = degToRad(-90.0f);
    float maxPitchX = degToRad( 90.0f);
    if (cam->rotation.x < minPitchX) cam->rotation.x = minPitchX;
    if (cam->rotation.x > maxPitchX) cam->rotation.x = maxPitchX;
}

// == Entity Movements == //

void moveEntObj(EntStruct* e, EntStruct* p) {
    float mainYaw = e->rotation.y;
    float secondaryStrength = 0.5f;

    moveEnt(e, e->rotation.y, e->surfRot, secondaryStrength, e->frict, 0.13f, 0.0f, 0);

    e->velocity.y -= e->fallFrict;

    if (e->velocity.y < -5.0f){ e->velocity.y = -5.0f; }

    runColl(e);

    e->coyote++;
    if (e->grounded == 1) {
        e->coyote = 0;

        if (e->velocity.x > -0.001 && e->velocity.x < 0.001) { e->velocity.x = 0.0f; }
        if (e->velocity.z > -0.001 && e->velocity.z < 0.001) { e->velocity.z = 0.0f; }
    }

    float dx = p->position.x - e->position.x;
    float dz = p->position.z - e->position.z;
    if (dx*dx + dz*dz < detectDist*detectDist) {
        float inputX = (p->position.x) - e->position.x;
        float inputZ = (p->position.z) - e->position.z;
        float targetYaw = atan2f(inputX, inputZ);
        
        e->rotation.y = targetYaw;
        e->surfRot = e->rotation.y;
        e->actions.ent.countdown = 10;
    }

    e->actions.ent.countdown--;
    if (e->actions.ent.countdown <= 0) {
        float inputX = randomFloat(-1.0f, 1.0f);
        float inputZ = randomFloat(-1.0f, 1.0f);
        float targetYaw = atan2f(inputX, inputZ);

        e->rotation.y = targetYaw;
        e->surfRot = e->rotation.y;
        if (randomInt(0, 100) > 99 && (e->grounded == 1 || e->coyote <= 10)) { e->velocity.y = 0.94f; e->grounded = 0; }

        e->actions.ent.countdown = randomInt(30, 100);
    }

    moveEnt(e, e->rotation.y, e->surfRot, secondaryStrength, e->frict, 0.13f, 0.0f, 1);
    // stateMachine(e);
}

static void objectTypes(ObjStruct obj){
    obj.timer--;

    obj.position.x += obj.velocity.x;
    obj.position.y += obj.velocity.y;
    obj.position.z += obj.velocity.z;
}