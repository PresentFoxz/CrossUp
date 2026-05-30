#include "game/libRay.h"
#include "game/movement.h"
#include "game/meshConvert.h"
#include "sound/audio.h"
#include "../Foxgine/collisions.h"

#include "../Foxgine/engine.h"
#include "profiler.h"

PlaydateAPI* pd;

Camera_t cam = {0};
Objects* allEnts = NULL;
EntStruct player = {0};
InputBuffer inpBuf = {0};

Mesh_t mapArray = {0};
Mesh_Chunks* sectorMesh;
WaterSlice* waterSlice;
static int mapIndex  = 0;
static int sectorAmt = 0;
static int waterAmt  = 0;

Mesh_t* objArray3D = NULL;
VertAnims* entArray3D = NULL;
textAnimsAtlas* allTexArray2D = NULL;
textAtlas* worldTextAtlasMem = NULL;

static int gameScreen = 0;
static int onStart = 0;
static int camType = 0;

int ambientLight = 0;
static int lastVal = 0;

static int update(void* userdata);
static int UnloadData();

PDMenuItem* interlaceItem;
void onInterlaceCycle(void* userdata) {
    const char* names[] = { "OFF", "1x", "2x", "4x" };
    int value = pd->system->getMenuItemValue(interlaceItem);
    value++;
    if (value > 3) value = 0;

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Interlace: %s", names[value]);
    pd->system->setMenuItemValue(interlaceItem, value);
    pd->system->setMenuItemTitle(interlaceItem, buffer);

    if (value != lastVal) {
        if (value == 0) { changeLacing(0, 0, false); }
        else if (value == 1) { changeLacing(0, 1, true); }
        else if (value == 2) { changeLacing(0, 2, true); }
        else if (value == 3) { changeLacing(0, 4, true); }
    } lastVal = value;

    pd->system->logToConsole("Interlace: %s", names[value]);
}

PDMenuItem* camStyle;
void onCameraSwap(void* userdata) {
    const char* names[] = { "Cam-Lock", "Cam-Float" };
    int value = pd->system->getMenuItemValue(camStyle);
    value++;
    if (value > 1) value = 0;

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Interlace: %s", names[value]);
    pd->system->setMenuItemValue(camStyle, value);
    pd->system->setMenuItemTitle(camStyle, buffer);
    camType = value;

    pd->system->logToConsole("Interlace: %s", names[camType]);
}

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg)
{
	if ( event == kEventInit )
	{
        pd = playdate;

		pd->display->setRefreshRate(BASE_FPS);
		pd->system->setUpdateCallback(update, NULL);

        interlaceItem = pd->system->addMenuItem("Interlace: OFF", onInterlaceCycle, NULL);
        camStyle = pd->system->addMenuItem("Cam-Lock", onCameraSwap, NULL);
	}

	if ( event == kEventTerminate )
	{
        UnloadData();
		UnloadAudioManager(&audioManager);
	}

	return 0;
}

static int UnloadData() {
    pd_free(allEnts);
    pd_free(objArray3D);
    pd_free(entArray3D);
    pd_free(allTexArray2D);
    // pd_free(worldTextAtlasMem);

    pd->system->removeMenuItem(interlaceItem);
}

