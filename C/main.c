#include "library.h"
#include "entities.h"
#include "draw.h"
#include "movement.h"
#include "collisions.h"
#include "Objects/allMeshes.h"

PlaydateAPI* pd;
Camera cam;
worldTris* allPoints;
staticPoints* static3D;
worldTris* entModels;
int gameStart = 1;

uint8_t* buf = NULL;
qfixed16_t zBuffer[sW * sH];

int rendAmt = 0;
int entAmt = 0;
int allAmt = 0;
int staticAmt = 0;

const int debug = 0;
const int colRend = 0;
int renderRadius = 85;

EntStruct* allEnts;
EntStruct player;
const int maxEntities = 200;

Mesh* mapArray;
Mesh* entArray;
const int maxMaps = 1;
const int maxEntStored = 2;

int modelIndex = 0;
int allPointsCount = 0;
const int lengthJoints = 3;

Vect3f rotModelSet;

Vect3f camForward;
Camera setCam;

static int cLib_init(lua_State* L);
static int cLib_render(lua_State* L);
static int cLib_playerAction(lua_State* L);
static int cLib_addEnt(lua_State* L);
static int cLib_SettingsChange(lua_State* L);
static int cLib_settingsModel(lua_State* L);

static const lua_reg cLibs[] ={
    { "init", cLib_init },
    { "render", cLib_render },
    { "player", cLib_playerAction },
    { "addEnt", cLib_addEnt },
    { "newSettings", cLib_SettingsChange },
    { "drawObject", cLib_settingsModel },
    { NULL, NULL }
};

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg) {
    if (event == kEventInitLua) {
        pd = playdate;

        const char* err;
        if ( !pd->lua->registerClass("cLib", cLibs, NULL, 0, &err)) { pd->system->logToConsole("%s:%i: registerClass failed, %s", __FILE__, __LINE__, err); } else { pd->system->logToConsole("Loaded cLibs!"); }
    }
    return 0;
}

