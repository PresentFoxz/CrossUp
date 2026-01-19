#include "library.h"
#include "entities.h"
#include "_3DMath.h"
#include "movement.h"
#include "collisions.h"
#include "mesh.h"
#include "meshConvert.h"

#include "Objects/animData.h"
#include "textures/allTexts.h"

Camera_t cam;
worldTris* allPoints;
staticPoints* static3D;
worldTris* entModels;

int entAmt = 0;
int allAmt = 0;
int staticAmt = 0;

const int colRend = 0;
int renderRadius = 85;

textAtlas* textAtlasMem;

Objects* allEnts;
EntStruct player;
#define MAX_ENTITIES 240

Mesh_t mapArray;
int mapIndex = 0;
const int mapObjsCount = 2;

Mesh_t* objArray;
const int maxProjs = 1;

VertAnims* entArray;
int entIndex = 0;

int allPointsCount = 0;
const int lengthJoints = 3;

Vect3f rotModelSet;
Vect3f camForward;
int gameScreen = 0;

static int addEnt(Vect3f pos, Vect3f rot, Vect3f size, float radius, float height, float frict, float fallFrict, int type, ModelType objType){
    if (entAmt < MAX_ENTITIES){
        allEnts[entAmt].type = objType;

        switch(objType) {
            case ENTITY:
                allEnts[entAmt].data.ent = createEntity(pos.x, pos.y, pos.z, rot.x, rot.y, rot.z, size.x, size.y, size.z, radius, height, frict, fallFrict, type);
                break;
            case OBJECT:
                allEnts[entAmt].data.obj = createObject(pos.x, pos.y, pos.z, rot.x, rot.y, rot.z, size.x, size.y, size.z, type, 1000);
                break;
        }
        entAmt++;

        if (objType != OBJECT) {
            int newPoint = (allPointsCount + entArray[type].count);
            if (newPoint > allPointsCount) { allPointsCount = (newPoint + 50); }
        }
    } else {
        printf("Max entities reached!\n");
    }

    return 0;
}

static void generateMap() {
    resetCollisionSurface();
    for (int i=0; i < mapArray.count; i++){
        int idx[3] = {(i*3), (i*3)+1, (i*3)+2};
        addCollisionSurface(
            (Vect3f){mapArray.data[idx[0]].x, mapArray.data[idx[0]].y, mapArray.data[idx[0]].z},
            (Vect3f){mapArray.data[idx[1]].x, mapArray.data[idx[1]].y, mapArray.data[idx[1]].z},
            (Vect3f){mapArray.data[idx[2]].x, mapArray.data[idx[2]].y, mapArray.data[idx[2]].z},
            SURFACE_NONE
        );
    }

    allPointsCount += mapArray.count;
    for (int i=0; i < maxProjs; i++) { allPointsCount += objArray[i].count; }
}

static void generateEnts() {
    if (mapIndex == 0) {
        addEnt((Vect3f){0.0f, 20.0f, -5.0f}, (Vect3f){0.0f, 0.0f, 0.0f}, (Vect3f){0.5f, 0.5f, 0.5f}, 0.5f, 1.8f, 0.56f, 0.08f, 0, ENTITY);
        addEnt((Vect3f){0.0f, 5.0f, 5.0f}, (Vect3f){0.0f, 0.0f, 0.0f}, (Vect3f){0.5f, 0.5f, 0.5f}, 0.5f, 1.8f, 0.56f, 0.08f, 0, OBJECT);
    } else if (mapIndex == 1) {
        addEnt((Vect3f){0.0f, 5.0f, 5.0f}, (Vect3f){0.0f, 0.0f, 0.0f}, (Vect3f){0.5f, 0.5f, 0.5f}, 0.5f, 1.8f, 0.56f, 0.08f, 0, OBJECT);
    }
}