static int init() {
    allPointsCount = 0;
    entAmt = 0;

    objArray3D = pd_malloc( sizeof(Mesh_t) * projDataCount3D);
    entArray3D = pd_malloc(sizeof(VertAnims) * entDataCount3D);
    allTexArray2D = pd_malloc( sizeof(textAnimsAtlas) * (entDataCount2D + projDataCount2D));
    allEnts = pd_malloc(sizeof(EntStruct) * MAX_ENTITIES);

    cam = createCamera(0.0f, 40.0f, -10.0f, 0.0f, 0.0f, 0.0f, 90.0f, 0.1f, 1000.0f);
    player = createEntity(0.0f, 20.0f, 0.0f, 0.0f, 0.0f, 0.0f, 3.0f, 3.0f, 3.0f, 2.5f, 4.0f, 0.55f, 0.08f, 0, D_3D);
    addLightPoint((Vector3f){0.0f, 2.0f, -5.0f}, 50, 10.0f);

    // convertFileToMesh(mapObjs[mapIndex], &mapArray, mapData[mapIndex][0], mapData[mapIndex][1], 0, mapSize[mapIndex]);

    for (int i=0; i < projDataCount3D; i++) convertFileToMesh(projObjs3D[i], &objArray3D[i], projData3D[i][0], projData3D[i][1], 0, (Vector3f){1.0f, 1.0f, 1.0f});
    for (int i=0; i < entDataCount3D; i++){
        int highest = allocAnimModel(&entArray3D[i], entData3D[i].totalAnims, entData3D[i].animFrameCounts, entData3D[i].animNames, 0, 1, 1, (Vector3f){1.0f, 1.0f, 1.0f});
        allPointsCount += ((highest * 2) * (entAmt+1));
        entArray3D[i].count = highest;
    }

    for (int i=0; i < entDataCount2D; i++) {
        allocAnimAtlas(&allTexArray2D[i], entData2D[i].totalAnims, entData2D[i].animFrameCounts, entData2D[i].animNames);
    }

    resetCollisionSurface();
    sectorMesh = readMapData(mapLeaf[mapIndex], &sectorAmt, &waterSlice, &waterAmt, &player, allEnts);
    allPointsCount += ((waterAmt * 2) + (entAmt * 2) + 2);

    for (int i=0; i < waterAmt; i++) { addWaves(waterSlice, i, randomInt(3, 5)); }
    for (int i=0; i < sectorAmt; i++) { generateMap(sectorMesh[i].map, sectorMesh[i].pos); }

    ambientLight = mapAmbientLight[mapIndex];
    // generateTriggers((Vector3f){5.0f, 5.0f, 5.0f}, (Vector3f){10.0f, 10.0f, 10.0f});

    resetAllArrays();

    InitAudioManager(&audioManager);
    // PlayMusic(&audioManager, "music/EITW", 1.0f, true, 0.0f);
    // PlayModuleMusic(&audioManager, "Echo in the Wind - Minecraft.wav");

    return 0;
}

static void addPlayer() {
    if (player.type < 0) return;
    if (player.dimention == D_3D && player.type >= entDataCount3D) return;
    if (player.dimention == D_2D && player.type >= entDataCount2D) return;
    movePlayerObj(&player, &cam, camType);

    if (player.currentAnim != player.lastAnim) {
        player.frameCount = 0;
        player.currentFrame = 0;
    }

    if (player.dimention == D_3D) {
        AnimFrames* anims = entArray3D[player.type].anims[player.currentAnim];
        int newFrame = anims->frames;

        if (player.currentFrame >= newFrame) {
            player.frameCount = 0;
            player.currentFrame = 0;
        }

        Mesh_t model = anims->meshModel[player.currentFrame];
        if (model.verts != NULL && model.triCount > 0 && model.bfc != NULL) {
            addObjToWorld3D(player.position, player.rotation, player.size, cam, 10.0f, model, false, -1, true, false);
        }
    } else if (player.dimention == D_BB) {
        addBilboard(player.position, player.size, cam, 0);
    } else if (player.dimention == D_2D) {
        addObjToWorld2D(player.position, cam, 10, 10, 0, 0);
    }

    player.lastAnim = player.currentAnim;
    player.frameCount++;
    if (player.frameCount > 4) {
        player.currentFrame++;
        player.frameCount = 0;
    }
}

static void addEntities(int ents, int objs) {
    for (int z = 0; z < entAmt; z++) {
        switch (allEnts[z].type) {
            case ENTITY:
                if (!ents) break;

                EntStruct *ent_ = &allEnts[z].data.ent;
                addBilboard(ent_->position, ent_->size, cam, -1);

                break;
            case OBJECT:
                if (!objs) break;

                ObjStruct *obj_ = &allEnts[z].data.obj;
                addBilboard(obj_->position, obj_->size, cam, -1);

                break;
        }
    }
}

