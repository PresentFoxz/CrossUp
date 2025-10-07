#include "draw.h"

const int pattern[4][2][2] = {
    {{0,1},{0,0}},
    {{1,0},{0,1}},
    {{1,1},{1,0}},
    {{1,1},{1,1}}
};

static void blockCol(uint x, uint y, int shade, uint8_t* row) {
    uint px = x % 2;
    uint py = y % 2;

    if (pattern[shade][py][px])
        setPixelRaw(x, row, 1);
    else
        setPixelRaw(x, row, 0);
}

void project2D(int point[2], float verts[3], float fov, float nearPlane) {
    float z = verts[2];
    if (z < nearPlane) z = nearPlane;

    float half = (fov / z);
    
    float projected_x = verts[0] * half;
    float projected_y = verts[1] * half;
    
    point[0] = (int)(projected_x + (sW_H + sX));
    point[1] = (int)(-projected_y + (sH_H + sY));
}

void RotationMatrix(float x, float y, float z, float sin1, float cos1, float sin2, float cos2, float sin3, float cos3, float* rot){
    float tempX = (z * sin1) + (x * cos1);
    float tempZ = (z * cos1) - (x * sin1);

    float tempY = (tempZ * sin2) + (y * cos2);
    float finalZ = (tempZ * cos2) - (y * sin2);

    float finalX = (tempX * cos3) - (tempY * sin3);
    float finalY = (tempX * sin3) + (tempY * cos3);

    rot[0] = finalX;
    rot[1] = finalY;
    rot[2] = finalZ;
}

void RotateVertexObject(float x, float y, float z, float sinx, float cosx, float siny, float cosy, float sinz, float cosz, float sx, float sy, float sz, float out[3]) {
    float X = x * sx;
    float Y = y * sy;
    float Z = z * sz;
    
    float y1 = Y * cosx - Z * sinx;
    float z1 = Y * sinx + Z * cosx;
    
    float x2 = X * cosy + z1 * siny;
    float z2 = -X * siny + z1 * cosy;
    
    float x3 = x2 * cosz - y1 * sinz;
    float y3 = x2 * sinz + y1 * cosz;

    out[0] = x3;
    out[1] = y3;
    out[2] = z2;
}

int windingOrder(int *p0, int *p1, int *p2) { return (p0[0]*p1[1] - p0[1]*p1[0] + p1[0]*p2[1] - p1[1]*p2[0] + p2[0]*p0[1] - p2[1]*p0[0] > 0); }

void drawFilledTrisZ(int tris[3][2], clippedTri fullTri, int triColor, qfixed16_t* zBuffer) {
    float v0x = (float)(tris[0][0]), v0y = (float)(tris[0][1]), v0z = fullTri.t1.z;
    float v1x = (float)(tris[1][0]), v1y = (float)(tris[1][1]), v1z = fullTri.t2.z;
    float v2x = (float)(tris[2][0]), v2y = (float)(tris[2][1]), v2z = fullTri.t3.z;

    int minX = (int)fmaxf(0.0f, (float)fminf(fminf((float)v0x, (float)v1x), (float)v2x));
    int maxX = (int)fminf((float)(SCREEN_W-1), (float)fmaxf(fmaxf((float)v0x, (float)v1x), (float)v2x));
    int minY = (int)fmaxf(0.0f, (float)fminf(fminf((float)v0y, (float)v1y), (float)v2y));
    int maxY = (int)fminf((float)(SCREEN_H-1), (float)fmaxf(fmaxf((float)v0y, (float)v1y), (float)v2y));

    float denom = (v1y - v2y)*(v0x - v2x) + (v2x - v1x)*(v0y - v2y);
    float invDenom = 1.0f / denom;

    float aStepX = (v1y - v2y) * invDenom;
    float bStepX = (v2y - v0y) * invDenom;
    float aStepY = (v2x - v1x) * invDenom;
    float bStepY = (v0x - v2x) * invDenom;

    const int rowStride = 52;

    float startX = (float)minX + 0.5f;
    float startY = (float)minY + 0.5f;
    float aStart = ((v1y - v2y)*(startX - v2x) + (v2x - v1x)*(startY - v2y)) * invDenom;
    float bStart = ((v2y - v0y)*(startX - v2x) + (v0x - v2x)*(startY - v2y)) * invDenom;

    qfixed16_t invZStepX = TO_FIXED1_15(aStepX * v0z + bStepX * v1z + (-aStepX - bStepX) * v2z);

    for (int y = minY; y <= maxY; y++) {
        int idx = y * SCREEN_W;
        uint8_t* row = buf + y * rowStride;
        
        float a = aStart;
        float b = bStart;
        float c = 1.0f - a - b;

        qfixed16_t invZ = TO_FIXED1_15(a * v0z + b * v1z + c * v2z);

        for (int x = minX; x <= maxX; x++) {
            if (((int)(a * 4096.0f) | (int)(b * 4096.0f) | (int)(c * 4096.0f)) >= 0) {
                int index = idx+x;
                if (invZ < zBuffer[index]) {
                    zBuffer[index] = invZ;
                    blockCol(x, y, triColor, row);
                }
            }
            
            a += aStepX;
            b += bStepX;
            c = 1.0f - a - b;
            invZ += invZStepX;
        }
        aStart += aStepY;
        bStart += bStepY;
    }
}

