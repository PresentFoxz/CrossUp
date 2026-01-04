#include "library.h"
#include "entities.h"
#include "draw.h"
#include "movement.h"
#include "collisions.h"
#include "Objects/allMeshes.h"

Camera_t cam;
worldTris* allPoints;
staticPoints* static3D;
worldTris* entModels;
int gameStart = 1;

qfixed16_t zBuffer[sW * sH];

int rendAmt = 0;
int entAmt = 0;
int allAmt = 0;
int staticAmt = 0;

const int colRend = 0;
int renderRadius = 85;

Objects* allEnts;
EntStruct player;
#define MAX_ENTITIES 240

Mesh_t* mapArray;
Mesh_t* objArray;
PlayerModel_t* entArray;
const int maxProjs = 1;
const int maxMaps = 1;
const int maxEntStored = 1;

int modelIndex = 0;
int allPointsCount = 0;
const int lengthJoints = 3;

Vect3f rotModelSet;

Vect3f camForward;
Camera_t setCam;

int gameScreen = 0;

static void generateMap(int len){
    allPoints = realloc(allPoints, sizeof(worldTris) * 0);
    resetCollisionSurface();
    for (int i=0; i < len; i++){
        int idx[3] = {(i*3), (i*3)+1, (i*3)+2};
        addCollisionSurface(
            (Vect3f){mapArray[modelIndex].data[idx[0]].x, mapArray[modelIndex].data[idx[0]].y, mapArray[modelIndex].data[idx[0]].z},
            (Vect3f){mapArray[modelIndex].data[idx[1]].x, mapArray[modelIndex].data[idx[1]].y, mapArray[modelIndex].data[idx[1]].z},
            (Vect3f){mapArray[modelIndex].data[idx[2]].x, mapArray[modelIndex].data[idx[2]].y, mapArray[modelIndex].data[idx[2]].z},
            SURFACE_NONE
        );
    }

    rendAmt = len;
    allPointsCount = 0;
    for (int i=0; i < entAmt; i++){ allPointsCount += entArray[allEnts[i].type].count; }
    allPointsCount += (rendAmt + entArray[player.type].count);

    allPoints = realloc(allPoints, allPointsCount * sizeof(worldTris));
}

static void generatePoints(int len){
    for (int i=0; i < len; i++){
        int x = randomInt(-10, 10);
        int y = 0;
        int z = randomInt(-10, 10);

        static3D[staticAmt++] = (staticPoints){
            .pos = {x, y, z},
            .joint = {{0, 0, 0}, {randomInt(1, 5), randomInt(1, 5), randomInt(1, 5)}},
            .color = 1
        };
    }
}

static int compareRenderTris(const void* a, const void* b) {
    float d1 = ((worldTris*)a)->dist;
    float d2 = ((worldTris*)b)->dist;
    return (d1 < d2) - (d1 > d2);
}

static int cLib_init() {
    setCam = createCamera(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 90.0f, 0.1f, 100.0f);
    cam = createCamera(10.0f, 3.0f, 41.0f, 0.0f, 0.0f, 0.0f, 90.0f, 0.1f, 100.0f);

    mapArray = realloc(mapArray, sizeof(Mesh_t) * maxMaps);
    objArray = realloc(objArray, sizeof(Mesh_t) * maxProjs);
    entArray = realloc(entArray, sizeof(PlayerModel_t) * maxEntStored);

    mapArray[0] = map;
    objArray[0] = disk;
    entArray[0] = testox;

    static3D = realloc(static3D, sizeof(staticPoints) * lengthJoints);
    allEnts = realloc(allEnts, sizeof(EntStruct) * MAX_ENTITIES);

    generateMap(mapArray[modelIndex].count);
    generatePoints(lengthJoints);

    int jointCount = 2;
    int* jointData = realloc(NULL, sizeof(int) * jointCount);
    for (int i = 0; i < jointCount; i++) { jointData[i] = 0; }
    player = createEntity(10.0f, 3.0f, 41.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.5f, 1.8f, 0.56f, 0.08f, 0, jointData, jointCount);

    return 0;
}