static void generateMap(int len){
    allPoints = pd->system->realloc(allPoints, sizeof(worldTris) * 0);
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

    allPoints = pd->system->realloc(allPoints, allPointsCount * sizeof(worldTris));
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

static int cLib_init(lua_State* L) {
    // int s = pd->lua->getArgInt(2);
    
    setCam = createCamera(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 90.0f, 0.1f, 100.0f);
    cam = createCamera(10.0f, 3.0f, 41.0f, 0.0f, 0.0f, 0.0f, 90.0f, 0.1f, 100.0f);

    mapArray = pd->system->realloc(mapArray, sizeof(Mesh) * maxMaps);
    entArray = pd->system->realloc(entArray, sizeof(Mesh) * maxEntStored);

    mapArray[0] = map;
    entArray[0] = cube;
    entArray[1] = arm;

    static3D = pd->system->realloc(static3D, sizeof(staticPoints) * lengthJoints);
    allEnts = pd->system->realloc(allEnts, sizeof(EntStruct) * entAmt);

    generateMap(mapArray[modelIndex].count);
    generatePoints(lengthJoints);

    int jointCount = 2;
    int* jointData = pd->system->realloc(NULL, sizeof(int) * jointCount);
    for (int i = 0; i < jointCount; i++) { jointData[i] = 0; }
    player = createPlayer(10.0f, 3.0f, 41.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.5f, 1.8f, 0.56f, 0.08f, 0, jointData, jointCount);

    return 0;
}

static int cLib_addEnt(lua_State* L){
    float xPos = pd->lua->getArgFloat(2);
    float yPos = pd->lua->getArgFloat(3);
    float zPos = pd->lua->getArgFloat(4);

    float xRot = pd->lua->getArgFloat(5);
    float yRot = pd->lua->getArgFloat(6);
    float zRot = pd->lua->getArgFloat(7);

    float xSize = pd->lua->getArgFloat(8);
    float ySize = pd->lua->getArgFloat(9);
    float zSize = pd->lua->getArgFloat(10);

    float radius = pd->lua->getArgFloat(11);
    float height = pd->lua->getArgFloat(12);

    float frict = pd->lua->getArgFloat(13);
    float fallFrict = pd->lua->getArgFloat(14);

    int type = pd->lua->getArgInt(12);

    if (entAmt < maxEntities){
        int jointCount = 2;
        int* jointData = pd->system->realloc(NULL, sizeof(int) * jointCount);
        for (int i = 0; i < jointCount; i++) { jointData[i] = 0; }

        allEnts = pd->system->realloc(allEnts, sizeof(EntStruct) * (entAmt + 1));
        allEnts[entAmt] = createEntity(xPos, yPos, zPos, xRot, yRot, zRot, xSize, ySize, zSize, radius, height, frict, fallFrict, type, jointData, jointData);
        entAmt++;

        int newPoint = (allPointsCount + entArray[type].count);
        if (newPoint > allPointsCount) {
            allPoints = pd->system->realloc(allPoints, newPoint * sizeof(worldTris));
            allPointsCount = newPoint;
        }
    } else {
        pd->system->logToConsole("Max entities reached!");
    }

    return 0;
}

static void renderCollision(float pCol[substeps][3], float CamYDirSin, float CamYDirCos, float CamXDirSin, float CamXDirCos, float CamZDirSin, float CamZDirCos, Camera usedCam){
    int size = 10;
    int half = size / 2;

    float fov = FROM_FIXED32(usedCam.fov);
    float nearPlane = FROM_FIXED32(usedCam.nearPlane);
    Vect3f camPos = {FROM_FIXED32(usedCam.position.x), FROM_FIXED32(usedCam.position.y), FROM_FIXED32(usedCam.position.z)};

    Vertex verts;
    int point[2];
    float heightMod[2] = {0.0f, FROM_FIXED32(player.height)};

    float camMatrix[3][3];
    computeCamMatrix(camMatrix, CamYDirSin, CamYDirCos, CamXDirSin, CamXDirCos, CamZDirSin, CamZDirCos);

    for (int z=0; z < substeps; z++){
        verts.x = pCol[z][0];
        verts.y = pCol[z][1];
        verts.z = pCol[z][2];

        for (int i = 0; i < 2; i++) {
            Vertex vertsEdit = {verts.x, (verts.y + heightMod[i]), verts.z};
            rotateVertexInPlace(&vertsEdit, camPos, camMatrix);
            project2D(&point[0], (float[]){vertsEdit.x, vertsEdit.y, vertsEdit.z}, fov, nearPlane);
    
            pd->graphics->fillRect((int)(point[0] - (half+1)), (int)(point[1] - (half+1)), size+2, size+2, kColorBlack);
            pd->graphics->fillRect((int)(point[0] - half), (int)(point[1] - half), size, size, kColorWhite);
        }
    }
}

static void renderLines(float CamYDirSin, float CamYDirCos, float CamXDirSin, float CamXDirCos, float CamZDirSin, float CamZDirCos, Camera usedCam){
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

static void renderTris(float CamYDirSin, float CamYDirCos, float CamXDirSin, float CamXDirCos, float CamZDirSin, float CamZDirCos, Camera usedCam){
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
            
            drawFilledTrisZ(tri1, clip1, color, zBuffer);
            // drawFilledTrisNoZ(tri1, color);
            
            if (output == 2){
                project2D(&tri2[0][0], (float[3]){clip2.t1.x, clip2.t1.y, clip2.t1.z}, fov, nearPlane); 
                project2D(&tri2[1][0], (float[3]){clip2.t2.x, clip2.t2.y, clip2.t2.z}, fov, nearPlane);
                project2D(&tri2[2][0], (float[3]){clip2.t3.x, clip2.t3.y, clip2.t3.z}, fov, nearPlane);
                
                drawFilledTrisZ(tri2, clip2, color, zBuffer);
                // drawFilledTrisNoZ(tri2, color);
            }
        }
    }
}