#define RENDER_DIST 150758.0f
#define RENDER_DIST_WATER 75379.0f
static void addMap() {
    Vector3f cPos = cam.position;

    float renderDist = cam.farPlane * 0.8f;
    for (int i=0; i < sectorAmt; i++) {
        Mesh_Chunks* sector = &sectorMesh[i];

        Mesh_t map = sector->map;
        if (map.triCount <= 0) continue;

        Vector3f pos  = sector->pos;
        Vector3f rot  = {0, 0, 0};
        Vector3f size = {1, 1, 1};

        Vector3f min = sector->min;
        Vector3f max = sector->max;

        if ((cPos.x > min.x && cPos.x < max.x) && (cPos.z > min.z && cPos.z < max.z)) {
            addObjToWorld3D(pos, rot, size, cam, 0.0f, map, false, -1, false, false);
            continue;
        }

        Vector2f clamp = {cam.position.x, cam.position.z};
        if (clamp.x < min.x) clamp.x = min.x;
        if (clamp.z < min.z) clamp.z = min.z;
        if (clamp.x > max.x) clamp.x = max.x;
        if (clamp.z > max.z) clamp.z = max.z;

        int dx = cPos.x - clamp.x;
        int dz = cPos.z - clamp.z;
        int dist = dx*dx + dz*dz;
        if (dist > renderDist * renderDist) continue;

        addObjToWorld3D(pos, rot, size, cam, 0.0f, map, false, -1, false, false);

        // Vector3f positions[4] = {
        //     {sector->min.x, -50, sector->min.z},
        //     {sector->max.x, -50, sector->max.z},
        //     {sector->min.x, -50, sector->max.z},
        //     {sector->max.x, -50, sector->min.z}
        // };

        // Vector3f rotDeg = {0, 0, 0};
        // for (int p=0; p < 4; p++) { addLineTo3D(positions[p], rotDeg, 100, cam, true, 100); }
    }

    for (int w=0; w < waterAmt; w++) {
        int amt = waterSlice[w].lineCount;
        if (amt == 0) continue;
    
        Vector2i pos = waterSlice[w].min;
        Vector2i whd = waterSlice[w].max;

        Vector2f dist = {
            (pos.x + whd.x * 0.5f) - cPos.x,
            (pos.z + whd.z * 0.5f) - cPos.z
        }; float distSq = dist.x*dist.x + dist.z*dist.z;
        Vector2f halfWD = {
            (whd.x * 0.5f),
            (whd.z * 0.5f)
        }; float chunkRadius = halfWD.x*halfWD.x + halfWD.z*halfWD.z;
        float maxDist = RENDER_DIST_WATER + chunkRadius;

        if (distSq >= maxDist) continue;

        for (int i=0; i < waterSlice[w].lineCount; i++) {
            addWaveToWorld3D(&waterSlice[w].lines[i], waterSlice[w].min, waterSlice[w].max, cam);
        }
    }
}

static void gameRender() {
    if (camType == 0) {
        handleCameraInput(&cam);
        updateCamera(&cam, &player, 12.0f);
    } else {
        flyCameraInput(&cam);
    }
    
    // pd->system->logToConsole("Cam at: [ %f | %f | %f ]", cam.position.x, cam.position.y, cam.position.z);
    addMap();
    // addEntities(1, 0);
    addPlayer();

    shootRender(cam, allTexArray2D);
}

static void titleRender() {
    cam.rotation.y += -0.02f;
    addMap();
}

static int update(void* userdata) {
    if (onStart == 0){
        gameScreen = 0;

        init();
        onStart = 1;
    } runInputBuffer();

    float dt = pd->system->getElapsedTime();
    pd->system->resetElapsedTime();
    UpdateAudioManager(&audioManager, dt);

    if (gameScreen == 0) {
        pd->graphics->setDrawMode(kDrawModeFillWhite);
        precomputedFunctions(&cam);
        titleRender();
        blitToScreen();

        const char* msg = "Press A to Start!!";
        pd->graphics->fillRect(148, 118, 135, 25, kColorBlack);
        pd->graphics->drawText(msg, strlen(msg), kASCIIEncoding, 150, 120);
        if (inpBuf.A) { gameScreen = 1; }
    } else if (gameScreen == 1) {
        precomputedFunctions(&cam);
        gameRender();
        blitToScreen();

        char msg[128];
        sprintf(msg, "Cam: %d | %d | %d", (int)(cam.position.x), (int)(cam.position.y), (int)(cam.position.z));
        pd->graphics->fillRect(0, 0, 300, 20, kColorBlack);
        pd->graphics->drawText(msg, sizeof(msg), kASCIIEncoding, 2, 2);
    }
    pd->graphics->fillRect(0, 220, 20, 20, kColorWhite);
    pd->system->drawFPS(2, 222);

    return 1;
}