static int cLib_addEnt(Vect3f pos, Vect3f rot, Vect3f size, float radius, float height, float frict, float fallFrict, int type, ModelType objType){
    if (entAmt < MAX_ENTITIES){
        int jointCount = 2;
        int* jointData = realloc(NULL, sizeof(int) * jointCount);
        for (int i = 0; i < jointCount; i++) { jointData[i] = 0; }
        
        allEnts[entAmt].type = objType;

        switch(objType) {
            case ENTITY:
                allEnts[entAmt].data.ent = createEntity(pos.x, pos.y, pos.z, rot.x, rot.y, rot.z, size.x, size.y, size.z, radius, height, frict, fallFrict, type, jointData, jointCount);
                break;
            case OBJECT:
                allEnts[entAmt].data.obj = createObject(pos.x, pos.y, pos.z, rot.x, rot.y, rot.z, size.x, size.y, size.z, type, 1000);
                break;
        }
        entAmt++;

        int newPoint = (allPointsCount + entArray[type].count);
        if (newPoint > allPointsCount) {
            allPoints = realloc(allPoints, newPoint * sizeof(worldTris));
            allPointsCount = newPoint;
        }
    } else {
        printf("Max entities reached!\n");
    }

    return 0;
}

static void renderLines(float CamYDirSin, float CamYDirCos, float CamXDirSin, float CamXDirCos, float CamZDirSin, float CamZDirCos, Camera_t usedCam){
    int point[2][2];

    float fov = FROM_FIXED32(usedCam.fov);
    float nearPlane = FROM_FIXED32(usedCam.nearPlane);
    Vect3f camPos = {FROM_FIXED32(usedCam.position.x), FROM_FIXED32(usedCam.position.y), FROM_FIXED32(usedCam.position.z)};

    Vertex verts[2];
    float camMatrix[3][3];
    computeCamMatrix(camMatrix, CamYDirSin, CamYDirCos, CamXDirSin, CamXDirCos, CamZDirSin, CamZDirCos);

    for (int index=0; index < staticAmt; index++){
        int x = (int)static3D[index].pos.x;
        int y = (int)static3D[index].pos.y;
        int z = (int)static3D[index].pos.z;
        int color = (int)static3D[index].color;

        for (int i=0; i < 2; i++){
            verts[i].x = static3D[index].joint[i][0];
            verts[i].y = static3D[index].joint[i][1];
            verts[i].z = static3D[index].joint[i][2];
        }

        if (verts[0].z <= 0.0f && verts[1].z <= 0.0f) { continue; }

        for (int v = 0; v < 2; v++) {
            rotateVertexInPlace(&verts[v], camPos, camMatrix);
            project2D(&point[v][0], (float[]){verts[v].x, verts[v].y, verts[v].z}, fov, nearPlane);
        }

        staticLineDrawing(point[0], point[1], color);
    }
}

