#include "libRay.h"
#include "movement.h"
#include "draw.h"
#include "meshConvert.h"

#include "../Foxgine/engine.h"

PlaydateAPI* pd;

Camera_t cam;
worldTris* entModels;
textAtlas* textAtlasMem;
Objects* allEnts;
EntStruct player;
Mesh_t mapArray;
Mesh_t* objArray;
VertAnims* entArray;

int gameScreen = 0;
const int maxProjs = 1;
const int mapObjsCount = 2;

int onStart = 0;

static int update(void* userdata);

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg)
{
	if ( event == kEventInit )
	{
        pd = playdate;

		pd->display->setRefreshRate(30);
		pd->system->setUpdateCallback(update, NULL);
	}
	
	return 0;
}

void eventHandlerShim(void) {}

static void generateEnts() {
    if (mapIndex == 0) {
        addEnt((Vect3f){0.0f, 20.0f, -5.0f}, (Vect3f){0.0f, 0.0f, 0.0f}, (Vect3f){0.5f, 0.5f, 0.5f}, 0.5f, 1.8f, 0.56f, 0.08f, 0, ENTITY, entArray, allEnts);
        addEnt((Vect3f){0.0f, 5.0f, 5.0f}, (Vect3f){0.0f, 0.0f, 0.0f}, (Vect3f){0.5f, 0.5f, 0.5f}, 0.5f, 1.8f, 0.56f, 0.08f, 0, OBJECT, entArray, allEnts);
    } else if (mapIndex == 1) {
        addEnt((Vect3f){0.0f, 5.0f, 5.0f}, (Vect3f){0.0f, 0.0f, 0.0f}, (Vect3f){0.5f, 0.5f, 0.5f}, 0.5f, 1.8f, 0.56f, 0.08f, 0, OBJECT, entArray, allEnts);
    }
}

static int init() {
    allPointsCount = 0;
    entIndex = 0;
    entAmt = 0;
    allAmt = 0;

    cam = createCamera(10.0f, 3.0f, 41.0f, 0.0f, 0.0f, 0.0f, 90.0f, 0.1f, 100.0f);
    player = createEntity(0.0f, 20.0f, -5.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.5f, 1.8f, 0.56f, 0.08f, 0);

    objArray = pd_malloc( sizeof(Mesh_t) * maxProjs);
    entArray = pd_malloc(sizeof(VertAnims) * entDataCount);
    allEnts = pd_malloc(sizeof(EntStruct) * MAX_ENTITIES);

    convertFileToMesh(mapObjs[mapIndex], &mapArray, mapData[mapIndex][0], mapData[mapIndex][1]);
    convertFileToMesh("Objects/proj/ball.obj", &objArray[0], 0, 0);

    for (int i=0; i < entDataCount; i++){
        int highest = allocAnimModel(&entArray[i], entData[i].totalAnims, entData[i].animFrameCounts, entData[i].animNames);
        allPointsCount += (highest * (entAmt+1));
        entArray[i].count = highest;
    }

    generateMap(mapArray);
    generateTextures(&textAtlasMem, 0);
    generateTriggers((Vect3f){5.0f, 5.0f, 5.0f}, (Vect3f){10.0f, 10.0f, 10.0f});
    generateEnts();

    resetAllVariables();

    return 0;
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
}

static void addEntities() {
    float dx, dy, dz;
    for (int z = 0; z < entAmt; z++) {
        switch (allEnts[z].type) {
            case ENTITY:
                EntStruct *ent_ = &allEnts[z].data.ent;
                if (ent_->type < 0 || ent_->type >= entDataCount) break;

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

static void addMap() {
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

static int render() {
    const float CamXDirSin = -sinf(FROM_FIXED32(cam.rotation.x));
    const float CamXDirCos = cosf(FROM_FIXED32(cam.rotation.x));
    const float CamYDirSin = -sinf(FROM_FIXED32(cam.rotation.y));
    const float CamYDirCos = cosf(FROM_FIXED32(cam.rotation.y));
    const float CamZDirSin = -sinf(FROM_FIXED32(cam.rotation.z));
    const float CamZDirCos = cosf(FROM_FIXED32(cam.rotation.z));

    addMap();
    addPlayer();
    addEntities();

    shootRender(CamYDirSin, CamYDirCos, CamXDirSin, CamXDirCos, CamZDirSin, CamZDirCos, cam, textAtlasMem);
    drawScreen();

    return 0;
}

static int playerAction() {
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

    return 0;
}

static int update(void* userdata) {
    if (onStart == 0){
        gameScreen = 1;

        init();
        scnBuf = pd_malloc(sizeof(int) * (sW/resolution * sH/resolution));
        onStart = 1;

        pd->system->logToConsole("Init Finished.");
    }

    for (int i=0; i < ((sW/resolution)*(sH/resolution)); i++) { scnBuf[i] = -1; }
    
    if (gameScreen == 1) {
        playerAction();
        render();

        pd->system->logToConsole("Player Pos: [ %f | %f | %f ]", FROM_FIXED32(player.position.x), FROM_FIXED32(player.position.y), FROM_FIXED32(player.position.z));
    }

    drawScreen();

    return 0;
}