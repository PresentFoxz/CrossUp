
#include "libEngine.h"

worldTris* allPoints;
TriangleOrdering* triOrder;
int* triFacing;
int chnkAmt = 0;

const float one_third = 0.3333333f;
int entAmt = 0;
int allPointsCount = 0;
int allAmt = 0;

Light_t* lightSource;
int lightAmt = 0;

void addLightPoint(Vector3f pos, uint8_t power, float falloff) {
    lightSource = pd_realloc(lightSource, sizeof(Light_t) * (lightAmt + 1));
    lightSource[lightAmt++] = (Light_t){ .pos = pos, .power = power, .falloff = falloff };
}

void generateMap(Mesh_t mapArray, Vector3f pos) {
    fixSurfaces(mapArray, (Vector2f){pos.x, pos.z});
    collisionChunks();

    for (int i=0; i < mapArray.triCount; i++){
        int* triNum = mapArray.tris[i];
        addCollisionSurface(
            (Vector3f){mapArray.verts[triNum[0]].x + pos.x, mapArray.verts[triNum[0]].y + pos.y, mapArray.verts[triNum[0]].z + pos.z},
            (Vector3f){mapArray.verts[triNum[1]].x + pos.x, mapArray.verts[triNum[1]].y + pos.y, mapArray.verts[triNum[1]].z + pos.z},
            (Vector3f){mapArray.verts[triNum[2]].x + pos.x, mapArray.verts[triNum[2]].y + pos.y, mapArray.verts[triNum[2]].z + pos.z},
            mapArray.normal[i], SURFACE_NONE
        );
    }

    allPointsCount += mapArray.triCount;
}

void generateTriggers(Vector3f pos, Vector3f size) {
    resetTriggers();
    addTriggers(pos, size, 1, 2);
}

static void quickSortIndices(TriangleOrdering* order, int left, int right) {
    if (left >= right) return;

    float pivot = order[(left + right) >> 1].dist;
    int i = left;
    int j = right;

    while (i <= j) {
        while (order[i].dist > pivot) i++;
        while (order[j].dist < pivot) j--;

        if (i <= j) {
            TriangleOrdering tmp = order[i];
            order[i] = order[j];
            order[j] = tmp;
            i++;
            j--;
        }
    }

    if (left < j) quickSortIndices(order, left, j);
    if (i < right) quickSortIndices(order, i, right);
}

uint8_t calculateLightnessValue(Vector3f tri, Light_t light, Vector3f normal) {
    float dx = light.pos.x - tri.x;
    float dy = light.pos.y - tri.y;
    float dz = light.pos.z - tri.z;

    float len = sqrtf(dx*dx + dy*dy + dz*dz);
    if (len <= 0.00001f) return 0;

    float attenuation = 1.0f - (len / light.falloff);
    if (attenuation <= 0.0f) return 0;

    dx /= len;
    dy /= len;
    dz /= len;

    float dot = normal.x*dx + normal.y*dy + normal.z*dz;
    if (dot <= 0.0f) return 0;

    float val = dot * light.power * attenuation;

    if (val >= 255.0f) val = 254.0f;
    if (val < 0.0f)   val = 0.0f;
    return (uint8_t)val;
}

uint8_t getBrightness(Vector3f tri, Light_t* lights, Vector3f normal, uint8_t color) {
    int brightness = ambientLight;
    for (int i = 0; i < lightAmt; i++) { brightness += calculateLightnessValue(tri, lights[i], normal); }

    brightness = (brightness * color) / 255;
    if (brightness >= 255) brightness = 254;
    if (brightness < 0)    brightness = 0;
    return (uint8_t)brightness;
}

