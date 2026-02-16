
#include "libEngine.h"

worldTris* allPoints;
int chnkAmt = 0;

const float one_third = 0.3333333f;
int entAmt = 0;
int mapIndex = 0;
int allPointsCount = 0;
int allAmt = 0;

void addEnt(Vect3f pos, Vect3f rot, Vect3f size, float radius, float height, float frict, float fallFrict, int type, ModelType objType, VertAnims* entArray, Objects* allEnts, Dimentions dimention) {
    if (entAmt < MAX_ENTITIES) {
        allEnts[entAmt].type = objType;

        switch(objType) {
            case ENTITY:
                allEnts[entAmt].data.ent = createEntity(pos.x, pos.y, pos.z, rot.x, rot.y, rot.z, size.x, size.y, size.z, radius, height, frict, fallFrict, type, dimention);
                break;
            case OBJECT:
                allEnts[entAmt].data.obj = createObject(pos.x, pos.y, pos.z, rot.x, rot.y, rot.z, size.x, size.y, size.z, type, 1000, dimention);
                break;
        }
        entAmt++;

        if (objType != OBJECT) {
            int newPoint = (allPointsCount + entArray[type].count);
            if (newPoint > allPointsCount) { allPointsCount = (newPoint + 50); }
        }
    }
}

void generateMap(Mesh_t mapArray) {
    resetCollisionSurface(mapArray); 
    for (int i=0; i < mapArray.triCount; i++){
        int* triNum = mapArray.tris[i];
        addCollisionSurface(
            (Vect3f){mapArray.verts[triNum[0]].x, mapArray.verts[triNum[0]].y, mapArray.verts[triNum[0]].z},
            (Vect3f){mapArray.verts[triNum[1]].x, mapArray.verts[triNum[1]].y, mapArray.verts[triNum[1]].z},
            (Vect3f){mapArray.verts[triNum[2]].x, mapArray.verts[triNum[2]].y, mapArray.verts[triNum[2]].z},
            SURFACE_NONE
        );
    }

    allPointsCount += mapArray.triCount;
}

void generateTriggers(Vect3f pos, Vect3f size) {
    resetTriggers();
    addTriggers(pos, size, 1, 2);
}

void generateMapTextures(textAtlas** textAtlasMem, int memArea) {
    *textAtlasMem = pd_realloc(*textAtlasMem, sizeof(textAtlas) * 1);
    if (!*textAtlasMem) return;
    (*textAtlasMem)[memArea] = testTexture;
}

static inline int compareRenderTris(const void* a, const void* b) {
    int da = ((worldTris*)a)->dist;
    int db = ((worldTris*)b)->dist;

    if (da < db) return 1;
    if (da > db) return -1;
    return 0;
}

static void renderToScn(int tri[3][2], clippedTri* clip, textAtlas* textAtlasMem, int t, int color, Dimentions dimention, Edge* edge, float dist, float fov, float nearPlane, float farPlane) {
    if (dimention == D_3D) {
        if (t != -1) {
            drawTexturedTris(
                tri,
                (float[3][2]){{clip->t1.u, clip->t1.v}, {clip->t2.u, clip->t2.v}, {clip->t3.u, clip->t3.v}},
                textAtlasMem[t].pixels,
                textAtlasMem[t].w,
                textAtlasMem[t].h
            );
        } else {
            drawFilledTris(tri, color);
        }
    } else if (dimention == D_2D) {
        drawImg(tri[0][0], tri[0][1], dist, 0, 0, 1024, 1024, textAtlasMem->pixels, textAtlasMem->w, textAtlasMem->h, fov, nearPlane);
        // drawImgNoScale(tri[0][0], tri[0][1], 0, 0, 9, 9, textAtlasMem->pixels, textAtlasMem->w, textAtlasMem->h);
    }
}

