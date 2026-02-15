
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

void generateTextures(textAtlas** textAtlasMem, int memArea) {
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

static void renderToScn(int tri[3][2], clippedTri* clip, textAtlas* textAtlasMem, int t, int color, Dimentions dimention, Edge* edge, float dist, float fov) {
    if (!textAtlasMem || !textAtlasMem[0].pixels) return;
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

        // for (int e = 0; e < 3; e++) {
        //     int t0 = edge->tri0;
        //     int t1 = edge->tri1;

        //     int front0 = allPoints[t0].bfc;
        //     int front1 = (t1 != -1) ? allPoints[t1].bfc : 0;

        //     if (t1 == -1 || front0 != front1)
        //     {
        //         int v0 = e;
        //         int v1 = (e + 1) % 3;

        //         drawLine(tri[v0][0], tri[v0][1], tri[v1][0], tri[v1][1], 0);
        //     }
        // }
    } else if ( dimention == D_2D) {
        drawImg(
            tri[0][0], tri[0][1], dist,
            textAtlasMem[t].pixels, textAtlasMem[t].w, textAtlasMem[t].h,
            fov
        );
    }
}

static void renderStart(Camera_t usedCam, textAtlas* textAtlasMem) {
    float fov = usedCam.fov;
    float nearPlane = usedCam.nearPlane;
    float farPlane = usedCam.farPlane;
    Vect3f camPos = {FROM_FIXED24_8(usedCam.position.x), FROM_FIXED24_8(usedCam.position.y), FROM_FIXED24_8(usedCam.position.z)};

    float camMatrix[3][3];
    computeCamMatrix(camMatrix, FROM_FIXED24_8(usedCam.rotation.x), FROM_FIXED24_8(usedCam.rotation.y), FROM_FIXED24_8(usedCam.rotation.z));

    int tri[3][2];
    int triCheck[3][2];

    clippedTri clipped[2] = {0};

    int triStart = 0;
    if (MAX_TRIS > 0 && allAmt > MAX_TRIS) { triStart = (allAmt - MAX_TRIS); }
    
    for (int index = triStart; index < allAmt; index++) {
        worldTris* src = &allPoints[index];

        Vertex verts[] = {0};
        int t = src->textID;

        if (src->dimentions == D_3D) {
            for (int v = 0; v < 3; v++) {
                verts[v].x = src->verts[v][0];
                verts[v].y = src->verts[v][1];
                verts[v].z = src->verts[v][2];
                verts[v].u = src->uvs[v][0];
                verts[v].v = src->uvs[v][1];

                rotateVertexInPlace(&verts[v], camPos, camMatrix);
            }

            int output = TriangleClipping( verts, &clipped[0], &clipped[1], nearPlane, farPlane );
            if (!output) continue;

            float tmp[3];
            for (int c = 0; c < output; c++) {
                tmp[0] = clipped[c].t1.x; tmp[1] = clipped[c].t1.y; tmp[2] = clipped[c].t1.z; project2D(&tri[0][0], tmp, fov, nearPlane);
                tmp[0] = clipped[c].t2.x; tmp[1] = clipped[c].t2.y; tmp[2] = clipped[c].t2.z; project2D(&tri[1][0], tmp, fov, nearPlane);
                tmp[0] = clipped[c].t3.x; tmp[1] = clipped[c].t3.y; tmp[2] = clipped[c].t3.z; project2D(&tri[2][0], tmp, fov, nearPlane);

                renderToScn(tri, &clipped[c], textAtlasMem, t, src->color, src->dimentions, src->edges, 0.0f, 0.0f);
            }
        } else if (src->dimentions == D_2D) {
            float tmp[3];
            tmp[0] = src->verts[0][0]; tmp[1] = src->verts[0][1]; tmp[2] = src->verts[0][2];

            if (tmp[2] < nearPlane) continue;

            project2D(&tri[0][0], tmp, fov, nearPlane);

            renderToScn(tri, NULL, NULL, t, 0, src->dimentions, NULL, src->dist, fov);
        }
    }
}

void addObjToWorld3D(Vect3f pos, Vect3f rot, Vect3f size, Camera_t cCam, float depthOffset, Mesh_t model, int lineDraw, int distMod) {
    if (allAmt >= allPointsCount) return;

    int renderRadiusNear = cCam.nearPlane;
    int renderRadiusFar  = cCam.farPlane;

    worldTris wTris;
    float renderRadiusSq = renderRadiusFar ? (renderRadiusFar * renderRadiusFar) : 0.0f;
    
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

    Vect3f tv[3];
    Vertex verts[3];
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

            tv[j].x = r[0] + pos.x;
            tv[j].y = r[1] + pos.y;
            tv[j].z = r[2] + pos.z;

            sumX += tv[j].x;
            sumY += tv[j].y;
            sumZ += tv[j].z;

            wTris.verts[j][0] = tv[j].x;
            wTris.verts[j][1] = tv[j].y;
            wTris.verts[j][2] = tv[j].z;

            verts[j].x = tv[j].x;
            verts[j].y = tv[j].y;
            verts[j].z = tv[j].z;

            wTris.edges[j] = edges[base + j];

            rotateVertexInPlace(&verts[j], camPos, camMatrix);
            project2D(&triScn[j][0], (float[3]){verts[j].x, verts[j].y, verts[j].z}, cCam.fov, cCam.nearPlane);
        }

        int bfc = windingOrder(triScn[0], triScn[1], triScn[2]);
        if (backFace[i] && !bfc) continue;

        float cx = sumX * one_third;
        float cy = sumY * one_third;
        float cz = sumZ * one_third;

        float dx = cx - camPos.x;
        float dy = cy - camPos.y;
        float dz = cz - camPos.z;
        float dist = dx*dx + dy*dy + dz*dz;

        if (renderRadiusFar && dist > renderRadiusSq) continue;

        wTris.dimentions = D_3D;
        wTris.color      = colorArray[i];
        wTris.dist       = dist - depthOffset;
        wTris.bfc        = backFace[i];
        wTris.lines      = lineDraw;
        wTris.textID     = -1;

        wTris.uvs[0][0] = 0.0f; wTris.uvs[0][1] = 0.0f;
        wTris.uvs[1][0] = 1.0f; wTris.uvs[1][1] = 0.0f;
        wTris.uvs[2][0] = 0.0f; wTris.uvs[2][1] = 1.0f;

        allPoints[allAmt++] = wTris;
    }
}

void addObjToWorld2D(Vect3f pos, Vect3f rot, Vect3f size, Camera_t cCam, float depthOffset, int textID) {
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
    
    float cx = pos.x * one_third;
    float cy = pos.y * one_third;
    float cz = pos.z * one_third;

    float dx = cx - camPos.x;
    float dy = cy - camPos.y;
    float dz = cz - camPos.z;
    float dist = dx*dx + dy*dy + dz*dz;

    if (renderRadiusFar && dist > renderRadiusSq) return;

    wTris.dimentions = D_2D;
    wTris.dist       = dist - depthOffset;
    wTris.textID     = 0;

    wTris.color      = 0;
    wTris.bfc        = -1;
    wTris.lines      = -1;

    allPoints[allAmt++] = wTris;
}

void shootRender(Camera_t cam, textAtlas* textAtlasMem) {
    qsort(allPoints, allAmt, sizeof(worldTris), compareRenderTris);

    renderStart(cam, textAtlasMem);
    allAmt = 0;
}

void resetAllVariables() {
    resShiftFix();
    
    allPoints = pd_realloc(allPoints, allPointsCount * sizeof(worldTris));
    allAmt = 0;
}