static void renderTris(float CamYDirSin, float CamYDirCos, float CamXDirSin, float CamXDirCos, float CamZDirSin, float CamZDirCos, Camera_t usedCam){
    float fov = FROM_FIXED32(usedCam.fov);
    float nearPlane = FROM_FIXED32(usedCam.nearPlane);
    float farPlane = FROM_FIXED32(usedCam.farPlane);
    Vect3f camPos = {FROM_FIXED32(usedCam.position.x), FROM_FIXED32(usedCam.position.y), FROM_FIXED32(usedCam.position.z)};

    int tri1[3][2];
    int tri2[3][2];
    int check[3][2];
    Vertex verts[3];
    clippedTri clip1, clip2;

    float camMatrix[3][3];
    computeCamMatrix(camMatrix, CamYDirSin, CamYDirCos, CamXDirSin, CamXDirCos, CamZDirSin, CamZDirCos);

    for (int index = 0; index < allAmt; index++){
        int color = allPoints[index].color;
        
        for (int v = 0; v < 3; v++) {
            verts[v].x = allPoints[index].verts[v][0];
            verts[v].y = allPoints[index].verts[v][1];
            verts[v].z = allPoints[index].verts[v][2];
            
            rotateVertexInPlace(&verts[v], camPos, camMatrix);
            project2D(&check[v][0], (float[]){verts[v].x, verts[v].y, verts[v].z}, fov, nearPlane);
        }

        if (allPoints[index].bfc == 1 && !windingOrder(check[0], check[1], check[2])) { continue; }
        if (verts[0].z <= 0.0f && verts[1].z <= 0.0f && verts[2].z <= 0.0f) { continue; }

        int output = TriangleClipping(verts, &clip1, &clip2, nearPlane, farPlane);
        if (output) {
            project2D(&tri1[0][0], (float[3]){clip1.t1.x, clip1.t1.y, clip1.t1.z}, fov, nearPlane);
            project2D(&tri1[1][0], (float[3]){clip1.t2.x, clip1.t2.y, clip1.t2.z}, fov, nearPlane);
            project2D(&tri1[2][0], (float[3]){clip1.t3.x, clip1.t3.y, clip1.t3.z}, fov, nearPlane);

            drawFilledTris(tri1, color);
            
            if (output == 2){
                project2D(&tri2[0][0], (float[3]){clip2.t1.x, clip2.t1.y, clip2.t1.z}, fov, nearPlane); 
                project2D(&tri2[1][0], (float[3]){clip2.t2.x, clip2.t2.y, clip2.t2.z}, fov, nearPlane);
                project2D(&tri2[2][0], (float[3]){clip2.t3.x, clip2.t3.y, clip2.t3.z}, fov, nearPlane);
                
                drawFilledTris(tri2, color);
            }
        }
    }
}

static void addObjectToWorld(Vect3f pos, Vect3f rot, Vect3f size, VectB Bone, Camera_t cCam, float depthOffset, int triCount, Vect3m* data, int* backFace, int* colorArray, int posDist) {
    worldTris tri;
    Vect3f rotatedBonePos;
    float rotMat[3][3];
    float boneMat[3][3];
    float finalMat[3][3];

    computeRotScaleMatrix(rotMat, rot.x, rot.y, rot.z, size.x, size.y, size.z);
    computeRotScaleMatrix(boneMat, degToRad(Bone.rot.x), degToRad(Bone.rot.y), degToRad(Bone.rot.z), size.x, size.y, size.z);
    rotateVertex(Bone.pos.x, Bone.pos.y, Bone.pos.z, rotMat, &rotatedBonePos.x);
    multiplyMatrix3x3(rotMat, boneMat, finalMat);

    float camX = FROM_FIXED32(cCam.position.x);
    float camY = FROM_FIXED32(cCam.position.y);
    float camZ = FROM_FIXED32(cCam.position.z);
    float renderRadiusSq = renderRadius ? (renderRadius * renderRadius) : 0.0f;
    const float one_third = 0.3333333f;

    Vect3f* transformedVerts = realloc(NULL, sizeof(Vect3f) * triCount * 3);
    
    for (int v = 0; v < triCount * 3; v++) {
        float r[3];
        rotateVertex(data[v].x, data[v].y, data[v].z, finalMat, r);
        transformedVerts[v].x = r[0] + (pos.x + rotatedBonePos.x);
        transformedVerts[v].y = r[1] + (pos.y + rotatedBonePos.y);
        transformedVerts[v].z = r[2] + (pos.z + rotatedBonePos.z);
    }

    for (int i = 0; i < triCount; i++) {
        float sumX = 0, sumY = 0, sumZ = 0;

        for (int j = 0; j < 3; j++) {
            Vect3f v = transformedVerts[i * 3 + j];
            tri.verts[j][0] = v.x;
            tri.verts[j][1] = v.y;
            tri.verts[j][2] = v.z;

            sumX += v.x;
            sumY += v.y;
            sumZ += v.z;
        }

        float cx = sumX * one_third;
        float cy = sumY * one_third;
        float cz = sumZ * one_third;

        float dx = cx - camX;
        float dy = cy - camY;
        float dz = cz - camZ;
        float dist = dx*dx + dy*dy + dz*dz;

        if (renderRadius && dist > renderRadiusSq) continue;
        if (dist <= 1e-8f) continue;

        tri.color = colorArray[i];
        tri.dist = dist - depthOffset;
        tri.bfc = backFace[i];
        allPoints[allAmt++] = tri;
    }

    transformedVerts = realloc(transformedVerts, sizeof(Vect3f) * 0);
}

