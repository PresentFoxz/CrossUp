#include "game/libRay.h"
#include "game/movement.h"
#include "game/meshConvert.h"
#include "sound/audio.h"

#include "../Foxgine/engine.h"

PlaydateAPI* pd;
uint8_t* buf = NULL;

Camera_t cam = {0};

Objects* allEnts = NULL;
EntStruct player = {0};
Mesh_t mapArray = {0};

Mesh_t* objArray3D = NULL;
VertAnims* entArray3D = NULL;
textAnimsAtlas* allObjArray2D = NULL;
textAtlas* worldTextAtlasMem = NULL;

int gameScreen = 0;
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

	if ( event == kEventTerminate )
	{
		UnloadAudioManager(&audioManager);
	}

	return 0;
}

static void generateEnts() {
    if (mapIndex == 0) {
        addEnt((Vect3f){0.0f, 20.0f, -5.0f}, (Vect3f){0.0f, 0.0f, 0.0f}, (Vect3f){1.0f, 1.0f, 1.0f}, 0.5f, 1.8f, 0.56f, 0.08f, 0, ENTITY, entArray3D, allEnts, D_3D);
        addEnt((Vect3f){0.0f, 5.0f, 5.0f}, (Vect3f){0.0f, 0.0f, 0.0f}, (Vect3f){0.5f, 0.5f, 0.5f}, 0.5f, 1.8f, 0.56f, 0.08f, 0, OBJECT, entArray3D, allEnts, D_3D);
    } else if (mapIndex == 1) {
        addEnt((Vect3f){0.0f, 5.0f, 5.0f}, (Vect3f){0.0f, 0.0f, 0.0f}, (Vect3f){0.5f, 0.5f, 0.5f}, 0.5f, 1.8f, 0.56f, 0.08f, 0, OBJECT, entArray3D, allEnts, D_3D);
    }
}

static int init() {
    allPointsCount = 0;
    entAmt = 0;

    objArray3D = pd_malloc( sizeof(Mesh_t) * projDataCount3D);
    entArray3D = pd_malloc(sizeof(VertAnims) * entDataCount3D);
    allObjArray2D = pd_malloc( sizeof(textAnimsAtlas) * (entDataCount2D + projDataCount2D));
    allEnts = pd_malloc(sizeof(EntStruct) * MAX_ENTITIES);

    cam = createCamera(0.0f, 3.0f, 10.0f, 0.0f, 180.0f, 0.0f, 90.0f, 0.1f, 1000.0f);
    player = createEntity(0.0f, 20.0f, -5.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.5f, 1.8f, 0.55f, 0.08f, 0, D_2D);
    // generateEnts();

    convertFileToMesh(mapObjs[mapIndex], &mapArray, mapData[mapIndex][0], mapData[mapIndex][1], 0, (Vect3f){2.0f, 2.0f, 2.0f});

    for (int i=0; i < projDataCount3D; i++) convertFileToMesh(projObjs3D[i], &objArray3D[i], projData3D[i][0], projData3D[i][1], 0, (Vect3f){1.0f, 1.0f, 1.0f});
    for (int i=0; i < entDataCount3D; i++){
        int highest = allocAnimModel(&entArray3D[i], entData3D[i].totalAnims, entData3D[i].animFrameCounts, entData3D[i].animNames, 0, 1, 1, (Vect3f){1.0f, 1.0f, 1.0f});
        allPointsCount += (highest * (entAmt+1));
        entArray3D[i].count = highest;
    }

    for (int i=0; i < entDataCount2D; i++) {
        allocAnimAtlas(&allObjArray2D[i], entData2D[i].totalAnims, entData2D[i].animFrameCounts, entData2D[i].animNames);
        allPointsCount++;
    }

    generateMap(mapArray);
    generateMapTextures(&worldTextAtlasMem, 0);
    generateTriggers((Vect3f){5.0f, 5.0f, 5.0f}, (Vect3f){10.0f, 10.0f, 10.0f});

    resetAllVariables();

    InitAudioManager(&audioManager);
    PlayModuleMusic(&audioManager, "music/adamsoft_-_sonic_trance_remix.mod");

    return 0;
}

static void addPlayer() {
    if (player.type < 0) return;
    if (player.dimention == D_3D && player.type >= entDataCount3D) return;
    if (player.dimention == D_2D && player.type >= entDataCount2D) return;

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
            Vect3f objectPos = {
                FROM_FIXED24_8(player.position.x),
                FROM_FIXED24_8(player.position.y),
                FROM_FIXED24_8(player.position.z)
            };
        
            Vect3f objectRot = {
                FROM_FIXED24_8(player.rotation.x),
                FROM_FIXED24_8(player.rotation.y),
                FROM_FIXED24_8(player.rotation.z)
            };
        
            Vect3f objectSize = {
                FROM_FIXED24_8(player.size.x),
                FROM_FIXED24_8(player.size.y),
                FROM_FIXED24_8(player.size.z)
            };
        
            addObjToWorld3D(
                objectPos, objectRot, objectSize,
                cam, 10.0f,
                model,
                true, 1
            );
        }
    } else if (player.dimention == D_2D) {
        textAtlasFrames* anims = allObjArray2D[player.type].animation[player.currentAnim];
        int newFrame = anims->frames;

        if (player.currentFrame >= newFrame) {
            player.frameCount = 0;
            player.currentFrame = 0;
        }

        Vect3f objectPos = {
            FROM_FIXED24_8(player.position.x),
            FROM_FIXED24_8(player.position.y),
            FROM_FIXED24_8(player.position.z)
        };
    
        Vect3f objectRot = {
            FROM_FIXED24_8(player.rotation.x),
            FROM_FIXED24_8(player.rotation.y),
            FROM_FIXED24_8(player.rotation.z)
        };
    
        Vect3f objectSize = {
            FROM_FIXED24_8(player.size.x),
            FROM_FIXED24_8(player.size.y),
            FROM_FIXED24_8(player.size.z)
        };

        addObjToWorld2D(
            objectPos, objectRot, objectSize,
            cam, 10.0f, 10.0f,
            player.currentAnim, player.currentFrame
        );
    }

    player.lastAnim = player.currentAnim;
    player.frameCount++;
    if (player.frameCount > 4) {
        player.currentFrame++;
        player.frameCount = 0;
    }
}