static void renderStart(Camera_t usedCam, textAnimsAtlas* allObjArray2D) {
    float fov = usedCam.fov;
    float nearPlane = usedCam.nearPlane;
    float farPlane = usedCam.farPlane;
    Vector3f camPos = usedCam.position;
    clippedTri clipped[2] = {0};

    int tri[3][2];
    int triStart = 0;
    if (MAX_TRIS > 0 && allAmt > MAX_TRIS) { triStart = (allAmt - MAX_TRIS); }
    for (int i = triStart; i < allAmt; i++) {
        worldTris* src = &allPoints[triOrder[i].idx];
        int t = src->textID;

        if (src->dimentions == D_3D) {
            int output = TriangleClipping(src->verts, &clipped[0], &clipped[1], nearPlane, farPlane);
            if (!output) continue;

            Vertex tmp[3];
            for (int c = 0; c < output; c++) {
                tmp[0].x = clipped[c].t1.x; tmp[0].y = clipped[c].t1.y; tmp[0].z = clipped[c].t1.z;
                tmp[1].x = clipped[c].t2.x; tmp[1].y = clipped[c].t2.y; tmp[1].z = clipped[c].t2.z;
                tmp[2].x = clipped[c].t3.x; tmp[2].y = clipped[c].t3.y; tmp[2].z = clipped[c].t3.z;
                for (int z = 0; z < 3; z++) { project2D(&tri[z][0], tmp[z], fov, nearPlane); }
                drawTriangle(tri, src->color);
            }
        } else if (src->dimentions == D_2D) {
            project2D(&tri[0][0], src->verts[0], fov, nearPlane);

            if (src->textID == -1) {
                drawRect(tri[0][0], tri[0][1], 5, 5, 20);
            } else {
                continue;

                textAtlas* textAtlasMem = &allObjArray2D->animation[t]->animData;
                drawImg(tri[0][0], tri[0][1], src->distMod, 0, 0, 30, 30, textAtlasMem->pixels, textAtlasMem->w, textAtlasMem->h, usedCam.projDist); 
                // drawImgNoScale(tri[0][0], tri[0][1], 0, 0, 30, 30, textAtlasMem->pixels, textAtlasMem->w, textAtlasMem->h);
            }
        }
    }
}

void addObjToWorld3D(Vector3f pos, Vector3f rot, Vector3f size, Camera_t cCam, float depthOffset, Mesh_t model, bool lightUse) {
    if (allAmt >= allPointsCount) return;

    worldTris* wTris;
    Vector3f camPos = cCam.position;
    float renderRadiusSq = cCam.farPlane ? (cCam.farPlane * cCam.farPlane) : 0.0f;

    int triCount = model.triCount;
    Vector3f* verticies = model.verts;
    int (*tris)[3] = model.tris;
    Edge* edges = model.edges;
    int* backFace = model.bfc;
    uint8_t* colorArray = model.color;
    Vector3f* normal = model.normal;

    bool rotObjs = false;
    float rotMat[3][3];
    if (rot.x != 0.0f || rot.y != 0.0f || rot.z != 0.0f || size.x != 1.0f || size.y != 1.0f || size.z != 1.0f) {
        computeRotScaleMatrix(rotMat, rot.x, rot.y, rot.z, size.x, size.y, size.z);
        rotObjs = true;
    }

    int triScn[3][2];
    int triIndex = allAmt;
    for (int i = 0; i < triCount; i++) {
        if (allAmt >= allPointsCount) return;
        wTris = &allPoints[allAmt];

        Vector3f r;
        int base = i * 3;
        float sumX = 0.0f, sumY = 0.0f, sumZ = 0.0f;
        float sumX_ = 0.0f, sumY_ = 0.0f, sumZ_ = 0.0f;
        for (int j = 0; j < 3; j++) {
            int idx = tris[i][j];
            if (rotObjs) { rotateVertex(verticies[idx], rotMat, &r); } else { r = verticies[idx]; }

            wTris->verts[j].x = r.x + pos.x;
            wTris->verts[j].y = r.y + pos.y;
            wTris->verts[j].z = r.z + pos.z;

            sumX += wTris->verts[j].x;
            sumY += wTris->verts[j].y;
            sumZ += wTris->verts[j].z;

            if (lightUse && lightAmt > 0) {
                sumX_ += verticies[idx].x;
                sumY_ += verticies[idx].y;
                sumZ_ += verticies[idx].z;
            }

            rotateVertexInPlace(&wTris->verts[j], camPos, cCam.camMatrix);
        }

        int cx = sumX * one_third;
        int cy = sumY * one_third;
        int cz = sumZ * one_third;

        Vector3f fVect = {cx - camPos.x, cy - camPos.y, cz - camPos.z};
        Vector3f n = normal[i];
        if (rotObjs) {
            rotateVertex(n, rotMat, &n);
            float len = sqrtf(n.x*n.x + n.y*n.y + n.z*n.z);
            if (len > 0.0f) { n.x /= len; n.y /= len; n.z /= len; }
        }
        float dot = n.x*fVect.x + n.y*fVect.y + n.z*fVect.z;
        triFacing[triIndex] = (dot < 0.0f);
        if (wTris->verts[0].z < cCam.nearPlane && wTris->verts[1].z < cCam.nearPlane && wTris->verts[2].z < cCam.nearPlane) continue;
        if (backFace[i] && !triFacing[triIndex]) { triIndex++; continue; }

        int dx = cx - camPos.x;
        int dy = cy - camPos.y;
        int dz = cz - camPos.z;
        int dist = (dx*dx + dy*dy + dz*dz);

        if (cCam.farPlane && dist > renderRadiusSq) continue;

        uint8_t color = colorArray[i];
        if (lightUse && lightAmt > 0) {
            int cx_ = sumX_ * one_third;
            int cy_ = sumY_ * one_third;
            int cz_ = sumZ_ * one_third;
            color = getBrightness((Vector3f){cx_, cy_, cz_}, lightSource, normal[i], color);
        }
        
        wTris->dimentions = D_3D;
        wTris->color      = color;
        wTris->distMod    = 0.0f;
        wTris->textID     = -1;

        wTris->verts[0].u = 0.0f; wTris->verts[0].v = 0.0f;
        wTris->verts[1].u = 1.0f; wTris->verts[1].v = 0.0f;
        wTris->verts[2].u = 0.0f; wTris->verts[2].v = 1.0f;

        triOrder[allAmt].idx = allAmt;
        triOrder[allAmt].dist = dist - depthOffset;
        allAmt++;
    }
}