static void addPlayer() {
    if (player.type < 0 || player.type >= maxEntStored) return;
    if (entArray[player.type].joints != player.jointCount) return;

    if (player.currentAnim != player.lastAnim) {
        player.frameCount = 0;
        for (int i=0; i < player.jointCount; i++){ player.currentFrame[i] = 0; }
    }

    for (int i=0; i < player.jointCount; i++){
        AnimMesh* anim = entArray[player.type].animations[i]->animations[player.currentAnim];
        if (player.frameCount >= anim->animOrientation[player.currentFrame[i]].frameSwap) { player.currentFrame[i]++; }
        if (player.currentFrame[i] >= anim->count-1) { player.currentFrame[i] = anim->count-1; }

        VectB bone = anim->animOrientation[player.currentFrame[i]];
        Vect3f objectPos = {
            FROM_FIXED32(player.position.x),
            FROM_FIXED32(player.position.y),
            FROM_FIXED32(player.position.z)
        };

        Vect3f objectRot = {
            FROM_FIXED32(player.rotation.x),
            FROM_FIXED32(player.rotation.y),
            FROM_FIXED32(player.rotation.z)
        };

        Vect3f objectSize = {
            FROM_FIXED32(player.size.x) + bone.size.x,
            FROM_FIXED32(player.size.y) + bone.size.y,
            FROM_FIXED32(player.size.z) + bone.size.z
        };

        addObjectToWorld(
            objectPos, objectRot, objectSize, bone,
            cam, 10.0f,
            anim->meshModel[bone.modelUsed]->count,
            anim->meshModel[bone.modelUsed]->data,
            anim->meshModel[bone.modelUsed]->bfc,
            anim->meshModel[bone.modelUsed]->color,
            true
        );
    }

    player.lastAnim = player.currentAnim;

    if (player.frameCount >= entArray[player.type].maxFrames[player.currentAnim] - 1) {
        player.frameCount = 0;
        for (int t = 0; t < player.jointCount; t++) {
            player.currentFrame[t] = 0;
        }
    } else {
        player.frameCount++;
    }

    for (int i=0; i < player.jointCount; i++){ TraceLog(LOG_INFO, "Limb: %d | Frame: %d", i, player.currentFrame[i]); }
    TraceLog(LOG_INFO, "FrameCount: %d | Animation: %d | Max Frames: %d", player.frameCount, player.currentAnim, entArray[player.type].maxFrames[player.currentAnim]);
}

