#include "library.h"
#include "entities.h"
#include "draw.h"
#include "movement.h"
#include "collisions.h"
#include "Objects/allMeshes.h"
#include "Objects/mesh.h"

PlaydateAPI* pd;
Camera cam;
worldTris* allPoints;
staticPoints* static3D;
worldTris* entModels;
int gameStart = 1;

uint8_t* buf = NULL;
qfixed16_t zBuffer[SCREEN_W * SCREEN_H];

int rendAmt = 0;
int entAmt = 0;
int allAmt = 0;
int staticAmt = 0;

const int debug = 0;
const int colRend = 0;
const int renderRadius = 85;

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

Vect3f camForward;

static int cLib_init(lua_State* L);
static int cLib_render(lua_State* L);
static int cLib_playerAction(lua_State* L);
static int cLib_addEnt(lua_State* L);

static const lua_reg cLibs[] ={
    { "init", cLib_init },
    { "render", cLib_render },
    { "player", cLib_playerAction },
    { "addEnt", cLib_addEnt },
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
            mapArray[modelIndex].data[idx[0]].x, mapArray[modelIndex].data[idx[0]].y, mapArray[modelIndex].data[idx[0]].z,
            mapArray[modelIndex].data[idx[1]].x, mapArray[modelIndex].data[idx[1]].y, mapArray[modelIndex].data[idx[1]].z,
            mapArray[modelIndex].data[idx[2]].x, mapArray[modelIndex].data[idx[2]].y, mapArray[modelIndex].data[idx[2]].z,
            SURFACE_NONE
        );
    }

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

    player = createPlayer(10.0f, 3.0f, 41.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.5f, 1.8f, 0.56f, 0.08f, 0);

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
        allEnts = pd->system->realloc(allEnts, sizeof(EntStruct) * (entAmt + 1));
        allEnts[entAmt] = createEntity(xPos, yPos, zPos, xRot, yRot, zRot, xSize, ySize, zSize, radius, height, frict, fallFrict, type);
        entAmt++;

        allPoints = pd->system->realloc(allPoints, (allPointsCount + entArray[type].count) * sizeof(worldTris));
    } else {
        pd->system->logToConsole("Max entities reached!");
    }

    return 0;
}

static void renderCollision(float x, float y, float z, float CamYDirSin, float CamYDirCos, float CamXDirSin, float CamXDirCos, float CamZDirSin, float CamZDirCos){
    int size = 10;
    int half = size / 2;

    float heightMod[2] = {0.0f, FROM_FIXED32(player.height)};
    for (int i = 0; i < 2; i++) {
        int point[2];
        float rot[3];
        RotationMatrix(
            x - FROM_FIXED32(cam.position.x),
            (y + heightMod[i]) - FROM_FIXED32(cam.position.y),
            z - FROM_FIXED32(cam.position.z),
            CamYDirSin, CamYDirCos,
            CamXDirSin, CamXDirCos,
            CamZDirSin, CamZDirCos,
            &rot[0]
        );

        if (rot[2] <= 0.0f) continue;
        project2D(&point[0], rot, FROM_FIXED32(cam.fov), FROM_FIXED32(cam.nearPlane));

        pd->graphics->fillRect((int)(point[0] - (half+1)), (int)(point[1] - (half+1)), size+2, size+2, kColorBlack);
        pd->graphics->fillRect((int)(point[0] - half), (int)(point[1] - half), size, size, kColorWhite);
    }
}

static void renderLines(float CamYDirSin, float CamYDirCos, float CamXDirSin, float CamXDirCos, float CamZDirSin, float CamZDirCos){
    int point[2][2];

    for (int index=0; index < staticAmt; index++){
        int x = static3D[index].pos.x;
        int y = static3D[index].pos.y;
        int z = static3D[index].pos.z;
        int verts[3][3];
        int color = static3D[index].color;

        for (int i=0; i < 2; i++){
            for (int z=0; z < 3; z++){
                verts[i][z] = static3D[index].joint[i][z];
            }
        }

        for (int v=0; v < 2; v++){
            float rot[3] = {0.0f, 0.0f, 0.0f};
            RotationMatrix(
                (float)(x + verts[v][0]) - FROM_FIXED32(cam.position.x),
                (float)(y + verts[v][1]) - FROM_FIXED32(cam.position.y),
                (float)(z + verts[v][2]) - FROM_FIXED32(cam.position.z),
                CamYDirSin, CamYDirCos,
                CamXDirSin, CamXDirCos,
                CamZDirSin, CamZDirCos,
                &rot[0]
            );
            project2D(&point[v][0], rot, cam.fov, cam.nearPlane);
        }

        staticLineDrawing(point[0], point[1], color);
    }
}