static void addEntities(int ents, int objs) {
    float dx, dy, dz;
    for (int z = 0; z < entAmt; z++) {
        switch (allEnts[z].type) {
            case ENTITY:
                if (!ents) break;
                EntStruct *ent_ = &allEnts[z].data.ent;

                moveEntObj(ent_, &player);
                if (ent_->type < 0 || ent_->type >= entDataCount3D) break;

                // if (ent_->currentAnim != ent_->lastAnim) {
                //     ent_->frameCount = 0;
                //     ent_->currentFrame = 0;
                // }
            
                AnimFrames* anims = entArray3D[ent_->type].anims[ent_->currentAnim];
                int newFrame = anims->frames;
            
                // if (ent_->currentFrame >= newFrame) {
                //     ent_->frameCount = 0;
                //     ent_->currentFrame = 0;
                // }
            
                Mesh_t model = anims->meshModel[ent_->currentFrame];
                if (model.verts != NULL && model.triCount > 0 && model.bfc != NULL) {
                    Vect3f objectPos = {
                        FROM_FIXED24_8(ent_->position.x),
                        FROM_FIXED24_8(ent_->position.y),
                        FROM_FIXED24_8(ent_->position.z)
                    };

                    Vect3f objectRot = {
                        FROM_FIXED24_8(ent_->rotation.x),
                        FROM_FIXED24_8(ent_->rotation.y),
                        FROM_FIXED24_8(ent_->rotation.z)
                    };

                    Vect3f objectSize = {
                        FROM_FIXED24_8(ent_->size.x),
                        FROM_FIXED24_8(ent_->size.y),
                        FROM_FIXED24_8(ent_->size.z)
                    };

                    addObjToWorld3D(
                        objectPos, objectRot, objectSize,
                        cam, 10.0f,
                        model,
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
                if (!objs) break;
                ObjStruct *obj_ = &allEnts[z].data.obj;
                // objectTypes(obj_);
                
                addObjToWorld3D(
                    (Vect3f){FROM_FIXED24_8(obj_->position.x), FROM_FIXED24_8(obj_->position.y), FROM_FIXED24_8(obj_->position.z)},
                    (Vect3f){FROM_FIXED24_8(obj_->rotation.x), FROM_FIXED24_8(obj_->rotation.y), FROM_FIXED24_8(obj_->rotation.z)},
                    (Vect3f){FROM_FIXED24_8(obj_->size.x), FROM_FIXED24_8(obj_->size.y), FROM_FIXED24_8(obj_->size.z)},
                    cam, 0.0f,
                    objArray3D[obj_->type],
                    false, 1
                );
                break;
        }
    }
}

static void addMap() {
    addObjToWorld3D(
        (Vect3f){0.0f, 0.0f, 0.0f}, (Vect3f){0.0f, 0.0f, 0.0f}, (Vect3f){1.0f, 1.0f, 1.0f},
        cam, 0.0f,
        mapArray,
        false, 1
    );
}

static int render() {
    movePlayerObj(&player, &cam);
    handleCameraInput(&cam);
    updateCamera(&cam, &player, 7.0f);
    
    addMap();
    // addEntities(1, 0);
    addPlayer();

    shootRender(cam, worldTextAtlasMem, allObjArray2D);

    return 0;
}

static inline void scnBufFix() {
    int8_t* p = scnBuf;
    int count = sW_L * sH_L;

    while (count--) {
        *p++ = -1;
    }
}

static int update(void* userdata) {
    if (onStart == 0){
        gameScreen = 1;

        init();
        initDitherByteLUT();
        scnBuf = pd_malloc(sizeof(int8_t) * (sW_L * sH_L));
        onStart = 1;
    }

    float dt = pd->system->getElapsedTime();
    pd->system->resetElapsedTime();
    UpdateAudioManager(&audioManager, dt);

    pd->graphics->clear(kColorBlack);
    buf = pd->graphics->getFrame();
    scnBufFix();
    skybox(5, 10, 10);
    runInputBuffer();

    precomputedFunctions(&cam);

    if (gameScreen == 1) {
        render();

        upscaleToScreen();

        if (inpBuf.B) {
            PlaySFX(&audioManager, "sfx/jump", 1.0f);
        }
    }

    pd->graphics->fillRect(0, 0, 20, 20, kColorWhite);
    pd->system->drawFPS(2, 2);

    return 1;
}