static void addEntities() {
    for (int z = 0; z < entAmt; z++) {
        switch (allEnts[z].type) {
            case ENTITY:
                EntStruct *ent_ = &allEnts[z].data.ent;
                if (ent_->type < 0 || ent_->type >= maxEntStored) continue;
                if (entArray[ent_->type].joints != ent_->jointCount) continue;

                if (ent_->currentAnim != ent_->lastAnim) {
                    ent_->frameCount = 0;
                    for (int i=0; i < ent_->jointCount; i++) { ent_->currentFrame[i] = 0; }
                }

                for (int i=0; i < ent_->jointCount; i++){
                    AnimMesh* anim = entArray[ent_->type].animations[i]->animations[ent_->currentAnim];
                    if (ent_->frameCount == anim->animOrientation[ent_->currentFrame[i]].frameSwap) { ent_->currentFrame[i]++; }
                    if (ent_->currentFrame[i] >= anim->count-1) { ent_->currentFrame[i] = anim->count-1; }

                    VectB bone = anim->animOrientation[ent_->currentFrame[i]];
                    Vect3f objectPos = {
                        FROM_FIXED32(ent_->position.x) + bone.pos.x,
                        FROM_FIXED32(ent_->position.y) + bone.pos.y,
                        FROM_FIXED32(ent_->position.z) + bone.pos.z
                    };

                    Vect3f objectRot = {
                        FROM_FIXED32(ent_->rotation.x),
                        FROM_FIXED32(ent_->rotation.y),
                        FROM_FIXED32(ent_->rotation.z)
                    };

                    Vect3f objectSize = {
                        FROM_FIXED32(ent_->size.x) + bone.size.x,
                        FROM_FIXED32(ent_->size.y) + bone.size.y,
                        FROM_FIXED32(ent_->size.z) + bone.size.z
                    };

                    if (objectRot.x > degToRad(360.0f)) { objectRot.x -= degToRad(360.0f); } else if (objectRot.x < degToRad(0.0f)) { objectRot.x += degToRad(360.0f); }
                    if (objectRot.y > degToRad(360.0f)) { objectRot.y -= degToRad(360.0f); } else if (objectRot.y < degToRad(0.0f)) { objectRot.y += degToRad(360.0f); }
                    if (objectRot.z > degToRad(360.0f)) { objectRot.z -= degToRad(360.0f); } else if (objectRot.z < degToRad(0.0f)) { objectRot.z += degToRad(360.0f); }
                    
                    int frame = anim->animOrientation[ent_->currentFrame[i]].modelUsed;
                    addObjectToWorld(
                        objectPos, objectRot, objectSize, bone,
                        cam, 10.0f,
                        anim->meshModel[frame]->count,
                        anim->meshModel[frame]->data,
                        anim->meshModel[frame]->bfc,
                        anim->meshModel[frame]->color,
                        true
                    );
                }

                ent_->frameCount++;
                ent_->lastAnim = ent_->currentAnim;

                if (ent_->frameCount >= entArray[ent_->type].maxFrames[ent_->currentAnim]-1) {
                    ent_->frameCount = 0;
                    for (int t=0; t < ent_->jointCount; t++){ ent_->currentFrame[t] = 0; }
                }
                break;
            case OBJECT:
                ObjStruct *obj_ = &allEnts[z].data.obj;
                // objectTypes(obj_);

                int objIdx = obj_->type;
                VectB bone = {
                    .pos = {0.0f, 0.0f, 0.0f},
                    .rot = {0.0f, 0.0f, 0.0f},
                    .size = {1.0f, 1.0f, 1.0f},
                    .frameSwap = 0,
                    .modelUsed = 0
                };

                addObjectToWorld(
                    (Vect3f){FROM_FIXED32(obj_->position.x), FROM_FIXED32(obj_->position.y), FROM_FIXED32(obj_->position.z)},
                    (Vect3f){FROM_FIXED32(obj_->rotation.x), FROM_FIXED32(obj_->rotation.y), FROM_FIXED32(obj_->rotation.z)},
                    (Vect3f){FROM_FIXED32(obj_->size.x), FROM_FIXED32(obj_->size.y), FROM_FIXED32(obj_->size.z)},
                    bone,
                    cam, 0.0f,
                    objArray[objIdx].count,
                    objArray[objIdx].data,
                    objArray[objIdx].bfc,
                    objArray[objIdx].color,
                    false
                );
                break;
        }
    }
}