static void generateTextures() {
    textAtlasMem = realloc(textAtlasMem, sizeof(textAtlas) * 1);
    textAtlasMem[0] = testTexture;
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
    allPointsCount = 0;
    entIndex = 0;
    entAmt = 0;
    allAmt = 0;
    staticAmt = 0;

    cam = createCamera(10.0f, 3.0f, 41.0f, 0.0f, 0.0f, 0.0f, 90.0f, 0.1f, 100.0f);
    player = createEntity(0.0f, 20.0f, -5.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.5f, 1.8f, 0.56f, 0.08f, 0);

    allPoints = malloc(sizeof(worldTris) * 0);
    objArray = malloc( sizeof(Mesh_t) * maxProjs);
    entArray = malloc(sizeof(VertAnims) * entDataCount);
    static3D = malloc(sizeof(staticPoints) * lengthJoints);
    allEnts = malloc(sizeof(EntStruct) * MAX_ENTITIES);

    convertFileToMesh(mapObjs[mapIndex], &mapArray, mapData[mapIndex][0], mapData[mapIndex][1]);
    convertFileToMesh("Objects/proj/ball.obj", &objArray[0], 0, 0);

    for (int i=0; i < entDataCount; i++){
        int highest = allocAnimModel(&entArray[i], entData[i].totalAnims, entData[i].animFrameCounts, entData[i].animNames);
        allPointsCount += (highest * (entAmt+1));
        entArray[i].count = highest;
    }

    generateMap();
    generatePoints(lengthJoints);
    generateTextures();

    generateEnts();

    allPoints = realloc(allPoints, allPointsCount * sizeof(worldTris));

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

            verts[v].u = allPoints[index].uvs[v][0];
            verts[v].v = allPoints[index].uvs[v][1];
            
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

            drawTexturedTris(tri1, (float[3][2]){ {clip1.t1.u, clip1.t1.v}, {clip1.t2.u, clip1.t2.v}, {clip1.t3.u, clip1.t3.v} }, textAtlasMem[0].pixels, textAtlasMem[0].w, textAtlasMem[0].h );
            // drawFilledTris(tri1, color);
            if (allPoints[index].lines == 1) drawTriLines(tri1);
            
            if (output == 2){
                project2D(&tri2[0][0], (float[3]){clip2.t1.x, clip2.t1.y, clip2.t1.z}, fov, nearPlane); 
                project2D(&tri2[1][0], (float[3]){clip2.t2.x, clip2.t2.y, clip2.t2.z}, fov, nearPlane);
                project2D(&tri2[2][0], (float[3]){clip2.t3.x, clip2.t3.y, clip2.t3.z}, fov, nearPlane);
                
                drawTexturedTris(tri2, (float[3][2]){ {clip2.t1.u, clip2.t1.v}, {clip2.t2.u, clip2.t2.v}, {clip2.t3.u, clip2.t3.v} }, textAtlasMem[0].pixels, textAtlasMem[0].w, textAtlasMem[0].h );
                // drawFilledTris(tri2, color);
                if (allPoints[index].lines == 1) drawTriLines(tri2);
            }
        }
    }
}

static void addObjectToWorld(Vect3f pos, Vect3f rot, Vect3f size, Camera_t cCam, float depthOffset, int triCount, Vect3f* data, int* backFace, int* colorArray, int lineDraw, int distMod) {
    worldTris tri;
    float rotMat[3][3];
    computeRotScaleMatrix(rotMat, rot.x, rot.y, rot.z, size.x, size.y, size.z);

    float camX = FROM_FIXED32(cCam.position.x);
    float camY = FROM_FIXED32(cCam.position.y);
    float camZ = FROM_FIXED32(cCam.position.z);
    float renderRadiusSq = renderRadius ? (renderRadius * renderRadius) : 0.0f;
    const float one_third = 0.3333333f;

    Vect3f* transformedVerts = realloc(NULL, sizeof(Vect3f) * triCount * 3);

    for (int v = 0; v < triCount * 3; v++) {
        float r[3];
        rotateVertex(data[v].x, data[v].y, data[v].z, rotMat, r);
        transformedVerts[v].x = r[0] + pos.x;
        transformedVerts[v].y = r[1] + pos.y;
        transformedVerts[v].z = r[2] + pos.z;
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

        if (distMod == 1) {
            if (renderRadius && dist > renderRadiusSq) continue;
            if (dist <= 1e-8f) continue;
        }

        tri.color = colorArray[i];
        tri.dist = dist - depthOffset;
        tri.bfc = backFace[i];
        tri.lines = lineDraw;

        tri.uvs[0][0] = 0.0f; tri.uvs[0][1] = 0.0f;
        tri.uvs[1][0] = 1.0f; tri.uvs[1][1] = 0.0f;
        tri.uvs[2][0] = 0.0f; tri.uvs[2][1] = 1.0f;

        allPoints[allAmt++] = tri;
    }

    transformedVerts = realloc(transformedVerts, sizeof(Vect3f) * 0);
}

static void addPlayer() {
    if (player.type < 0 || player.type >= entDataCount) return;

    if (player.currentAnim != player.lastAnim) {
        player.frameCount = 0;
        player.currentFrame = 0;
    }

    AnimFrames* anims = entArray[player.type].anims[player.currentAnim];
    int newFrame = anims->frames;

    if (player.currentFrame >= newFrame) {
        player.frameCount = 0;
        player.currentFrame = 0;
    }

    Mesh_t model = anims->meshModel[player.currentFrame];
    if (model.data != NULL && model.count > 0 && model.bfc != NULL) {
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
            FROM_FIXED32(player.size.x),
            FROM_FIXED32(player.size.y),
            FROM_FIXED32(player.size.z)
        };
    
        addObjectToWorld(
            objectPos, objectRot, objectSize,
            cam, 10.0f,
            model.count,
            model.data,
            model.bfc,
            model.color,
            true, 1
        );
    }

    player.lastAnim = player.currentAnim;
    player.frameCount++;
    if (player.frameCount > 4) {
        player.currentFrame++;
        player.frameCount = 0;
    }

    // TraceLog(LOG_INFO, "FrameCount: %d | Animation: %d | Current Frame: %d", player.frameCount, player.currentAnim, player.currentFrame);
}