static void renderStart(Camera_t usedCam, textAtlas* worldTextAtlasMem, textAnimsAtlas* allObjArray2D) {
    float fov = usedCam.fov;
    float nearPlane = usedCam.nearPlane;
    float farPlane = usedCam.farPlane;
    Vect3f camPos = {FROM_FIXED24_8(usedCam.position.x), FROM_FIXED24_8(usedCam.position.y), FROM_FIXED24_8(usedCam.position.z)};
    clippedTri clipped[2] = {0};

    float camMatrix[3][3];
    computeCamMatrix(camMatrix, FROM_FIXED24_8(usedCam.rotation.x), FROM_FIXED24_8(usedCam.rotation.y), FROM_FIXED24_8(usedCam.rotation.z));

    int tri[3][2];
    int triStart = 0;
    if (MAX_TRIS > 0 && allAmt > MAX_TRIS) { triStart = (allAmt - MAX_TRIS); }
    for (int index = triStart; index < allAmt; index++) {
        worldTris* src = &allPoints[index];
        int t = src->textID;

        if (src->dimentions == D_3D) {
            int output = TriangleClipping( src->verts, &clipped[0], &clipped[1], nearPlane, farPlane );
            if (!output) continue;

            float tmp[3];
            for (int c = 0; c < output; c++) {
                tmp[0] = clipped[c].t1.x; tmp[1] = clipped[c].t1.y; tmp[2] = clipped[c].t1.z; project2D(&tri[0][0], tmp, fov, nearPlane);
                tmp[0] = clipped[c].t2.x; tmp[1] = clipped[c].t2.y; tmp[2] = clipped[c].t2.z; project2D(&tri[1][0], tmp, fov, nearPlane);
                tmp[0] = clipped[c].t3.x; tmp[1] = clipped[c].t3.y; tmp[2] = clipped[c].t3.z; project2D(&tri[2][0], tmp, fov, nearPlane);

                renderToScn(tri, &clipped[c], worldTextAtlasMem, t, src->color, src->dimentions, src->edges, src->dist, fov, nearPlane, farPlane);
            }
        } else if (src->dimentions == D_2D) {
            float tmp[3];
            rotateVertexInPlace(&src->verts[0], camPos, camMatrix);

            tmp[0] = src->verts[0].x; tmp[1] = src->verts[0].y; tmp[2] = src->verts[0].z;
            if (tmp[2] < nearPlane || tmp[2] > farPlane) continue;

            project2D(&tri[0][0], tmp, fov, nearPlane);
            renderToScn(tri, NULL, &allObjArray2D->animation[t]->animData, t, 0, src->dimentions, NULL, src->lines, fov, nearPlane, farPlane);
        }
    }
}

void addObjToWorld3D(Vect3f pos, Vect3f rot, Vect3f size, Camera_t cCam, float depthOffset, Mesh_t model, int lineDraw, int distMod) {
    if (allAmt >= allPointsCount) return;

    worldTris wTris;
    float renderRadiusSq = cCam.farPlane ? (cCam.farPlane * cCam.farPlane) : 0.0f;
    
    float camMatrix[3][3];
    Vect3f camPos = {FROM_FIXED24_8(cCam.position.x), FROM_FIXED24_8(cCam.position.y), FROM_FIXED24_8(cCam.position.z)};
    Vect3f camRot = {FROM_FIXED24_8(cCam.rotation.x), FROM_FIXED24_8(cCam.rotation.y), FROM_FIXED24_8(cCam.rotation.z)};
    computeCamMatrix(camMatrix, camRot.x, camRot.y, camRot.z);

    int vertCount = model.vertCount;
    int triCount = model.triCount;
    Vect3f* verticies = model.verts;
    int (*tris)[3] = model.tris;
    Edge* edges = model.edges;
    int* backFace = model.bfc;
    int* colorArray = model.color;
    int flipped = model.flipped;
    int outline = model.outline;

    float rotMat[3][3];
    computeRotScaleMatrix(rotMat, rot.x, rot.y, rot.z, size.x, size.y, size.z);

    int triScn[3][2];
    for (int i = 0; i < triCount; i++) {
        if (allAmt >= allPointsCount) return;

        int base = i * 3;
        float sumX = 0.0f, sumY = 0.0f, sumZ = 0.0f;
        for (int j = 0; j < 3; j++) {
            int idx = tris[i][j];
            float r[3];

            rotateVertex(
                verticies[idx].x,
                verticies[idx].y,
                verticies[idx].z,
                rotMat,
                r
            );

            wTris.verts[j].x = r[0] + pos.x;
            wTris.verts[j].y = r[1] + pos.y;
            wTris.verts[j].z = r[2] + pos.z;
            wTris.edges[j] = edges[base + j];

            sumX += wTris.verts[j].x;
            sumY += wTris.verts[j].y;
            sumZ += wTris.verts[j].z;

            rotateVertexInPlace(&wTris.verts[j], camPos, camMatrix);
            project2D(&triScn[j][0], (float[3]){wTris.verts[j].x, wTris.verts[j].y, wTris.verts[j].z}, cCam.fov, cCam.nearPlane);
        }

        int bfc = windingOrder(triScn[0], triScn[1], triScn[2]);
        if (backFace[i] && !bfc) continue;

        float cx = sumX * one_third;
        float cy = sumY * one_third;
        float cz = sumZ * one_third;

        float dx = cx - camPos.x;
        float dy = cy - camPos.y;
        float dz = cz - camPos.z;
        float dist = sqrtf(dx*dx + dy*dy + dz*dz);

        if (cCam.farPlane && dist > renderRadiusSq) continue;

        wTris.dimentions = D_3D;
        wTris.color      = colorArray[i];
        wTris.dist       = dist - depthOffset;
        wTris.lines      = lineDraw;
        wTris.textID     = -1;

        wTris.verts[0].u = 0.0f; wTris.verts[0].v = 0.0f;
        wTris.verts[1].u = 1.0f; wTris.verts[1].v = 0.0f;
        wTris.verts[2].u = 0.0f; wTris.verts[2].v = 1.0f;

        allPoints[allAmt++] = wTris;
    }
}