static void addMap(){
    VectB bone = {
        .pos = {0.0f, 0.0f, 0.0f},
        .rot = {0.0f, 0.0f, 0.0f},
        .size = {1.0f, 1.0f, 1.0f},
        .frameSwap = 0,
        .modelUsed = 0,
    };

    addObjectToWorld(
        (Vect3f){0.0f, 0.0f, 0.0f}, (Vect3f){0.0f, 0.0f, 0.0f}, (Vect3f){1.0f, 1.0f, 1.0f}, bone,
        cam, 0.0f,
        mapArray[modelIndex].count,
        mapArray[modelIndex].data,
        mapArray[modelIndex].bfc,
        mapArray[modelIndex].color,
        false
    );
}

static int cLib_render() {
    const float CamXDirSin = -sinf(FROM_FIXED32(cam.rotation.x));
    const float CamXDirCos = cosf(FROM_FIXED32(cam.rotation.x));
    const float CamYDirSin = -sinf(FROM_FIXED32(cam.rotation.y));
    const float CamYDirCos = cosf(FROM_FIXED32(cam.rotation.y));
    const float CamZDirSin = -sinf(FROM_FIXED32(cam.rotation.z));
    const float CamZDirCos = cosf(FROM_FIXED32(cam.rotation.z));

    addMap();
    addEntities();
    addPlayer();
    qsort(allPoints, allAmt, sizeof(worldTris), compareRenderTris);
    
    renderTris(CamYDirSin, CamYDirCos, CamXDirSin, CamXDirCos, CamZDirSin, CamZDirCos, cam);
    renderLines(CamYDirSin, CamYDirCos, CamXDirSin, CamXDirCos, CamZDirSin, CamZDirCos, cam);

    return 0;
}

static int cLib_playerAction() {
    for (int i=0; i < (sW * sH); i++){ zBuffer[i] = FLT_MAX16; }
    allAmt = 0;

    movePlayerObj(&player, &cam, colRend);
    handleCameraInput(&cam);
    updateCamera(&cam, &player, 7.0f);

    // for(int i=0; i < entAmt; i++){
    //     switch (allEnts[i].type) {
    //         case ENTITY:
    //             EntStruct *ent_ = &allEnts[i].data.ent;
    //             moveEntObj(ent_, &player);
    //             break;
    //         case OBJECT:
    //             break;    
    //     }
    // }

    camForward.x = cosf(FROM_FIXED32(cam.rotation.x)) * sinf(FROM_FIXED32(cam.rotation.y));
    camForward.y = sinf(FROM_FIXED32(cam.rotation.x));
    camForward.z = cosf(FROM_FIXED32(cam.rotation.x)) * cosf(FROM_FIXED32(cam.rotation.y));

    return 0;
}

static int cLib_SettingsChange(){
    renderRadius = 85;

    camForward.x = cosf(FROM_FIXED32(setCam.rotation.x)) * sinf(FROM_FIXED32(setCam.rotation.y));
    camForward.y = sinf(FROM_FIXED32(setCam.rotation.x));
    camForward.z = cosf(FROM_FIXED32(setCam.rotation.x)) * cosf(FROM_FIXED32(setCam.rotation.y));

    return 0;
}

int main(){
    InitWindow(sW, sH, "CrossUp");
    SetTargetFPS(30);

    gameScreen = 1;

    cLib_init();
    cLib_addEnt((Vect3f){0.0, 4.0, 0.0}, (Vect3f){0.0, 0.0, 0.0}, (Vect3f){1.0, 1.0, 1.0}, 0.5, 1.8, 0.56, 0.08, 0, ENTITY);
    cLib_addEnt((Vect3f){0.0, 4.0, -10.0}, (Vect3f){0.0, 0.0, 0.0}, (Vect3f){1.0, 1.0, 1.0}, 0.5, 1.8, 0.56, 0.08, 0, OBJECT);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        if (gameScreen == 1) {
            cLib_playerAction();
            cLib_render();
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}