static void addEntities() {
    for (int z = 0; z < entAmt; z++) {
        switch (allEnts[z].type) {
            case ENTITY:
                EntStruct *ent_ = &allEnts[z].data.ent;
                if (ent_->type < 0 || ent_->type >= entDataCount) continue;

                // if (ent_->currentAnim != ent_->lastAnim) {
                //     ent_->frameCount = 0;
                //     ent_->currentFrame = 0;
                // }
            
                AnimFrames* anims = entArray[ent_->type].anims[ent_->currentAnim];
                int newFrame = anims->frames;
            
                // if (ent_->currentFrame >= newFrame) {
                //     ent_->frameCount = 0;
                //     ent_->currentFrame = 0;
                // }
            
                Mesh_t model = anims->meshModel[ent_->currentFrame];
                if (model.data != NULL && model.count > 0 && model.bfc != NULL) {
                    Vect3f objectPos = {
                        FROM_FIXED32(ent_->position.x),
                        FROM_FIXED32(ent_->position.y),
                        FROM_FIXED32(ent_->position.z)
                    };

                    Vect3f objectRot = {
                        FROM_FIXED32(ent_->rotation.x),
                        FROM_FIXED32(ent_->rotation.y),
                        FROM_FIXED32(ent_->rotation.z)
                    };

                    Vect3f objectSize = {
                        FROM_FIXED32(ent_->size.x),
                        FROM_FIXED32(ent_->size.y),
                        FROM_FIXED32(ent_->size.z)
                    };

                    addObjectToWorld(
                        objectPos, objectRot, objectSize,
                        cam, 10.0f,
                        model.count,
                        model.data,
                        model.bfc,
                        model.color,
                        true, 1
                    );
                }

                // ent_->lastAnim = ent_->currentAnim;
                // ent_->frameCount++;
                // if (ent_->frameCount > 4) {
                //     ent_->currentFrame++;
                //     ent_->frameCount = 0;
                // }

                break;
            case OBJECT:
                ObjStruct *obj_ = &allEnts[z].data.obj;
                // objectTypes(obj_);
                
                addObjectToWorld(
                    (Vect3f){FROM_FIXED32(obj_->position.x), FROM_FIXED32(obj_->position.y), FROM_FIXED32(obj_->position.z)},
                    (Vect3f){FROM_FIXED32(obj_->rotation.x), FROM_FIXED32(obj_->rotation.y), FROM_FIXED32(obj_->rotation.z)},
                    (Vect3f){FROM_FIXED32(obj_->size.x), FROM_FIXED32(obj_->size.y), FROM_FIXED32(obj_->size.z)},
                    cam, 0.0f,
                    objArray[obj_->type].count,
                    objArray[obj_->type].data,
                    objArray[obj_->type].bfc,
                    objArray[obj_->type].color,
                    false, 1
                );
                break;
        }
    }
}

static void addMap(){
    addObjectToWorld(
        (Vect3f){0.0f, 0.0f, 0.0f}, (Vect3f){0.0f, 0.0f, 0.0f}, (Vect3f){1.0f, 1.0f, 1.0f},
        cam, 0.0f,
        mapArray.count,
        mapArray.data,
        mapArray.bfc,
        mapArray.color,
        false, 1
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
    allAmt = 0;

    movePlayerObj(&player, &cam);
    handleCameraInput(&cam);
    updateCamera(&cam, &player, 7.0f);

    for(int i=0; i < entAmt; i++){
        switch (allEnts[i].type) {
            case ENTITY:
                EntStruct *ent_ = &allEnts[i].data.ent;
                moveEntObj(ent_, &player);
                break;
            case OBJECT:
                break;    
        }
    }

    camForward.x = cosf(FROM_FIXED32(cam.rotation.x)) * sinf(FROM_FIXED32(cam.rotation.y));
    camForward.y = sinf(FROM_FIXED32(cam.rotation.x));
    camForward.z = cosf(FROM_FIXED32(cam.rotation.x)) * cosf(FROM_FIXED32(cam.rotation.y));

    return 0;
}

int main(){
    InitWindow(sW, sH, "CrossUp");
    SetTargetFPS(30);

    gameScreen = 1;

    cLib_init();

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        if (gameScreen == 1) {
            cLib_playerAction();
            cLib_render();

            if (IsKeyPressed(KEY_M)) {
                mapIndex++;
                if (mapIndex >= mapObjsCount) mapIndex = 0;
                cLib_init();
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}