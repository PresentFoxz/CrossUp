#include "library.h"
#include "entities.h"
#include "draw.h"
#include "movement.h"
#include "collisions.h"
#include "Objects/allMeshes.h"
#include "Objects/mesh.h"

PlaydateAPI* pd;
Camera cam;
worldTris* mapPoints;
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
Vect3f camForward;
const int maxMaps = 1;
const int maxEntStored = 2;

int modelIndex = 0;
const int lengthJoints = 3;

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
    resetCollisionSurface();
    for (int i=0; i < len; i++){
        int idx[3] = {(i*3), (i*3)+1, (i*3)+2};

        mapPoints[rendAmt++] = (worldTris){
            .verts = {
                {mapArray[modelIndex].data[idx[0]].x, mapArray[modelIndex].data[idx[0]].y, mapArray[modelIndex].data[idx[0]].z},
                {mapArray[modelIndex].data[idx[1]].x, mapArray[modelIndex].data[idx[1]].y, mapArray[modelIndex].data[idx[1]].z},
                {mapArray[modelIndex].data[idx[2]].x, mapArray[modelIndex].data[idx[2]].y, mapArray[modelIndex].data[idx[2]].z}
            },
            .objType = mapArray[modelIndex].color[i],
            .dist = 0.0f
        };

        addCollisionSurface(
            mapArray[modelIndex].data[idx[0]].x, mapArray[modelIndex].data[idx[0]].y, mapArray[modelIndex].data[idx[0]].z,
            mapArray[modelIndex].data[idx[1]].x, mapArray[modelIndex].data[idx[1]].y, mapArray[modelIndex].data[idx[1]].z,
            mapArray[modelIndex].data[idx[2]].x, mapArray[modelIndex].data[idx[2]].y, mapArray[modelIndex].data[idx[2]].z,
            SURFACE_NONE
        );
    }
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

    mapPoints = pd->system->realloc(mapPoints, sizeof(worldTris) * mapArray[modelIndex].count);
    static3D = pd->system->realloc(static3D, sizeof(staticPoints) * lengthJoints);
    allEnts = pd->system->realloc(allEnts, sizeof(EntStruct) * entAmt);

    generateMap(mapArray[modelIndex].count);
    generatePoints(lengthJoints);

    player = createPlayer(10.0f, 3.0f, 41.0f, 0.0f, 0.0f, 0.0f, 0.5f, 1.8f, 0.56f, 0.08f, 0);

    return 0;
}