static void addObjectToWorld(Vect3f pos, Vect3f rot, Vect3f size, Camera cCam, int type, int triCount, Vect3m* data, int* backFace, int* colorArray, int posDist) {
    // Vect3f dirNorm;
    worldTris tri;
    float rotMat[3][3];
    computeRotScaleMatrix(rotMat, rot.x, rot.y, rot.z, size.x, size.y, size.z);

    float camX = FROM_FIXED32(cCam.position.x);
    float camY = FROM_FIXED32(cCam.position.y);
    float camZ = FROM_FIXED32(cCam.position.z);
    float renderRadiusSq = renderRadius ? (renderRadius * renderRadius) : 0.0f;
    const float one_third = 0.3333333f;

    Vect3f* transformedVerts = pd->system->realloc(NULL, sizeof(Vect3f) * triCount * 3);
    
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

        if (renderRadius && dist > renderRadiusSq) continue;
        if (dist <= 1e-8f) continue;

        tri.color = colorArray[i];
        tri.dist = dist;
        tri.bfc = backFace[i];
        allPoints[allAmt++] = tri;

        // float invLen = fastInvSqrt(dist);
        // dirNorm.x = dx * invLen;
        // dirNorm.y = dy * invLen;
        // dirNorm.z = dz * invLen;

        // float d = dirNorm.x * camForward.x + dirNorm.y * camForward.y + dirNorm.z * camForward.z;
        // if (d > -0.867f) {
        //     tri.color = colorArray[i];
        //     tri.dist = dist;
        //     allPoints[allAmt++] = tri;
        // }
    }

    transformedVerts = pd->system->realloc(transformedVerts, sizeof(Vect3f) * 0);
}

static void addPlayer() {
    if (player.type < 0 || player.type >= maxEntStored) return;
    if (testox.joints != player.jointCount) return;

    if (player.currentAnim != player.lastAnim) { 
        for (int i=0; i < player.jointCount; i++) { player.frameCount[i] = 0; player.currentFrame[i] = 0; }
    }

    for (int i=0; i < player.jointCount; i++){
        AnimMesh* anim = testox.animations[i]->animations[player.currentAnim];
        int frameCount = anim->animOrientation[player.currentFrame[i]].frameCount;

        player.frameCount[i]++;
        if (player.frameCount[i] >= frameCount) {
            player.frameCount[i] = 0;
            player.currentFrame[i]++;
        }

        if (player.currentFrame[i] >= testox.maxFrames[player.currentAnim][i]) { player.frameCount[i] = 0; player.currentFrame[i] = 0; }

        VectB bone = anim->animOrientation[player.currentFrame[i]];
        Vect3f objectPos = {
            FROM_FIXED32(player.position.x) + bone.pos.x,
            FROM_FIXED32(player.position.y) + bone.pos.y,
            FROM_FIXED32(player.position.z) + bone.pos.z
        };

        Vect3f objectSize = {
            FROM_FIXED32(player.size.x) + bone.size.x,
            FROM_FIXED32(player.size.y) + bone.size.y,
            FROM_FIXED32(player.size.z) + bone.size.z
        };
        
        addObjectToWorld(
            objectPos,
            (Vect3f){FROM_FIXED32(player.rotation.x), FROM_FIXED32(player.rotation.y), FROM_FIXED32(player.rotation.z)},
            objectSize,
            cam,
            i,
            anim->meshModel[anim->animOrientation->modelUsed]->count,
            anim->meshModel[anim->animOrientation->modelUsed]->data,
            anim->meshModel[anim->animOrientation->modelUsed]->bfc,
            anim->meshModel[anim->animOrientation->modelUsed]->color,
            true
        );
    }

    player.lastAnim = player.currentAnim;

    pd->system->logToConsole("Animation: %d", player.currentAnim);
}

static void addTestModel(Vect3f pos, Vect3f rot, Vect3f size, int type) {
    addObjectToWorld(
        pos,
        rot,
        size,
        cam,
        type,
        entArray[type].count,
        entArray[type].data,
        entArray[type].bfc,
        entArray[type].color,
        true
    );
}