static void renderTris(float CamYDirSin, float CamYDirCos, float CamXDirSin, float CamXDirCos, float CamZDirSin, float CamZDirCos){
    float fov = FROM_FIXED32(cam.fov);
    float nearPlane = FROM_FIXED32(cam.nearPlane);
    float farPlane = FROM_FIXED32(cam.farPlane);
    Vect3f camPos = {FROM_FIXED32(cam.position.x), FROM_FIXED32(cam.position.y), FROM_FIXED32(cam.position.z)};

    int tri1[3][2];
    int tri2[3][2];
    int check[3][2];
    Vertex verts[3];
    clippedTri clip1, clip2;

    for (int index = 0; index < allAmt; index++){
        int color = allPoints[index].objType;
        for (int v = 0; v < 3; v++) {
            verts[v].x = allPoints[index].verts[v][0] - camPos.x;
            verts[v].y = allPoints[index].verts[v][1] - camPos.y;
            verts[v].z = allPoints[index].verts[v][2] - camPos.z;

            float rot[3];
            RotationMatrix(
                verts[v].x, verts[v].y, verts[v].z,
                CamYDirSin, CamYDirCos,
                CamXDirSin, CamXDirCos,
                CamZDirSin, CamZDirCos,
                &rot[0]
            );

            verts[v].x = rot[0];
            verts[v].y = rot[1];
            verts[v].z = rot[2];
            project2D(&check[v][0], (float[]){verts[v].x, verts[v].y, verts[v].z}, fov, nearPlane);
        }

        if (!windingOrder(check[0], check[1], check[2])) { continue; }
        if (verts[0].z <= 0.0f && verts[1].z <= 0.0f && verts[2].z <= 0.0f) { continue; }

        int output = TriangleClipping(verts, &clip1, &clip2, nearPlane, farPlane);
        if (output) { 
            project2D(&tri1[0][0], (float[3]){clip1.t1.x, clip1.t1.y, clip1.t1.z}, fov, nearPlane);
            project2D(&tri1[1][0], (float[3]){clip1.t2.x, clip1.t2.y, clip1.t2.z}, fov, nearPlane);
            project2D(&tri1[2][0], (float[3]){clip1.t3.x, clip1.t3.y, clip1.t3.z}, fov, nearPlane);

            clip1.t1.z = (clip1.t1.z - nearPlane) / (farPlane - nearPlane);
            clip1.t2.z = (clip1.t2.z - nearPlane) / (farPlane - nearPlane);
            clip1.t3.z = (clip1.t3.z - nearPlane) / (farPlane - nearPlane);
            drawFilledTrisZ(tri1, clip1, color, zBuffer);
            if (output == 2){
                project2D(&tri2[0][0], (float[3]){clip2.t1.x, clip2.t1.y, clip2.t1.z}, fov, nearPlane); 
                project2D(&tri2[1][0], (float[3]){clip2.t2.x, clip2.t2.y, clip2.t2.z}, fov, nearPlane);
                project2D(&tri2[2][0], (float[3]){clip2.t3.x, clip2.t3.y, clip2.t3.z}, fov, nearPlane);

                clip2.t1.z = (clip2.t1.z - nearPlane) / (farPlane - nearPlane);
                clip2.t2.z = (clip2.t2.z - nearPlane) / (farPlane - nearPlane);
                clip2.t3.z = (clip2.t3.z - nearPlane) / (farPlane - nearPlane);
                drawFilledTrisZ(tri2, clip2, color, zBuffer);
            }
        }
    }
}

