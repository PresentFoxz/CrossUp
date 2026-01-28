
#include "libEngine.h"

worldTris* allPoints;
int renderRadius = 85;
int entAmt = 0;
int allAmt = 0;
int mapIndex = 0;
int entIndex = 0;
int allPointsCount = 0;
int* scnBuf;


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
    resetCollisionSurface();
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
    *textAtlasMem = realloc(*textAtlasMem, sizeof(textAtlas) * 1);
    if (!*textAtlasMem) return;
    (*textAtlasMem)[memArea] = testTexture;
}

static int compareRenderTris(const void* a, const void* b) {
    float d1 = ((worldTris*)a)->dist;
    float d2 = ((worldTris*)b)->dist;
    return (d1 < d2) - (d1 > d2);
}

static void renderTris(float CamYDirSin, float CamYDirCos, float CamXDirSin, float CamXDirCos, float CamZDirSin, float CamZDirCos, Camera_t usedCam, textAtlas* textAtlasMem) {
    float fov = FROM_FIXED32(usedCam.fov);
    float nearPlane = FROM_FIXED32(usedCam.nearPlane);
    float farPlane = FROM_FIXED32(usedCam.farPlane);
    Vect3f camPos = {FROM_FIXED32(usedCam.position.x), FROM_FIXED32(usedCam.position.y), FROM_FIXED32(usedCam.position.z)};

    int tri1[3][2];
    int tri2[3][2];
    int check[3][2];
    Vertex verts[3];
    clippedTri clip1, clip2;

    float camMatrix[3][3];
    computeCamMatrix(camMatrix, CamYDirSin, CamYDirCos, CamXDirSin, CamXDirCos, CamZDirSin, CamZDirCos);

    for (int index = 0; index < allAmt; index++){
        int color = allPoints[index].color;
        
        for (int v = 0; v < 3; v++) {
            verts[v].x = allPoints[index].verts[v][0];
            verts[v].y = allPoints[index].verts[v][1];
            verts[v].z = allPoints[index].verts[v][2];

            verts[v].u = allPoints[index].uvs[v][0];
            verts[v].v = allPoints[index].uvs[v][1];
            
            rotateVertexInPlace(&verts[v], camPos, camMatrix);
            project2D(&check[v][0], (float[]){verts[v].x, verts[v].y, verts[v].z}, fov, nearPlane);
        }

        if (allPoints[index].bfc == 1 && !windingOrder(check[0], check[1], check[2])) { continue; }
        if (verts[0].z <= 0.0f && verts[1].z <= 0.0f && verts[2].z <= 0.0f) { continue; }

        int output = TriangleClipping(verts, &clip1, &clip2, nearPlane, farPlane);
        if (output) {
            project2D(&tri1[0][0], (float[3]){clip1.t1.x, clip1.t1.y, clip1.t1.z}, fov, nearPlane);
            project2D(&tri1[1][0], (float[3]){clip1.t2.x, clip1.t2.y, clip1.t2.z}, fov, nearPlane);
            project2D(&tri1[2][0], (float[3]){clip1.t3.x, clip1.t3.y, clip1.t3.z}, fov, nearPlane);
            
            Vect3f dither1 = {clip1.t1.z, clip1.t2.z, clip1.t3.z};
            drawTexturedTris(tri1, (float[3][2]){ {clip1.t1.u, clip1.t1.v}, {clip1.t2.u, clip1.t2.v}, {clip1.t3.u, clip1.t3.v} }, textAtlasMem[0].pixels, textAtlasMem[0].w, textAtlasMem[0].h, dither1 );
            // drawFilledTris(tri1, color, dither1);
            // if (allPoints[index].lines == 1) drawTriLines(tri1);
            
            if (output == 2){
                project2D(&tri2[0][0], (float[3]){clip2.t1.x, clip2.t1.y, clip2.t1.z}, fov, nearPlane); 
                project2D(&tri2[1][0], (float[3]){clip2.t2.x, clip2.t2.y, clip2.t2.z}, fov, nearPlane);
                project2D(&tri2[2][0], (float[3]){clip2.t3.x, clip2.t3.y, clip2.t3.z}, fov, nearPlane);
                
                Vect3f dither2 = {clip2.t1.z, clip2.t2.z, clip2.t3.z};
                drawTexturedTris(tri2, (float[3][2]){ {clip2.t1.u, clip2.t1.v}, {clip2.t2.u, clip2.t2.v}, {clip2.t3.u, clip2.t3.v} }, textAtlasMem[0].pixels, textAtlasMem[0].w, textAtlasMem[0].h, dither2 );
                // drawFilledTris(tri2, color, dither2);
                // if (allPoints[index].lines == 1) drawTriLines(tri2);
            }
        }
    }
}