static void addEntities() {
    for (int i = 0; i < entAmt; i++) {
        if (allEnts[i].type < 0 || allEnts[i].type >= maxEntStored) continue;
        addObjectToWorld(
            (Vect3f){FROM_FIXED32(allEnts[i].position.x), FROM_FIXED32(allEnts[i].position.y), FROM_FIXED32(allEnts[i].position.z)},
            (Vect3f){FROM_FIXED32(allEnts[i].rotation.x), FROM_FIXED32(allEnts[i].rotation.y), FROM_FIXED32(allEnts[i].rotation.z)},
            (Vect3f){FROM_FIXED32(allEnts[i].size.x), FROM_FIXED32(allEnts[i].size.y), FROM_FIXED32(allEnts[i].size.z)},
            cam,
            allEnts[i].type,
            entArray[allEnts[i].type].count,
            entArray[allEnts[i].type].data,
            entArray[allEnts[i].type].bfc,
            entArray[allEnts[i].type].color,
            false
        );
    }
}

static void addMap(){
    addObjectToWorld(
        (Vect3f){0.0f, 0.0f, 0.0f},
        (Vect3f){0.0f, 0.0f, 0.0f},
        (Vect3f){1.0f, 1.0f, 1.0f},
        cam,
        modelIndex,
        mapArray[modelIndex].count,
        mapArray[modelIndex].data,
        mapArray[modelIndex].bfc,
        mapArray[modelIndex].color,
        false
    );
}

static int cLib_render(lua_State* L) {
    const float CamXDirSin = -sinf(FROM_FIXED32(cam.rotation.x));
    const float CamXDirCos = cosf(FROM_FIXED32(cam.rotation.x));
    const float CamYDirSin = -sinf(FROM_FIXED32(cam.rotation.y));
    const float CamYDirCos = cosf(FROM_FIXED32(cam.rotation.y));
    const float CamZDirSin = -sinf(FROM_FIXED32(cam.rotation.z));
    const float CamZDirCos = cosf(FROM_FIXED32(cam.rotation.z));

    addMap();
    addEntities();
    addPlayer();
    // addTestModel((Vect3f){0.0f, 3.0f, 0.0f}, (Vect3f){0.0f, 0.0f, 0.0f}, (Vect3f){1.0f, 1.0f, 1.0f}, 2);
    // qsort(allPoints, allAmt, sizeof(worldTris), compareRenderTris);
    
    renderTris(CamYDirSin, CamYDirCos, CamXDirSin, CamXDirCos, CamZDirSin, CamZDirCos, cam);
    renderLines(CamYDirSin, CamYDirCos, CamXDirSin, CamXDirCos, CamZDirSin, CamZDirCos, cam);

    if (colRend){ renderCollision(pColPoints, CamYDirSin, CamYDirCos, CamXDirSin, CamXDirCos, CamZDirSin, CamZDirCos, cam); }

    // pd->system->logToConsole( "State: %d", player.state );
    // pd->system->logToConsole( "Velocity: [ %f | %f | %f ]", meterToUnit(player.velocity.x), meterToUnit(player.velocity.y), meterToUnit(player.velocity.z) );
    
    if (debug == 1) {
        // pd->system->logToConsole( "All allocation: %d", (sizeof(worldTris) * mapArray[modelIndex].count) + (sizeof(EntStruct) * entAmt) + (sizeof(worldTris) * allAmt) + (sizeof(staticPoints) * lengthJoints) );
        // pd->system->logToConsole( "Map Array: %d", (sizeof(worldTris) * mapArray[modelIndex].count) );
        // pd->system->logToConsole( "All Points: %d", (sizeof(worldTris) * allAmt) );
        // pd->system->logToConsole( "Static Points: %d", (sizeof(staticPoints) * lengthJoints) );

        char text[64];
        snprintf(text, sizeof(text), "Player: %d %d %d", (int)(FROM_FIXED32(player.position.x)), (int)(FROM_FIXED32(player.position.y)), (int)(FROM_FIXED32(player.position.z)));
        pd->graphics->fillRect(0, 18, sizeof(text) * 2, 44, kColorWhite);
        pd->graphics->drawText(text, strlen(text), kASCIIEncoding, 2, 20);
        snprintf(text, sizeof(text), "Camera: %d %d %d", (int)(FROM_FIXED32(cam.position.x)), (int)(FROM_FIXED32(cam.position.y)), (int)(FROM_FIXED32(cam.position.z)));
        pd->graphics->drawText(text, strlen(text), kASCIIEncoding, 2, 42);
    }

    return 0;
}

