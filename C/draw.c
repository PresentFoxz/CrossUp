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

    float invZ = 1.0f / z;
    float scale = fov * invZ;

    point[0] = (int)(verts[0] * scale + (sW_H + sX));
    point[1] = (int)(-verts[1] * scale + (sH_H + sY));
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

void rotateVertex(float x, float y, float z, float rotMat[3][3], float out[3]) {
    out[0] = x * rotMat[0][0] + y * rotMat[0][1] + z * rotMat[0][2];
    out[1] = x * rotMat[1][0] + y * rotMat[1][1] + z * rotMat[1][2];
    out[2] = x * rotMat[2][0] + y * rotMat[2][1] + z * rotMat[2][2];
}

void computeRotScaleMatrix(float rotMat[3][3], float angleX, float angleY, float angleZ, float sx, float sy, float sz) {
    float sinX = sinf(angleX), cosX = cosf(angleX);
    float sinY = sinf(angleY), cosY = cosf(angleY);
    float sinZ = sinf(angleZ), cosZ = cosf(angleZ);

    rotMat[0][0] = (cosY * cosZ) * sx;
    rotMat[0][1] = (-cosY * sinZ) * sx;
    rotMat[0][2] = (sinY) * sx;

    rotMat[1][0] = (sinX * sinY * cosZ + cosX * sinZ) * sy;
    rotMat[1][1] = (-sinX * sinY * sinZ + cosX * cosZ) * sy;
    rotMat[1][2] = (-sinX * cosY) * sy;

    rotMat[2][0] = (-cosX * sinY * cosZ + sinX * sinZ) * sz;
    rotMat[2][1] = (cosX * sinY * sinZ + sinX * cosZ) * sz;
    rotMat[2][2] = (cosX * cosY) * sz;
}

int windingOrder(int *p0, int *p1, int *p2) { return (p0[0]*p1[1] - p0[1]*p1[0] + p1[0]*p2[1] - p1[1]*p2[0] + p2[0]*p0[1] - p2[1]*p0[0] > 0); }

static inline int32_t edge_A(const int v0[2], const int v1[2]) { return v0[1] - v1[1]; }
static inline int32_t edge_B(const int v0[2], const int v1[2]) { return v1[0] - v0[0]; }
static inline int32_t edge_C(const int v0[2], const int v1[2]) { return v0[0]*v1[1] - v0[1]*v1[0]; }

void drawFilledTrisZ(int tris[3][2], clippedTri fullTri, int triColor, qfixed16_t* zBuffer) {
    int area = (tris[1][0]-tris[0][0])*(tris[2][1]-tris[0][1]) - (tris[2][0]-tris[0][0])*(tris[1][1]-tris[0][1]);
    int flip = (area < 0) ? -1 : 1;

    float x0 = (float)tris[0][0], y0 = (float)tris[0][1], z0 = fullTri.t1.z;
    float x1 = (float)tris[1][0], y1 = (float)tris[1][1], z1 = fullTri.t2.z;
    float x2 = (float)tris[2][0], y2 = (float)tris[2][1], z2 = fullTri.t3.z;

    int minX = (int)fmaxf(0.0f, fminf(fminf(x0, x1), x2));
    int maxX = (int)fminf((float)(SCREEN_W-1), fmaxf(fmaxf(x0, x1), x2));
    int minY = (int)fmaxf(0.0f, fminf(fminf(y0, y1), y2));
    int maxY = (int)fminf((float)(SCREEN_H-1), fmaxf(fmaxf(y0, y1), y2));

    if (minX > maxX || minY > maxY) return;

    int32_t A01 = edge_A(tris[0], tris[1]), B01 = edge_B(tris[0], tris[1]), C01 = edge_C(tris[0], tris[1]);
    int32_t A12 = edge_A(tris[1], tris[2]), B12 = edge_B(tris[1], tris[2]), C12 = edge_C(tris[1], tris[2]);
    int32_t A20 = edge_A(tris[2], tris[0]), B20 = edge_B(tris[2], tris[0]), C20 = edge_C(tris[2], tris[0]);

    long long px = minX;
    long long py = minY;

    long long w0_row = (long long)A12 * px + (long long)B12 * py + (long long)C12;
    long long w1_row = (long long)A20 * px + (long long)B20 * py + (long long)C20;
    long long w2_row = (long long)A01 * px + (long long)B01 * py + (long long)C01;

    float det = (x0*(y1 - y2) + x1*(y2 - y0) + x2*(y0 - y1));
    if (det == 0.0f) return;
    float invDet = 1.0f / det;
    
    float A = (z0*(y1 - y2) + z1*(y2 - y0) + z2*(y0 - y1)) * invDet;
    float B = (z0*(x2 - x1) + z1*(x0 - x2) + z2*(x1 - x0)) * invDet;
    float C = z0 - A*x0 - B*y0;

    qfixed16_t invZStepX = TO_FIXED1_15(A);
    qfixed16_t invZStepY = TO_FIXED1_15(B);
    qfixed16_t invZ_row   = TO_FIXED1_15(A * minX + B * minY + C);

    const int rowStride = 52;

    for (int y = minY; y <= maxY; ++y) {
        int idx = y * SCREEN_W;

        long long w0 = w0_row;
        long long w1 = w1_row;
        long long w2 = w2_row;

        uint8_t* row = buf + y * rowStride;

        qfixed16_t invZ = invZ_row;
        for (int x = minX; x <= maxX; ++x) {
            if ((w0*flip >= 0) && (w1*flip >= 0) && (w2*flip >= 0)) {
                int index = idx + x;
                if (invZ < zBuffer[index]) {
                    zBuffer[index] = invZ;
                    blockCol(x, y, triColor, row);
                }
            }
            w0 += A12;
            w1 += A20;
            w2 += A01;
            invZ += invZStepX;
        }
        w0_row += B12;
        w1_row += B20;
        w2_row += B01;
        invZ_row += invZStepY;
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