void addObjectToWorld(Vect3f pos, Vect3f rot, Vect3f size, Camera_t cCam, float depthOffset, int triCount, Vect3f* data, int* backFace, int* colorArray, int lineDraw, int distMod) {
    if (allAmt >= allPointsCount) return;

    worldTris tri;
    float rotMat[3][3];
    computeRotScaleMatrix(rotMat, rot.x, rot.y, rot.z, size.x, size.y, size.z);

    float camX = FROM_FIXED32(cCam.position.x);
    float camY = FROM_FIXED32(cCam.position.y);
    float camZ = FROM_FIXED32(cCam.position.z);
    float renderRadiusSq = renderRadius ? (renderRadius * renderRadius) : 0.0f;
    const float one_third = 0.3333333f;

    Vect3f* transformedVerts = realloc(NULL, sizeof(Vect3f) * triCount * 3);

    for (int v = 0; v < triCount * 3; v++) {
        float r[3];
        rotateVertex(data[v].x, data[v].y, data[v].z, rotMat, r);
        transformedVerts[v].x = r[0] + pos.x;
        transformedVerts[v].y = r[1] + pos.y;
        transformedVerts[v].z = r[2] + pos.z;
    }

    for (int i = 0; i < triCount; i++) {
        float sumX = 0, sumY = 0, sumZ = 0;

        for (int j = 0; j < 3; j++) {
            Vect3f v = transformedVerts[i * 3 + j];
            tri.verts[j][0] = v.x;
            tri.verts[j][1] = v.y;
            tri.verts[j][2] = v.z;

            sumX += v.x;
            sumY += v.y;
            sumZ += v.z;
        }

        float cx = sumX * one_third;
        float cy = sumY * one_third;
        float cz = sumZ * one_third;
    
        float dx = cx - camX;
        float dy = cy - camY;
        float dz = cz - camZ;
        float dist = dx*dx + dy*dy + dz*dz;

        if (distMod == 1) {
            if (renderRadius && dist > renderRadiusSq) continue;
            if (dist <= 1e-8f) continue;
        }

        tri.color = colorArray[i];
        tri.dist = dist - depthOffset;
        tri.bfc = backFace[i];
        tri.lines = lineDraw;

        tri.uvs[0][0] = 0.0f; tri.uvs[0][1] = 0.0f;
        tri.uvs[1][0] = 1.0f; tri.uvs[1][1] = 0.0f;
        tri.uvs[2][0] = 0.0f; tri.uvs[2][1] = 1.0f;

        if (allAmt >= allPointsCount) return;
        allPoints[allAmt++] = tri;
    }

    transformedVerts = realloc(transformedVerts, sizeof(Vect3f) * 0);
}

void shootRender(float CamYDirSin, float CamYDirCos, float CamXDirSin, float CamXDirCos, float CamZDirSin, float CamZDirCos, Camera_t cam, textAtlas* textAtlasMem) {
    qsort(allPoints, allAmt, sizeof(worldTris), compareRenderTris);
    renderTris(CamYDirSin, CamYDirCos, CamXDirSin, CamXDirCos, CamZDirSin, CamZDirCos, cam, textAtlasMem);
    allAmt = 0;
}

void resetAllVariables() {
    printf("All Points Count: %d", allPointsCount);
    allPoints = realloc(allPoints, allPointsCount * sizeof(worldTris));
    allAmt = 0;
}