static int cLib_playerAction(lua_State* L) {
    pd->graphics->clear(kColorBlack);
    buf = pd->graphics->getFrame();
    for (int i=0; i < (sW * sH); i++){ zBuffer[i] = FLT_MAX16; }
    allAmt = 0;

    movePlayerObj(&player, &cam, colRend);
    handleCameraInput(&cam);
    updateCamera(&cam, &player, 7.0f);

    for(int i=0; i < entAmt; i++){ moveEntObj(&allEnts[i], &player); }

    camForward.x = cosf(FROM_FIXED32(cam.rotation.x)) * sinf(FROM_FIXED32(cam.rotation.y));
    camForward.y = sinf(FROM_FIXED32(cam.rotation.x));
    camForward.z = cosf(FROM_FIXED32(cam.rotation.x)) * cosf(FROM_FIXED32(cam.rotation.y));

    return 0;
}

static void addModel(int type){
    addObjectToWorld(
        (Vect3f){5.0f, 0.0f, 5.0f},
        (Vect3f){rotModelSet.x, rotModelSet.y, rotModelSet.z},
        (Vect3f){1.0f, 1.0f, 1.0f},
        setCam,
        type,
        entArray[type].count,
        entArray[type].data,
        entArray[type].bfc,
        entArray[type].color,
        true
    );

    rotModelSet.x += degToRad(randomFloat(0.5f, 2.0f));
    rotModelSet.y += degToRad(randomFloat(0.5f, 2.0f));
    rotModelSet.z += degToRad(randomFloat(0.5f, 2.0f));

    if (rotModelSet.x > degToRad(360.0f)) rotModelSet.x = degToRad(0.0f);
    if (rotModelSet.x < degToRad(0.0f)) rotModelSet.x = degToRad(360.0f);

    if (rotModelSet.y > degToRad(360.0f)) rotModelSet.y = degToRad(0.0f);
    if (rotModelSet.y < degToRad(0.0f)) rotModelSet.y = degToRad(360.0f);

    if (rotModelSet.z > degToRad(360.0f)) rotModelSet.z = degToRad(0.0f);
    if (rotModelSet.z < degToRad(0.0f)) rotModelSet.z = degToRad(360.0f);
}

static int cLib_settingsModel(lua_State* L) {
    pd->graphics->clear(kColorBlack);
    buf = pd->graphics->getFrame();
    for (int i=0; i < (sW * sH); i++){ zBuffer[i] = FLT_MAX16; }
    allAmt = 0;

    const float CamXDirSin = -sinf(FROM_FIXED32(setCam.rotation.x));
    const float CamXDirCos = cosf(FROM_FIXED32(setCam.rotation.x));
    const float CamYDirSin = -sinf(FROM_FIXED32(setCam.rotation.y));
    const float CamYDirCos = cosf(FROM_FIXED32(setCam.rotation.y));
    const float CamZDirSin = -sinf(FROM_FIXED32(setCam.rotation.z));
    const float CamZDirCos = cosf(FROM_FIXED32(setCam.rotation.z));

    addModel(pd->lua->getArgInt(2));
    qsort(allPoints, allAmt, sizeof(worldTris), compareRenderTris);
    
    renderTris(CamYDirSin, CamYDirCos, CamXDirSin, CamXDirCos, CamZDirSin, CamZDirCos, setCam);

    return 0;
}

static int cLib_SettingsChange(lua_State* L){
    renderRadius = pd->lua->getArgInt(2);
    pixSizeX = pd->lua->getArgInt(3);
    pixSizeY = pd->lua->getArgInt(4);

    camForward.x = cosf(FROM_FIXED32(setCam.rotation.x)) * sinf(FROM_FIXED32(setCam.rotation.y));
    camForward.y = sinf(FROM_FIXED32(setCam.rotation.x));
    camForward.z = cosf(FROM_FIXED32(setCam.rotation.x)) * cosf(FROM_FIXED32(setCam.rotation.y));

    return 0;
}