static int cLib_addEnt(lua_State* L){
    float xPos = pd->lua->getArgFloat(2);
    float yPos = pd->lua->getArgFloat(3);
    float zPos = pd->lua->getArgFloat(4);

    float xRot = pd->lua->getArgFloat(5);
    float yRot = pd->lua->getArgFloat(6);
    float zRot = pd->lua->getArgFloat(7);

    float radius = pd->lua->getArgFloat(8);
    float height = pd->lua->getArgFloat(9);

    float frict = pd->lua->getArgFloat(10);
    float fallFrict = pd->lua->getArgFloat(11);

    int type = pd->lua->getArgInt(12);

    if (entAmt < maxEntities){
        allEnts = pd->system->realloc(allEnts, sizeof(EntStruct) * (entAmt + 1));
        allEnts[entAmt] = createEntity(xPos, yPos, zPos, xRot, yRot, zRot, radius, height, frict, fallFrict, type);
        entAmt++;
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

static void renderTris(float CamYDirSin, float CamYDirCos, float CamXDirSin, float CamXDirCos, float CamZDirSin, float CamZDirCos, float fovX, float fovY){
    int objRend = 0;
    for (int index = 0; index < allAmt; index++){
        int tri1[3][2];
        int tri2[3][2];
        int check[3][2];

        // if (allPoints[index].dist > 200.0f) { continue; }
        Vertex verts[3];
        clippedTri clip1, clip2;
        int color = allPoints[index].objType;

        for (int i=0; i < 3; i++){
            verts[i].x = allPoints[index].verts[i][0];
            verts[i].y = allPoints[index].verts[i][1];
            verts[i].z = allPoints[index].verts[i][2];
        }

        for (int v=0; v < 3; v++){
            float rot[3];
            RotationMatrix(
                (float)verts[v].x - FROM_FIXED32(cam.position.x),
                (float)verts[v].y - FROM_FIXED32(cam.position.y),
                (float)verts[v].z - FROM_FIXED32(cam.position.z),
                CamYDirSin, CamYDirCos,
                CamXDirSin, CamXDirCos,
                CamZDirSin, CamZDirCos,
                &rot[0]
            );

            verts[v].x = rot[0];
            verts[v].y = rot[1];
            verts[v].z = rot[2];
            project2D(&check[v][0], (float[]){verts[v].x, verts[v].y, verts[v].z}, FROM_FIXED32(cam.fov), FROM_FIXED32(cam.nearPlane));
        }
        
        if (verts[0].z <= 0.0f && verts[1].z <= 0.0f && verts[2].z <= 0.0f) { continue; }

        int output = TriangleClipping(verts, &clip1, &clip2, FROM_FIXED32(cam.nearPlane), FROM_FIXED32(cam.farPlane));
        if (output <= 0) { continue; }

        project2D(&tri1[0][0], (float[3]){clip1.t1.x, clip1.t1.y, clip1.t1.z}, FROM_FIXED32(cam.fov), FROM_FIXED32(cam.nearPlane));
        project2D(&tri1[1][0], (float[3]){clip1.t2.x, clip1.t2.y, clip1.t2.z}, FROM_FIXED32(cam.fov), FROM_FIXED32(cam.nearPlane));
        project2D(&tri1[2][0], (float[3]){clip1.t3.x, clip1.t3.y, clip1.t3.z}, FROM_FIXED32(cam.fov), FROM_FIXED32(cam.nearPlane));
        if (output == 2){
            project2D(&tri2[0][0], (float[3]){clip2.t1.x, clip2.t1.y, clip2.t1.z}, FROM_FIXED32(cam.fov), FROM_FIXED32(cam.nearPlane)); 
            project2D(&tri2[1][0], (float[3]){clip2.t2.x, clip2.t2.y, clip2.t2.z}, FROM_FIXED32(cam.fov), FROM_FIXED32(cam.nearPlane));
            project2D(&tri2[2][0], (float[3]){clip2.t3.x, clip2.t3.y, clip2.t3.z}, FROM_FIXED32(cam.fov), FROM_FIXED32(cam.nearPlane));
        }

        if (windingOrder(check[0], check[1], check[2])){
            float nearDist = FROM_FIXED32(cam.nearPlane);
            float farDist = FROM_FIXED32(cam.farPlane);

            clip1.t1.z = (clip1.t1.z - nearDist) / (farDist - nearDist);
            clip1.t2.z = (clip1.t2.z - nearDist) / (farDist - nearDist);
            clip1.t3.z = (clip1.t3.z - nearDist) / (farDist - nearDist);
            drawFilledTrisZ(tri1, clip1, color, zBuffer);
            
	        if (output == 2) {
                clip2.t1.z = (clip2.t1.z - nearDist) / (farDist - nearDist);
                clip2.t2.z = (clip2.t2.z - nearDist) / (farDist - nearDist);
                clip2.t3.z = (clip2.t3.z - nearDist) / (farDist - nearDist);
                drawFilledTrisZ(tri2, clip2, color, zBuffer);
            }
        }

        objRend++;
    }

    // pd->system->logToConsole("Rendered: %d tris", objRend);
}

static void addPlayer(float CamYDirSin, float CamYDirCos, float CamXDirSin, float CamXDirCos, float CamZDirSin, float CamZDirCos, float fovX, float fovY){
    int playerRendered = 0;
    if (player.type < 0) return;
    if (player.type >= maxEntStored) return;

    int triCount = entArray[player.type].count;
    if (triCount <= 0) return;

    Vect3f dirNorm;
    Vect3f pos = {FROM_FIXED32(player.position.x), FROM_FIXED32(player.position.y), FROM_FIXED32(player.position.z)};

    float sinx = sinf(FROM_FIXED32(player.rotation.x));
    float cosx = cosf(FROM_FIXED32(player.rotation.x));
    float siny = sinf(FROM_FIXED32(player.rotation.y));
    float cosy = cosf(FROM_FIXED32(player.rotation.y));
    float sinz = sinf(FROM_FIXED32(player.rotation.z));
    float cosz = cosf(FROM_FIXED32(player.rotation.z));
    
    entModels = pd->system->realloc(entModels, sizeof(worldTris) * entArray[player.type].count);

    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float offsetZ = 0.0f;

    float dx = pos.x - FROM_FIXED32(cam.position.x);
    float dy = pos.y - FROM_FIXED32(cam.position.y);
    float dz = pos.z - FROM_FIXED32(cam.position.z);
            
    float dist = dx*dx + dy*dy + dz*dz;
    if (renderRadius && dist > renderRadius * renderRadius) return;

    for (int i=0; i < entArray[player.type].count; i++){
        int idx[3] = {(i*3), (i*3)+1, (i*3)+2};
    
        entModels[playerRendered++] = (worldTris){
            .verts = {
                { (entArray[player.type].data[idx[0]].x + offsetX),
                  (entArray[player.type].data[idx[0]].y + offsetY),
                  (entArray[player.type].data[idx[0]].z + offsetZ) },
        
                { (entArray[player.type].data[idx[1]].x + offsetX),
                  (entArray[player.type].data[idx[1]].y + offsetY),
                  (entArray[player.type].data[idx[1]].z + offsetZ) },

                { (entArray[player.type].data[idx[2]].x + offsetX),
                  (entArray[player.type].data[idx[2]].y + offsetY),
                  (entArray[player.type].data[idx[2]].z + offsetZ) }
            },
            .objType = entArray[player.type].color[i],
            .dist = dist
        };
    }

    allPoints = pd->system->realloc(allPoints, (allAmt + entArray[player.type].count) * sizeof(worldTris));
    for(int m=0; m < entArray[player.type].count; m++){
        if (dist > 1e-8f) {
            float invLen = 1.0f / sqrtf(dist);
            dirNorm.x = dx * invLen;
            dirNorm.y = dy * invLen;
            dirNorm.z = dz * invLen;
        } else {
            continue;
        }

        if (dot(dirNorm, camForward) > -0.867f) {
            float oRot[3][3];
            for (int z=0; z<3; z++){
                RotateVertexObject(
                    entModels[m].verts[z][0], entModels[m].verts[z][1], entModels[m].verts[z][2],
                    sinx, cosx, siny, cosy, sinz, cosz,
                    1.0f, 1.0f, 1.0f,
                    &oRot[z][0]
                );
            }

            allPoints[allAmt++] = (worldTris){
                .verts = {
                    {oRot[0][0] + pos.x, oRot[0][1] + pos.y, oRot[0][2] + pos.z},
                    {oRot[1][0] + pos.x, oRot[1][1] + pos.y, oRot[1][2] + pos.z},
                    {oRot[2][0] + pos.x, oRot[2][1] + pos.y, oRot[2][2] + pos.z}
                },
                .objType = entModels[m].objType,
                .dist = dist
            };
        }
    }
}

static void addEntities(float CamYDirSin, float CamYDirCos, float CamXDirSin, float CamXDirCos, float CamZDirSin, float CamZDirCos, float fovX, float fovY){
    Vect3f dirNorm;

    for(int i = 0; i < entAmt; i++){
        int entRendered = 0;
        entModels = pd->system->realloc(entModels, sizeof(worldTris) * entArray[allEnts[i].type].count);
        Vect3f pos = {FROM_FIXED32(allEnts[i].position.x), FROM_FIXED32(allEnts[i].position.y), FROM_FIXED32(allEnts[i].position.z)};

        float sinx = sinf(FROM_FIXED32(allEnts[i].rotation.x));
        float cosx = cosf(FROM_FIXED32(allEnts[i].rotation.x));
        float siny = sinf(FROM_FIXED32(allEnts[i].rotation.y));
        float cosy = cosf(FROM_FIXED32(allEnts[i].rotation.y));
        float sinz = sinf(FROM_FIXED32(allEnts[i].rotation.z));
        float cosz = cosf(FROM_FIXED32(allEnts[i].rotation.z));

        float dx = pos.x - FROM_FIXED32(cam.position.x);
        float dy = pos.y - FROM_FIXED32(cam.position.y);
        float dz = pos.z - FROM_FIXED32(cam.position.z);
                
        float dist = dx*dx + dy*dy + dz*dz;
        if (renderRadius && dist > renderRadius * renderRadius) continue;

        for (int z=0; z < entArray[allEnts[i].type].count; z++){
            int idx[3] = {(z*3), (z*3)+1, (z*3)+2};
            
            entModels[entRendered++] = (worldTris){
                .verts = {
                    {entArray[allEnts[i].type].data[idx[0]].x, entArray[allEnts[i].type].data[idx[0]].y, entArray[allEnts[i].type].data[idx[0]].z},
                    {entArray[allEnts[i].type].data[idx[1]].x, entArray[allEnts[i].type].data[idx[1]].y, entArray[allEnts[i].type].data[idx[1]].z},
                    {entArray[allEnts[i].type].data[idx[2]].x, entArray[allEnts[i].type].data[idx[2]].y, entArray[allEnts[i].type].data[idx[2]].z}
                },
                .objType = entArray[allEnts[i].type].color[z],
                .dist = 0.0f
            };
        }

        allPoints = pd->system->realloc(allPoints, (allAmt + entArray[allEnts[i].type].count) * sizeof(worldTris));
        for(int m=0; m < entArray[allEnts[i].type].count; m++){
            if (dist > 1e-8f) {
                float invLen = 1.0f / sqrtf(dist);
                dirNorm.x = dx * invLen;
                dirNorm.y = dy * invLen;
                dirNorm.z = dz * invLen;
            } else {
                continue;
            }
    
            if (dot(dirNorm, camForward) > -0.867f) {
                float oRot[3][3];
                for (int z=0; z<3; z++){
                    RotateVertexObject(
                        entModels[m].verts[z][0], entModels[m].verts[z][1], entModels[m].verts[z][2],
                        sinx, cosx, siny, cosy, sinz, cosz,
                        1.0f, 1.0f, 1.0f,
                        &oRot[z][0]
                    );
                }

                allPoints[allAmt++] = (worldTris){
                    .verts = {
                        {oRot[0][0] + pos.x, oRot[0][1] + pos.y, oRot[0][2] + pos.z},
                        {oRot[1][0] + pos.x, oRot[1][1] + pos.y, oRot[1][2] + pos.z},
                        {oRot[2][0] + pos.x, oRot[2][1] + pos.y, oRot[2][2] + pos.z}
                    },
                    .objType = entModels[m].objType,
                    .dist = dist
                };
            }
        }
    }
}

static void addMap(){
    Vect3f dirNorm;
    allPoints = pd->system->realloc(allPoints, (allAmt + rendAmt) * sizeof(worldTris));
    for (int i = 0; i < rendAmt; i++) {
        float* v0 = mapPoints[i].verts[0];
        float* v1 = mapPoints[i].verts[1];
        float* v2 = mapPoints[i].verts[2];

        float cx = (v0[0] + v1[0] + v2[0]) / 3.0f;
        float cy = (v0[1] + v1[1] + v2[1]) / 3.0f;
        float cz = (v0[2] + v1[2] + v2[2]) / 3.0f;
        
        float dx = cx - FROM_FIXED32(cam.position.x);
        float dy = cy - FROM_FIXED32(cam.position.y);
        float dz = cz - FROM_FIXED32(cam.position.z);
        
        float dist = dx*dx + dy*dy + dz*dz;
        if (renderRadius && dist > renderRadius * renderRadius) continue;

        if (dist > 1e-8f) {
            float invLen = 1.0f / sqrtf(dist);
            dirNorm.x = dx * invLen;
            dirNorm.y = dy * invLen;
            dirNorm.z = dz * invLen;
        } else {
            continue;
        }

        if (dot(dirNorm, camForward) > -0.867f) {
            allPoints[allAmt++] = (worldTris){
                .verts = {
                    {mapPoints[i].verts[0][0], mapPoints[i].verts[0][1], mapPoints[i].verts[0][2]},
                    {mapPoints[i].verts[1][0], mapPoints[i].verts[1][1], mapPoints[i].verts[1][2]},
                    {mapPoints[i].verts[2][0], mapPoints[i].verts[2][1], mapPoints[i].verts[2][2]}
                },
                .objType = mapPoints[i].objType,
                .dist = dist
            };
        }
    }
}

static int cLib_render(lua_State* L) {
    const float CamXDirSin = -sinf(FROM_FIXED32(cam.rotation.x));
    const float CamXDirCos = cosf(FROM_FIXED32(cam.rotation.x));
    const float CamYDirSin = -sinf(FROM_FIXED32(cam.rotation.y));
    const float CamYDirCos = cosf(FROM_FIXED32(cam.rotation.y));
    const float CamZDirSin = -sinf(FROM_FIXED32(cam.rotation.z));
    const float CamZDirCos = cosf(FROM_FIXED32(cam.rotation.z));

    const float fovX = 140.0f * (3.14159265f / 180.0f);
    const float fovY = 2.0f * atanf(tanf(fovX * 0.5f) * (240.0f / 400.0f));

    addMap();
    addEntities(CamYDirSin, CamYDirCos, CamXDirSin, CamXDirCos, CamZDirSin, CamZDirCos, fovX, fovY);
    addPlayer(CamYDirSin, CamYDirCos, CamXDirSin, CamXDirCos, CamZDirSin, CamZDirCos, fovX, fovY);
    // qsort(allPoints, allAmt, sizeof(worldTris), compareRenderTris);
    
    renderTris(CamYDirSin, CamYDirCos, CamXDirSin, CamXDirCos, CamZDirSin, CamZDirCos, fovX, fovY);
    // renderLines(CamYDirSin, CamYDirCos, CamXDirSin, CamXDirCos, CamZDirSin, CamZDirCos);

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
    
    allPoints = pd->system->realloc(allPoints, sizeof(worldTris) * 0);
    entModels = pd->system->realloc(entModels, sizeof(worldTris) * 0);

    movePlayerObj(&player, &cam);
    handleCameraInput(&cam);
    updateCamera(&cam, &player, 7.0f);

    for(int i=0; i < entAmt; i++){ moveEntObj(&allEnts[i], &player); }

    camForward.x = cosf(FROM_FIXED32(cam.rotation.x)) * sinf(FROM_FIXED32(cam.rotation.y));
    camForward.y = sinf(FROM_FIXED32(cam.rotation.x));
    camForward.z = cosf(FROM_FIXED32(cam.rotation.x)) * cosf(FROM_FIXED32(cam.rotation.y));

    return 0;
}