static void addObjectToWorld(Vect3f pos, Vect3f rot, Vect3f size, int type, int triCount, Vect3m* data, int* colorArray, int posDist) {
    Vect3f dirNorm;
    float rotMat[3][3];
    computeRotScaleMatrix(rotMat, rot.x, rot.y, rot.z, size.x, size.y, size.z);

    float camX = FROM_FIXED32(cam.position.x);
    float camY = FROM_FIXED32(cam.position.y);
    float camZ = FROM_FIXED32(cam.position.z);

    float near = FROM_FIXED32(cam.nearPlane);
    float far = FROM_FIXED32(cam.farPlane);

    float dx = pos.x - camX;
    float dy = pos.y - camY;
    float dz = pos.z - camZ;

    float dist;
    if (posDist){
        dist = dx*dx + dy*dy + dz*dz;
        if (renderRadius && dist > renderRadius * renderRadius) return;
    }

    for (int i = 0; i < triCount; i++) {
        float cx = (data[i*3].x + data[i*3 + 1].x + data[i*3 + 2].x) / 3.0f;
        float cy = (data[i*3].y + data[i*3 + 1].y + data[i*3 + 2].y) / 3.0f;
        float cz = (data[i*3].z + data[i*3 + 1].z + data[i*3 + 2].z) / 3.0f;

        dist = (cx - camX)*(cx - camX) + (cy - camY)*(cy - camY) + (cz - camZ)*(cz - camZ);
        if (renderRadius && dist > renderRadius * renderRadius) continue;

        if (dist > 1e-8f) {
            float invLen = 1.0f / sqrtf(dist);
            dirNorm.x = (cx - camX) * invLen;
            dirNorm.y = (cy - camY) * invLen;
            dirNorm.z = (cz - camZ) * invLen;
        } else {
            continue;
        }
        
        if (dot(dirNorm, camForward) > -0.867f) {
            worldTris tri;
            for (int j = 0; j < 3; j++) {
                float rotated[3];
                rotateVertex(data[i*3 + j].x, data[i*3 + j].y, data[i*3 + j].z, rotMat, rotated);
                tri.verts[j][0] = rotated[0] + pos.x;
                tri.verts[j][1] = rotated[1] + pos.y;
                tri.verts[j][2] = rotated[2] + pos.z;
            }

            tri.objType = colorArray[i];
            tri.dist = dist;
            allPoints[allAmt++] = tri;
        }
    }
}

static void addPlayer() {
    if (player.type < 0 || player.type >= maxEntStored) return;
    addObjectToWorld(
        (Vect3f){FROM_FIXED32(player.position.x), FROM_FIXED32(player.position.y), FROM_FIXED32(player.position.z)},
        (Vect3f){FROM_FIXED32(player.rotation.x), FROM_FIXED32(player.rotation.y), FROM_FIXED32(player.rotation.z)},
        (Vect3f){FROM_FIXED32(player.size.x), FROM_FIXED32(player.size.y), FROM_FIXED32(player.size.z)},
        player.type,
        entArray[player.type].count,
        entArray[player.type].data,
        entArray[player.type].color,
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
            allEnts[i].type,
            entArray[allEnts[i].type].count,
            entArray[allEnts[i].type].data,
            entArray[allEnts[i].type].color,
            true
        );
    }
}

static void addMap(){
    addObjectToWorld(
        (Vect3f){0.0f, 0.0f, 0.0f},
        (Vect3f){0.0f, 0.0f, 0.0f},
        (Vect3f){1.0f, 1.0f, 1.0f},
        modelIndex,
        mapArray[modelIndex].count,
        mapArray[modelIndex].data,
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
    addEntities(CamYDirSin, CamYDirCos, CamXDirSin, CamXDirCos, CamZDirSin, CamZDirCos);
    addPlayer(CamYDirSin, CamYDirCos, CamXDirSin, CamXDirCos, CamZDirSin, CamZDirCos);
    // qsort(allPoints, allAmt, sizeof(worldTris), compareRenderTris);
    
    renderTris(CamYDirSin, CamYDirCos, CamXDirSin, CamXDirCos, CamZDirSin, CamZDirCos);

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

    if (colRend){
        for (int i=0; i < substeps; i++){ 
            renderCollision(pColPoints[i][0], pColPoints[i][1], pColPoints[i][2], CamYDirSin, CamYDirCos, CamXDirSin, CamXDirCos, CamZDirSin, CamZDirCos);
        }
    }

    return 0;
}

static int cLib_playerAction(lua_State* L) {
    pd->graphics->clear(kColorBlack);
    buf = pd->graphics->getFrame();
    for (int i=0; i < (SCREEN_W * SCREEN_H); i++){ zBuffer[i] = FLT_MAX16; }
    allAmt = 0;

    entModels = pd->system->realloc(entModels, sizeof(worldTris) * 0);

    movePlayerObj(&player, &cam, colRend);
    handleCameraInput(&cam);
    updateCamera(&cam, &player, 7.0f);

    for(int i=0; i < entAmt; i++){ moveEntObj(&allEnts[i], &player); }

    camForward.x = cosf(FROM_FIXED32(cam.rotation.x)) * sinf(FROM_FIXED32(cam.rotation.y));
    camForward.y = sinf(FROM_FIXED32(cam.rotation.x));
    camForward.z = cosf(FROM_FIXED32(cam.rotation.x)) * cosf(FROM_FIXED32(cam.rotation.y));

    uint64_t currentTime = pd->system->getCurrentTimeMilliseconds();

    if (lastTime != 0) {
        deltaTime = (currentTime - lastTime) / 1000000.0f;
    } else {
        deltaTime = 0.0f;
    }

    lastTime = currentTime;

    return 0;
}