int TriangleClipping(Vertex verts[3], clippedTri* outTri1, clippedTri* outTri2, float nearPlane, float farPlane) {
    int inScreen[3], outScreen[3];
    int inAmt = 0, outAmt = 0;

    for (int i = 0; i < 3; i++) {
        if (verts[i].z >= nearPlane && verts[i].z <= farPlane) {
            inScreen[inAmt++] = i;
        } else {
            outScreen[outAmt++] = i;
        }
    }

    Vertex cross0, cross1;

    if (inAmt == 0) {
        // Entire triangle is outside
        return 0;
    } else if (inAmt == 3) {
        // Entire triangle is inside
        outTri1->t1 = verts[0];
        outTri1->t2 = verts[1];
        outTri1->t3 = verts[2];
        return 1;
    } else if (inAmt == 1) {
        // One vertex inside, two outside → create one triangle
        int in0 = inScreen[0];
        int out0 = outScreen[0];
        int out1 = outScreen[1];

        float t0 = (nearPlane - verts[out0].z) / (verts[in0].z - verts[out0].z);
        float t1 = (nearPlane - verts[out1].z) / (verts[in0].z - verts[out1].z);

        // Interpolate positions
        cross0.x = verts[out0].x + t0 * (verts[in0].x - verts[out0].x);
        cross0.y = verts[out0].y + t0 * (verts[in0].y - verts[out0].y);
        cross0.z = verts[out0].z + t0 * (verts[in0].z - verts[out0].z);

        cross1.x = verts[out1].x + t1 * (verts[in0].x - verts[out1].x);
        cross1.y = verts[out1].y + t1 * (verts[in0].y - verts[out1].y);
        cross1.z = verts[out1].z + t1 * (verts[in0].z - verts[out1].z);

        // Interpolate UVs
        cross0.u = verts[out0].u + t0 * (verts[in0].u - verts[out0].u);
        cross0.v = verts[out0].v + t0 * (verts[in0].v - verts[out0].v);

        cross1.u = verts[out1].u + t1 * (verts[in0].u - verts[out1].u);
        cross1.v = verts[out1].v + t1 * (verts[in0].v - verts[out1].v);

        // Assign clipped triangle
        outTri1->t1 = verts[in0];  // inside vertex
        outTri1->t2 = cross0;      // clipped vertex
        outTri1->t3 = cross1;      // clipped vertex

        return 1;
    } else if (inAmt == 2) {
        // Two vertices inside, one outside → create two triangles
        int in0 = inScreen[0];
        int in1 = inScreen[1];
        int out0 = outScreen[0];

        float t0 = (nearPlane - verts[out0].z) / (verts[in0].z - verts[out0].z);
        float t1 = (nearPlane - verts[out0].z) / (verts[in1].z - verts[out0].z);

        // Interpolate positions
        cross0.x = verts[out0].x + t0 * (verts[in0].x - verts[out0].x);
        cross0.y = verts[out0].y + t0 * (verts[in0].y - verts[out0].y);
        cross0.z = verts[out0].z + t0 * (verts[in0].z - verts[out0].z);

        cross1.x = verts[out0].x + t1 * (verts[in1].x - verts[out0].x);
        cross1.y = verts[out0].y + t1 * (verts[in1].y - verts[out0].y);
        cross1.z = verts[out0].z + t1 * (verts[in1].z - verts[out0].z);

        // Interpolate UVs
        cross0.u = verts[out0].u + t0 * (verts[in0].u - verts[out0].u);
        cross0.v = verts[out0].v + t0 * (verts[in0].v - verts[out0].v);

        cross1.u = verts[out0].u + t1 * (verts[in1].u - verts[out0].u);
        cross1.v = verts[out0].v + t1 * (verts[in1].v - verts[out0].v);

        // First clipped triangle
        outTri1->t1 = verts[in0];
        outTri1->t2 = verts[in1];
        outTri1->t3 = cross1;

        // Second clipped triangle
        outTri2->t1 = verts[in0];
        outTri2->t2 = cross0;
        outTri2->t3 = cross1;

        return 2;
    }

    return 0;
}

void staticLineDrawing(int p0[2], int p1[2], int color){
    const int rand = randomInt(3, 15);
    int new[2] = {p0[0], p0[1]};
    int old[2] = {p0[0], p0[1]};

    for (int i=0; i <= rand; i++){
        int end = randomInt(3, 20);
        old[0] = new[0];
        old[1] = new[1];

        int dx = new[0] - p1[0];
        int dy = new[1] - p1[1];
        
        int dist = ((dx*dx)+(dy*dy)/2);
        if (dist < 1) { dist = 1; } else if (dist > end) { dist = end; }
        int xRand = randomInt(0, dist);
        int yRand = randomInt(0, dist);

        if (new[0] > p1[0]){
            new[0] += -xRand;
        } else {
            new[0] += xRand;
        }
        
        if (new[1] > p1[1]){
            new[1] += -yRand;
        } else {
            new[1] += yRand;
        }

        int size = randomInt(1, 4);
        LCDBitmapDrawMode colIDX = (color) ? kColorWhite : kColorBlack;

        if (i != rand){
            pd->graphics->drawLine(old[0], old[1], new[0], new[1], size, colIDX);
        } else {
            pd->graphics->drawLine(new[0], new[1], p1[0], p1[1], size, colIDX);
        }
    }
}