void addObjToWorld2D(Vect3f pos, Vect3f rot, Vect3f size, Camera_t cCam, float objDepthOffset, float sprtDepthOffset, int anim, int animFrame) {
    if (allAmt >= allPointsCount) return;

    int renderRadiusNear = cCam.nearPlane;
    int renderRadiusFar  = cCam.farPlane;

    worldTris wTris;
    float renderRadiusSq = renderRadiusFar ? (renderRadiusFar * renderRadiusFar) : 0.0f;
    
    float camMatrix[3][3];
    Vect3f camPos = {FROM_FIXED24_8(cCam.position.x), FROM_FIXED24_8(cCam.position.y), FROM_FIXED24_8(cCam.position.z)};
    Vect3f camRot = {FROM_FIXED24_8(cCam.rotation.x), FROM_FIXED24_8(cCam.rotation.y), FROM_FIXED24_8(cCam.rotation.z)};
    computeCamMatrix(camMatrix, camRot.x, camRot.y, camRot.z);

    float rotMat[3][3];
    computeRotScaleMatrix(rotMat, rot.x, rot.y, rot.z, size.x, size.y, size.z);

    Vect3f tv[3];
    Vertex verts[3];
    int triScn[3][2];

    float dx = pos.x - camPos.x;
    float dy = (pos.y - 5.0f) - camPos.y;
    float dz = pos.z - camPos.z;
    float dist = sqrtf(dx*dx + dy*dy + dz*dz);

    if (renderRadiusFar && dist > renderRadiusSq) return;

    wTris.verts[0].x = pos.x; wTris.verts[0].y = pos.y; wTris.verts[0].z = pos.z;
    wTris.dimentions = D_2D;
    wTris.dist       = dist - objDepthOffset;
    wTris.lines      = dist + sprtDepthOffset;
    wTris.textID     = anim;
    wTris.color      = animFrame;

    allPoints[allAmt++] = wTris;
}

void shootRender(Camera_t cam, textAtlas* worldTextAtlasMem, textAnimsAtlas* allObjArray2D) {
    qsort(allPoints, allAmt, sizeof(worldTris), compareRenderTris);

    renderStart(cam, worldTextAtlasMem, allObjArray2D);
    allAmt = 0;
}

void resetAllVariables() {
    resShiftFix();
    
    allPoints = pd_realloc(allPoints, allPointsCount * sizeof(worldTris));
    allAmt = 0;
}