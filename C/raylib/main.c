#include "libRay.h"
#include "movement.h"
#include "meshConvert.h"

#include "../Foxgine/engine.h"

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

int freeFly = 0;

void generateEnts() {
    if (mapIndex == 0) {
        addEnt((Vect3f){0.0f, 20.0f, -5.0f}, (Vect3f){0.0f, 0.0f, 0.0f}, (Vect3f){1.0f, 1.0f, 1.0f}, 0.5f, 1.8f, 0.56f, 0.08f, 0, ENTITY, entArray, allEnts);
        addEnt((Vect3f){0.0f, 5.0f, 5.0f}, (Vect3f){0.0f, 0.0f, 0.0f}, (Vect3f){0.5f, 0.5f, 0.5f}, 0.5f, 1.8f, 0.56f, 0.08f, 0, OBJECT, entArray, allEnts);
    } else if (mapIndex == 1) {
        addEnt((Vect3f){0.0f, 5.0f, 5.0f}, (Vect3f){0.0f, 0.0f, 0.0f}, (Vect3f){0.5f, 0.5f, 0.5f}, 0.5f, 1.8f, 0.56f, 0.08f, 0, OBJECT, entArray, allEnts);
    }
}

static int init() {
    allPointsCount = 0;
    entAmt = 0;

    cam = createCamera(0.0f, 3.0f, 10.0f, 0.0f, 180.0f, 0.0f, 90.0f, 0.1f, 100.0f);
    player = createEntity(0.0f, 20.0f, -5.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.5f, 1.8f, 0.55f, 0.08f, 0);

    objArray = pd_malloc( sizeof(Mesh_t) * maxProjs);
    entArray = pd_malloc(sizeof(VertAnims) * entDataCount);
    allEnts = pd_malloc(sizeof(EntStruct) * MAX_ENTITIES);

    convertFileToMesh(mapObjs[mapIndex], &mapArray, mapData[mapIndex][0], mapData[mapIndex][1], 0);
    convertFileToMesh("Objects/proj/ball.obj", &objArray[0], 0, 0, 0);

    for (int i=0; i < entDataCount; i++){
        int highest = allocAnimModel(&entArray[i], entData[i].totalAnims, entData[i].animFrameCounts, entData[i].animNames, 0, 1, 1);
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
            model.flipped,
            model.outline,
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

                moveEntObj(ent_, &player);
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
                        model.flipped,
                        model.outline,
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
                    objArray[obj_->type].flipped,
                    objArray[obj_->type].outline,
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
        mapArray.flipped,
        mapArray.outline,
        false, 1
    );
}

static int render(int camType) {
    movePlayerObj(&player, &cam, camType);
    if (camType){
        handleCameraInput(&cam);
        updateCamera(&cam, &player, 7.0f);
    } else {
        handleCameraFreeInput(&cam);
    }

    addMap();
    addPlayer();
    // addEntities();

    shootRender(cam, textAtlasMem);
    drawScreen();

    return 0;
}

static inline void scnBufFix() {
    int8_t* p = scnBuf;
    int count = sW_L * sH_L;

    while (count--) {
        *p++ = -1;
    }
}

int main() {
    InitWindow(sW, sH, "CrossUp");
    SetTargetFPS(30);

    gameScreen = 1;
    freeFly = 0;

    init();
    scnBuf = pd_malloc(sizeof(int8_t) * (sW_L * sH_L));

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        scnBufFix();
        skybox(5, 10, 10);

        if (gameScreen == 1) {
            render(freeFly);

            if (IsKeyPressed(KEY_M)) {
                mapIndex++;
                if (mapIndex >= mapObjsCount) mapIndex = 0;
                init();
            }

            if (IsKeyPressed(KEY_N)) {
                freeFly++;

                if (freeFly > 1) freeFly = 0;
            }
        }

        drawScreen();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}