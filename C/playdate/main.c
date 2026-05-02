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
textAnimsAtlas* allObjArray2D = NULL;
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
    pd_free(allObjArray2D);
    // pd_free(worldTextAtlasMem);

    pd->system->removeMenuItem(interlaceItem);
}

static int init() {
    allPointsCount = 0;
    entAmt = 0;

    // objArray3D = pd_malloc( sizeof(Mesh_t) * projDataCount3D);
    // entArray3D = pd_malloc(sizeof(VertAnims) * entDataCount3D);
    // allObjArray2D = pd_malloc( sizeof(textAnimsAtlas) * (entDataCount2D + projDataCount2D));
    // allEnts = pd_malloc(sizeof(EntStruct) * MAX_ENTITIES);

    cam = createCamera(0.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 90.0f, 0.1f, 1000.0f);
    player = createEntity(33.8f, 10.0f, -7.8f, 0.0f, 0.0f, 0.0f, 3.0f, 3.0f, 3.0f, 2.5f, 4.0f, 0.55f, 0.08f, 0, D_3D);
    addLightPoint((Vector3f){0.0f, 2.0f, -5.0f}, 50, 10.0f);

    // convertFileToMesh(mapObjs[mapIndex], &mapArray, mapData[mapIndex][0], mapData[mapIndex][1], 0, mapSize[mapIndex]);

    // for (int i=0; i < projDataCount3D; i++) convertFileToMesh(projObjs3D[i], &objArray3D[i], projData3D[i][0], projData3D[i][1], 0, (Vector3f){1.0f, 1.0f, 1.0f});
    // for (int i=0; i < entDataCount3D; i++){
    //     int highest = allocAnimModel(&entArray3D[i], entData3D[i].totalAnims, entData3D[i].animFrameCounts, entData3D[i].animNames, 0, 1, 1, (Vector3f){1.0f, 1.0f, 1.0f});
    //     allPointsCount += (highest * (entAmt+1));
    //     entArray3D[i].count = highest;
    // }

    // for (int i=0; i < entDataCount2D; i++) {
    //     allocAnimAtlas(&allObjArray2D[i], entData2D[i].totalAnims, entData2D[i].animFrameCounts, entData2D[i].animNames);
    //     allPointsCount++;
    // }

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
    movePlayerObj(&player, &cam, camType);
    addBilboard(player.position, player.size, cam);

    // Vector3f nPos = {player.position.x, player.position.y + player.height, player.position.z};
    // addObjToWorld2D(nPos, cam, 10, 10, -1, 0);
    // addObjToWorld2D(player.position, cam, 0, 10, -1, 0);
}

static void addEntities(int ents, int objs) {
    for (int z = 0; z < entAmt; z++) {
        switch (allEnts[z].type) {
            case ENTITY:
                if (!ents) break;

                EntStruct *ent_ = &allEnts[z].data.ent;
                addBilboard(ent_->position, ent_->size, cam);

                break;
            case OBJECT:
                if (!objs) break;

                ObjStruct *obj_ = &allEnts[z].data.obj;
                addBilboard(obj_->position, obj_->size, cam);

                break;
        }
    }
}

#define RENDER_DIST 150758.0f
#define RENDER_DIST_WATER 75379.0f
static void addMap() {
    float renderDist = cam.farPlane * 0.8f;
    for (int i=0; i < sectorAmt; i++) {
        Mesh_Chunks* sector = &sectorMesh[i];

        Mesh_t map = sector->map;
        if (map.triCount <= 0) continue;

        Vector3f pos = sector->pos; Vector3f whd = sector->whd;

        Vector2f dist = {
            (pos.x + whd.x * 0.5f) - cam.position.x,
            (pos.z + whd.z * 0.5f) - cam.position.z
        }; float distSq = dist.x*dist.x + dist.z*dist.z;
        Vector2f halfWD = {
            (whd.x * 0.5f),
            (whd.z * 0.5f)
        }; float chunkRadius = halfWD.x*halfWD.x + halfWD.z*halfWD.z;
        float maxDist = RENDER_DIST + chunkRadius;

        if (distSq >= maxDist) continue;

        Vector3f rot = {0.0f, 0.0f, 0.0f};
        Vector3f size = {1.0f, 1.0f, 1.0f};

        addObjToWorld3D(
            pos, rot, size,
            cam, 0.0f,
            map, false
        );
    }

    for (int w=0; w < waterAmt; w++) {
        int amt = waterSlice[w].lineCount;
        if (amt == 0) continue;
    
        Vector2i pos = waterSlice[w].min;
        Vector2i whd = waterSlice[w].max;

        Vector2f dist = {
            (pos.x + whd.x * 0.5f) - cam.position.x,
            (pos.z + whd.z * 0.5f) - cam.position.z
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

    shootRender(cam, allObjArray2D);
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
    }
    pd->graphics->fillRect(0, 0, 20, 20, kColorWhite);
    pd->system->drawFPS(2, 2);

    return 1;
}