void addWaves(WaterSlice* water, int index, int wAmt) {
    Vector2i min = water[index].min;
    Vector2i max = water[index].max;

    float y = water[index].y;

    int oldCount = water[index].lineCount;
    int newCount = oldCount + wAmt;

    water[index].lines = pd_realloc(water[index].lines, sizeof(LineSlice) * newCount);

    for (int w = 0; w < wAmt; w++) {
        int i = oldCount + w;

        float x = randomFloat(min.x, max.x);
        float z = randomFloat(min.z, max.z);
        int length = 11;

        water[index].lines[i] = (LineSlice) {
            .point = (Vector3f){x, y, z}, .z = length,
            .y = {y - 5, y + 5, 0.0f},
            .yDir = 0, .yVel = 0.0f, .yRange = {0.2f, -0.2f},
            .zVel = randomFloat(0.2f, 1.2f), .length = length, .color = randomInt(20, 31)
        };

        pd->system->logToConsole("X: %f | Z: %f | Index: %d", x, z, i);
    }

    water[index].lineCount = newCount;

    pd->system->logToConsole("%d water waves made!", wAmt);
}

void addWaveToWorld3D(LineSlice* line, Vector2i boundMin, Vector2i boundMax, Camera_t cCam) {
    Mesh_t waves = {0};

    float depth = line->length;
    Vector3f p0 = {0.0f, 0.0f, 0.0f};
    Vector3f p1 = {0.0f, 0.0f, depth};

    line->y[2] += line->yVel;
    if (line->yDir == 0) {
        if (line->y[2] < line->yRange[1]) { line->yVel += 0.2f; }
        else { line->yVel += 0.05f; }
        if (line->y[2] > line->yRange[0]) line->yDir = 1;
    } else {
        if (line->y[2] > line->yRange[0]) { line->yVel -= 0.2f;}
        else { line->yVel -= 0.05f; }
        if (line->y[2] < line->yRange[1]) line->yDir = 0;
    }

    if (line->yVel >= 0.8f) { line->yVel = 0.8f; }
    else if (line->yVel <= -0.8f) { line->yVel = -0.8f; }

    line->point.z += line->zVel;
    if (line->point.z > boundMin.z + line->length) {
        float x = randomFloat(boundMin.x, boundMax.x);
        line->point = (Vector3f){x, line->point.y, boundMax.z + line->length};
        line->y[2] = 0.0f;
        line->yVel = 0.0f;
        line->yDir = 0;
        line->zVel = randomFloat(0.2f, 1.2f);
        line->color = randomInt(20, 31);
    }
    
    float y0 = line->y[0];
    float y1 = line->y[1];
    float y2 = line->y[2];

    int color = line->color;

    Vector3f l0 = {p0.x + (y2*0.5f), y0, p0.z};
    Vector3f l1 = {p1.x, y0 + y2, p1.z};
    Vector3f l2 = {p1.x, y1 + y2, p1.z};

    Vector3f r0 = {p0.x, y0 + y2, p0.z + depth};
    Vector3f r1 = {p1.x - (y2*0.5f), y0, p1.z + depth};
    Vector3f r2 = {p0.x, y1 + y2, p0.z + depth};
    
    pushTri(&waves, l0.x, l0.y, l0.z, l2.x, l2.y, l2.z, l1.x, l1.y, l1.z, 0, color);
    pushTri(&waves, r0.x, r0.y, r0.z, r1.x, r1.y, r1.z, r2.x, r2.y, r2.z, 0, color);

    Vector3f pos  = line->point;
    Vector3f rot  = {0.0f, 0.0f, 0.0f};
    Vector3f size = {0.5f, 0.2f, 0.5f};

    addObjToWorld3D(pos, rot, size, cCam, 50.0f, waves, false);
}

