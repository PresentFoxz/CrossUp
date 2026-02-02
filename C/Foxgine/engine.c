
#include "libEngine.h"

worldTris* allPoints;
int chnkAmt = 0;

int renderRadius = 85;
int entAmt = 0;
int mapIndex = 0;
int allPointsCount = 0;
int allAmt = 0;

void addEnt(Vect3f pos, Vect3f rot, Vect3f size, float radius, float height, float frict, float fallFrict, int type, ModelType objType, VertAnims* entArray, Objects* allEnts) {
    if (entAmt < MAX_ENTITIES) {
        allEnts[entAmt].type = objType;

        switch(objType) {
            case ENTITY:
                allEnts[entAmt].data.ent = createEntity(pos.x, pos.y, pos.z, rot.x, rot.y, rot.z, size.x, size.y, size.z, radius, height, frict, fallFrict, type);
                break;
            case OBJECT:
                allEnts[entAmt].data.obj = createObject(pos.x, pos.y, pos.z, rot.x, rot.y, rot.z, size.x, size.y, size.z, type, 1000);
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
    for (int i=0; i < mapArray.count; i++){
        int idx[3] = {(i*3), (i*3)+1, (i*3)+2};
        addCollisionSurface(
            (Vect3f){mapArray.data[idx[0]].x, mapArray.data[idx[0]].y, mapArray.data[idx[0]].z},
            (Vect3f){mapArray.data[idx[1]].x, mapArray.data[idx[1]].y, mapArray.data[idx[1]].z},
            (Vect3f){mapArray.data[idx[2]].x, mapArray.data[idx[2]].y, mapArray.data[idx[2]].z},
            SURFACE_NONE
        );
    }

    allPointsCount += mapArray.count;
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
    float d = ((worldTris*)b)->dist - ((worldTris*)a)->dist;
    return (d > 0) - (d < 0);
}

static void renderTriData(int tri[3][2], clippedTri clip, textAtlas* textAtlasMem, int t, int color) {
    if (!textAtlasMem || !textAtlasMem[0].pixels) return;

    if (t != -1) {
        drawTexturedTris(
            tri,
            (float[3][2]){{clip.t1.u, clip.t1.v}, {clip.t2.u, clip.t2.v}, {clip.t3.u, clip.t3.v}},
            textAtlasMem[t].pixels,
            textAtlasMem[t].w,
            textAtlasMem[t].h
        );
    } else {
        drawFilledTris(tri, color);
    }
}

static void renderTris(Camera_t usedCam, textAtlas* textAtlasMem) {
    float fov = usedCam.fov;
    float nearPlane = usedCam.nearPlane;
    float farPlane = usedCam.farPlane;
    Vect3f camPos = {FROM_FIXED32(usedCam.position.x), FROM_FIXED32(usedCam.position.y), FROM_FIXED32(usedCam.position.z)};

    float camMatrix[3][3];
    computeCamMatrix(camMatrix, FROM_FIXED32(usedCam.rotation.x), FROM_FIXED32(usedCam.rotation.y), FROM_FIXED32(usedCam.rotation.z));

    int tri1[3][2];
    int tri2[3][2];
    int triCheck[3][2];

    clippedTri clip1, clip2;

    int triStart = 0;
    if (MAX_TRIS > 0 && allAmt > MAX_TRIS) { triStart = (allAmt - MAX_TRIS); }

    for (int index = triStart; index < allAmt; index++) {
        worldTris* src = &allPoints[index];
        int color = src->color;

        Vertex verts[3];
        int t = src->textID;
            
        for (int v = 0; v < 3; v++) {
            verts[v].x = src->verts[v][0];
            verts[v].y = src->verts[v][1];
            verts[v].z = src->verts[v][2];
            verts[v].u = src->uvs[v][0];
            verts[v].v = src->uvs[v][1];

            rotateVertexInPlace(&verts[v], camPos, camMatrix);
        }

        if (verts[0].z <= 0.0f && verts[1].z <= 0.0f && verts[2].z <= 0.0f) continue;
        int output = TriangleClipping( verts, &clip1, &clip2, nearPlane, farPlane );
        if (!output) continue;
        
        project2D(&tri1[0][0], (float[3]){clip1.t1.x, clip1.t1.y, clip1.t1.z}, fov, nearPlane);
        project2D(&tri1[1][0], (float[3]){clip1.t2.x, clip1.t2.y, clip1.t2.z}, fov, nearPlane);
        project2D(&tri1[2][0], (float[3]){clip1.t3.x, clip1.t3.y, clip1.t3.z}, fov, nearPlane);

        renderTriData(tri1, clip1, textAtlasMem, t, color);
        
        if (output == 2) {
            project2D(&tri2[0][0], (float[3]){clip2.t1.x, clip2.t1.y, clip2.t1.z}, fov, nearPlane);
            project2D(&tri2[1][0], (float[3]){clip2.t2.x, clip2.t2.y, clip2.t2.z}, fov, nearPlane);
            project2D(&tri2[2][0], (float[3]){clip2.t3.x, clip2.t3.y, clip2.t3.z}, fov, nearPlane);

            renderTriData(tri2, clip2, textAtlasMem, t, color);
        }
    }
}

void addObjectToWorld(Vect3f pos, Vect3f rot, Vect3f size, Camera_t cCam, float depthOffset, int triCount, Vect3f* data, int* backFace, int* colorArray, int flipped, int outline, int lineDraw, int distMod) {
    if (allAmt >= allPointsCount) return;

    worldTris tri;
    float rotMat[3][3];
    computeRotScaleMatrix(rotMat, rot.x, rot.y, rot.z, size.x, size.y, size.z);

    float renderRadiusSq = renderRadius ? (renderRadius * renderRadius) : 0.0f;
    const float one_third = 0.3333333f;

    float camMatrix[3][3];
    Vect3f camPos = {FROM_FIXED32(cCam.position.x), FROM_FIXED32(cCam.position.y), FROM_FIXED32(cCam.position.z)};
    Vect3f camRot = {FROM_FIXED32(cCam.rotation.x), FROM_FIXED32(cCam.rotation.y), FROM_FIXED32(cCam.rotation.z)};
    computeCamMatrix(camMatrix, camRot.x, camRot.y, camRot.z);

    for (int i = 0; i < triCount; i++) {
        if (allAmt >= allPointsCount) return;

        Vect3f tv[3];
        Vertex verts[3];
        int triScn[3][2];
        float sumX = 0.0f, sumY = 0.0f, sumZ = 0.0f;
        for (int j = 0; j < 3; j++) {
            int idx = i * 3 + j;
            float r[3];

            rotateVertex(
                data[idx].x,
                data[idx].y,
                data[idx].z,
                rotMat,
                r
            );

            tv[j].x = r[0] + pos.x;
            tv[j].y = r[1] + pos.y;
            tv[j].z = r[2] + pos.z;

            sumX += tv[j].x;
            sumY += tv[j].y;
            sumZ += tv[j].z;

            tri.verts[j][0] = tv[j].x;
            tri.verts[j][1] = tv[j].y;
            tri.verts[j][2] = tv[j].z;

            verts[j].x = tv[j].x;
            verts[j].y = tv[j].y;
            verts[j].z = tv[j].z;

            rotateVertexInPlace(&verts[j], camPos, camMatrix);
            project2D(&triScn[j][0], (float[3]){verts[j].x, verts[j].y, verts[j].z}, cCam.fov, cCam.nearPlane);
        }

        int bfc = windingOrder(triScn[0], triScn[1], triScn[2]);
        if (outline && !bfc) {
            worldTris outTri = tri;
            float outScale = 0.2f;

            float outMat[3][3];
            computeRotScaleMatrix(outMat, rot.x, rot.y, rot.z, size.x + outScale, size.y + outScale, size.z + outScale);

            Vect3f tt[3];
            for (int j = 0; j < 3; j++) {
                int idx = i * 3 + j;
                float r[3];

                rotateVertex(
                    data[idx].x,
                    data[idx].y,
                    data[idx].z,
                    outMat,
                    r
                );

                tt[j].x = r[0] + pos.x;
                tt[j].y = r[1] + pos.y;
                tt[j].z = r[2] + pos.z;

                outTri.verts[j][0] = tt[j].x;
                outTri.verts[j][1] = tt[j].y - outScale;
                outTri.verts[j][2] = tt[j].z;
            }

            outTri.color = 3;
            outTri.lines = 0;
            outTri.textID = -1;
            allPoints[allAmt++] = outTri;
        } if (backFace[i] && !bfc) continue;
        
        float cx = sumX * one_third;
        float cy = sumY * one_third;
        float cz = sumZ * one_third;

        float dx = cx - camPos.x;
        float dy = cy - camPos.y;
        float dz = cz - camPos.z;
        float dist = dx*dx + dy*dy + dz*dz;

        if (renderRadius && dist > renderRadiusSq) continue;
        
        tri.color = colorArray[i];
        tri.dist  = dist - depthOffset;
        tri.bfc   = backFace[i];
        tri.lines = lineDraw;
        tri.textID = -1;

        tri.uvs[0][0] = 0.0f; tri.uvs[0][1] = 0.0f;
        tri.uvs[1][0] = 1.0f; tri.uvs[1][1] = 0.0f;
        tri.uvs[2][0] = 0.0f; tri.uvs[2][1] = 1.0f;

        allPoints[allAmt++] = tri;
    }
}

void shootRender(Camera_t cam, textAtlas* textAtlasMem) {
    qsort(allPoints, allAmt, sizeof(worldTris), compareRenderTris);

    renderTris(cam, textAtlasMem);
    allAmt = 0;
}

void resetAllVariables() {
    allPoints = pd_realloc(allPoints, allPointsCount * sizeof(worldTris));
    allAmt = 0;
}