void addBilboard(Vector3f pos, Vector3f size, Camera_t cCam) {
    Mesh_t bilboard = {0};

    float yaw = cCam.rotation.y + M_PI;

    float s = sinf(yaw);
    float c = cosf(yaw);

    float hw = size.x * 0.5f;
    float hh = size.y * 0.5f;

    Vector3f v0 = {-0.5f, 0, -0.5f};
    Vector3f v1 = {0.5f, 0, -0.5f};
    Vector3f v2 = {0.5f, 2, -0.5f};
    Vector3f v3 = {-0.5f, 2, -0.5f};

    pushTri(&bilboard, v0.x, v0.y, v0.z, v2.x, v2.y, v2.z, v1.x, v1.y, v1.z, 0, 15);
    pushTri(&bilboard, v0.x, v0.y, v0.z, v3.x, v3.y, v3.z, v2.x, v2.y, v2.z, 0, 25);

    float dx = cCam.position.x - pos.x;
    float dz = cCam.position.z - pos.z;

    float facingCamAngle = atan2f(dx, dz);
    Vector3f rot = {0, facingCamAngle, 0};

    addObjToWorld3D(pos, rot, size, cCam, 50.0f, bilboard, false);
}

void addObjToWorld2D(Vector3f pos, Camera_t cCam, float objDepthOffset, float sprtDepthOffset, int anim, int animFrame) {
    if (allAmt >= allPointsCount) return;
    
    worldTris* wTris = &allPoints[allAmt];
    Vector3f camPos = cCam.position;
    float renderRadiusSq = cCam.farPlane ? (cCam.farPlane * cCam.farPlane) : 0.0f;

    float dx = pos.x - camPos.x;
    float dy = (pos.y - 5.0f) - camPos.y;
    float dz = pos.z - camPos.z;
    float dist = (dx*dx + dy*dy + dz*dz);

    if (cCam.farPlane && dist > renderRadiusSq) return;

    wTris->verts[0].x = pos.x; wTris->verts[0].y = pos.y; wTris->verts[0].z = pos.z;

    rotateVertexInPlace(&wTris->verts[0], camPos, cCam.camMatrix);
    if (wTris->verts[0].z < cCam.nearPlane || wTris->verts[0].z > cCam.farPlane) return;

    wTris->dimentions = D_2D;
    wTris->distMod    = sqrtf(dist) + sprtDepthOffset;
    wTris->textID     = anim;
    wTris->color      = animFrame;

    triOrder[allAmt].idx = allAmt;
    triOrder[allAmt].dist = dist - objDepthOffset;
    allAmt++;
}

void shootRender(Camera_t cam, textAnimsAtlas* allObjArray2D) {
    if (allAmt > 1) quickSortIndices(triOrder, 0, allAmt - 1);

    renderStart(cam, allObjArray2D);
    allAmt = 0;
}

void resetAllArrays() {
    allPoints = pd_malloc(sizeof(worldTris) * allPointsCount);
    triOrder = pd_malloc(sizeof(TriangleOrdering) * allPointsCount);
    triFacing = pd_malloc(sizeof(int) * allPointsCount);
    allAmt = 0;
}

void precomputedFunctions(Camera_t* cam) { 
    Vector3f cRot = cam->rotation;

    computeCamMatrix(cam->camMatrix, cRot.x, cRot.y, cRot.z);
    cam->projDist = (sW_H * 0.5f) / tanf(cam->fov * 0.5f);

    cam->fVect.x = cos(cRot.x) * sin(cRot.y);
    cam->fVect.y = sin(cRot.x);
    cam->fVect.z = cos(cRot.x) * cos(cRot.y);
}