#include "draw.h"

static const uint8_t shadeLUT[4][2][2] = {
    {{0,1},{0,0}},
    {{0,1},{1,0}},
    {{0,0},{1,1}},
    {{1,1},{1,1}}
};

static inline void multiPixl(uint gridX, uint gridY, int shade, uint8_t* row) {
    if (gridX + 1 >= sW || gridY + 1 >= sH) return;

    const uint8_t* lut0 = shadeLUT[shade][gridY & 1];
    const uint8_t* lut1 = shadeLUT[shade][(gridY + 1) & 1];

    uint8_t* row0 = buf + (gridY) * rowStride;
    uint8_t* row1 = buf + (gridY + 1) * rowStride;

    uint8_t px = gridX & 1;
    setPixelRaw(gridX,     row0, lut0[px]);
    setPixelRaw(gridX + 1, row0, lut0[px ^ 1]);
    setPixelRaw(gridX,     row1, lut1[px]);
    setPixelRaw(gridX + 1, row1, lut1[px ^ 1]);
}

static inline void singlePixl(uint x, uint y, int shade, uint8_t* row) {
    uint px = x % 2;
    uint py = y % 2;

    setPixelRaw(x, row, shadeLUT[shade][py][px]);
}

static void drawCustomLine(int x1, int y1, int x2, int y2, int sizeMin, int sizeMax, int type, qfixed16_t* zBuffer, float z1, float z2) {
    int dx = x2 > x1 ? x2 - x1 : x1 - x2;
    int sx = x1 < x2 ? 1 : -1;
    int dy = y2 > y1 ? y2 - y1 : y1 - y2;
    int sy = y1 < y2 ? 1 : -1;

    int err = dx - dy;

    int newX = x1;
    int newY = y1;

    int length = dx > dy ? dx : dy;
    qfixed16_t zStep = TO_FIXED1_15((length > 0) ? (z2 - z1) / length : 0);
    qfixed16_t currZ = TO_FIXED1_15(z1);

    while (1) {
        for (int oy = sizeMin; oy <= sizeMax; oy++) {
            for (int ox = sizeMin; ox <= sizeMax; ox++) {
                int px = newX + ox;
                int py = newY + oy;

                if (px < 0 || px >= sW || py < 0 || py >= sH) continue;

                int index = py * sW + px;

                if (currZ < zBuffer[index]) {
                    zBuffer[index] = currZ;
                    setPixelRaw(px, buf + py * 52, type ? 1 : 0);
                }
            }
        }

        if (newX == x2 && newY == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            newX += sx;
        }
        if (e2 < dx) {
            err += dx;
            newY += sy;
        }

        currZ += zStep;
    }
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

void rotateVertexInPlace(Vertex* v, Vect3f camPos, float rotMat[3][3]) {
    float x = v->x - camPos.x;
    float y = v->y - camPos.y;
    float z = v->z - camPos.z;
    
    float rx = x * rotMat[0][0] + y * rotMat[0][1] + z * rotMat[0][2];
    float ry = x * rotMat[1][0] + y * rotMat[1][1] + z * rotMat[1][2];
    float rz = x * rotMat[2][0] + y * rotMat[2][1] + z * rotMat[2][2];
    
    v->x = rx;
    v->y = ry;
    v->z = rz;
}

void rotateVertex(float x, float y, float z, float rotMat[3][3], float out[3]) {
    out[0] = x * rotMat[0][0] + y * rotMat[0][1] + z * rotMat[0][2];
    out[1] = x * rotMat[1][0] + y * rotMat[1][1] + z * rotMat[1][2];
    out[2] = x * rotMat[2][0] + y * rotMat[2][1] + z * rotMat[2][2];
}

void computeCamMatrix(float m[3][3], float sinY, float cosY, float sinX, float cosX, float sinZ, float cosZ) {
    m[0][0] = cosY * cosZ + sinY * sinX * sinZ;
    m[0][1] = cosX * sinZ;
    m[0][2] = -sinY * cosZ + cosY * sinX * sinZ;

    m[1][0] = -cosY * sinZ + sinY * sinX * cosZ;
    m[1][1] = cosX * cosZ;
    m[1][2] = sinY * sinZ + cosY * sinX * cosZ;

    m[2][0] = sinY * cosX;
    m[2][1] = -sinX;
    m[2][2] = cosY * cosX;
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

static inline int32_t edgeA(int y0, int y1) { return y0 - y1; }
static inline int32_t edgeB(int x0, int x1) { return x1 - x0; }
static inline int32_t edgeC(int x0, int y0, int x1, int y1) { return x0*y1 - x1*y0; }

void drawFilledTrisZ(int tris[3][2], clippedTri fullTri, int triColor, qfixed16_t* zBuffer) {
    int x0 = tris[0][0], y0 = tris[0][1];
    int x1 = tris[1][0], y1 = tris[1][1];
    int x2 = tris[2][0], y2 = tris[2][1];

    float z0 = fullTri.t1.z;
    float z1 = fullTri.t2.z;
    float z2 = fullTri.t3.z;
    
    int minX = x0 < x1 ? (x0 < x2 ? x0 : x2) : (x1 < x2 ? x1 : x2);
    int maxX = x0 > x1 ? (x0 > x2 ? x0 : x2) : (x1 > x2 ? x1 : x2);
    int minY = y0 < y1 ? (y0 < y2 ? y0 : y2) : (y1 < y2 ? y1 : y2);
    int maxY = y0 > y1 ? (y0 > y2 ? y0 : y2) : (y1 > y2 ? y1 : y2);
    
    if (minX < 0) minX = 0;
    if (minY < 0) minY = 0;
    if (maxX >= sW) maxX = sW - 1;
    if (maxY >= sH) maxY = sH - 1;
    
    minX = (minX / pixSizeX) * pixSizeX;
    minY = (minY / pixSizeY) * pixSizeY;
    maxX = ((maxX + pixSizeX - 1) / pixSizeX) * pixSizeX;
    maxY = ((maxY + pixSizeY - 1) / pixSizeY) * pixSizeY;
    
    int32_t A01 = edgeA(y0, y1), B01 = edgeB(x0, x1), C01 = edgeC(x0, y0, x1, y1);
    int32_t A12 = edgeA(y1, y2), B12 = edgeB(x1, x2), C12 = edgeC(x1, y1, x2, y2);
    int32_t A20 = edgeA(y2, y0), B20 = edgeB(x2, x0), C20 = edgeC(x2, y2, x0, y0);

    int flip = ((x1 - x0)*(y2 - y0) - (x2 - x0)*(y1 - y0)) < 0 ? -1 : 1;
    
    int32_t w0_row = A12*minX + B12*minY + C12;
    int32_t w1_row = A20*minX + B20*minY + C20;
    int32_t w2_row = A01*minX + B01*minY + C01;
    
    float det = (x0*(y1 - y2) + x1*(y2 - y0) + x2*(y0 - y1));
    if (det == 0.0f) return;
    float invDet = 1.0f / det;
    float stepX = (z0*(y1 - y2) + z1*(y2 - y0) + z2*(y0 - y1)) * invDet;
    float stepY = (z0*(x2 - x1) + z1*(x0 - x2) + z2*(x1 - x0)) * invDet;
    float zC = z0 - stepX*x0 - stepY*y0;

    qfixed16_t invZStepX = TO_FIXED1_15(stepX);
    qfixed16_t invZStepY = TO_FIXED1_15(stepY);
    qfixed16_t invZ_row = TO_FIXED1_15(stepX*minX + stepY*minY + zC);

    for (int y = minY; y < sH && y <= maxY; y += pixSizeY) {
        uint gridY = y & ~(pixSizeY - 1);
        if (gridY >= sH) gridY = sH - 1;
        int idx = gridY * sW;

        int32_t w0 = w0_row;
        int32_t w1 = w1_row;
        int32_t w2 = w2_row;

        uint8_t* row = buf + gridY * rowStride;
        qfixed16_t invZ = invZ_row;

        for (int x = minX; x < sW && x <= maxX; x += pixSizeX) {
            uint gridX = x & ~(pixSizeX - 1);
            if (gridX >= sW) gridX = sW - 1;
            
            if ((w0*flip >= 0) && (w1*flip >= 0) && (w2*flip >= 0)) {
                int index = idx + gridX;

                if (invZ < zBuffer[index]) {
                    zBuffer[index] = invZ;
                    multiPixl(gridX, gridY, triColor, row);
                }
            }

            w0 += A12 * pixSizeX;
            w1 += A20 * pixSizeX;
            w2 += A01 * pixSizeX;
            invZ += invZStepX * pixSizeX;
        }

        w0_row += B12 * pixSizeY;
        w1_row += B20 * pixSizeY;
        w2_row += B01 * pixSizeY;
        invZ_row += invZStepY * pixSizeY;
    }
}

static inline int edgeFunc(int x0, int y0, int x1, int y1, int px, int py) { return (py - y0) * (x1 - x0) - (px - x0) * (y1 - y0); }

void drawFilledTrisNoZ(int tris[3][2], int triColor) {
    int x0 = tris[0][0], y0 = tris[0][1];
    int x1 = tris[1][0], y1 = tris[1][1];
    int x2 = tris[2][0], y2 = tris[2][1];
    
    int minX = x0 < x1 ? (x0 < x2 ? x0 : x2) : (x1 < x2 ? x1 : x2);
    int maxX = x0 > x1 ? (x0 > x2 ? x0 : x2) : (x1 > x2 ? x1 : x2);
    int minY = y0 < y1 ? (y0 < y2 ? y0 : y2) : (y1 < y2 ? y1 : y2);
    int maxY = y0 > y1 ? (y0 > y2 ? y0 : y2) : (y1 > y2 ? y1 : y2);
    
    if (minX < 0) minX = 0;
    if (minY < 0) minY = 0;
    if (maxX >= sW) maxX = sW - 1;
    if (maxY >= sH) maxY = sH - 1;
    
    minX = (minX / pixSizeX) * pixSizeX;
    minY = (minY / pixSizeY) * pixSizeY;
    maxX = ((maxX + pixSizeX - 1) / pixSizeX) * pixSizeX;
    maxY = ((maxY + pixSizeY - 1) / pixSizeY) * pixSizeY;

    int A01 = (y0 - y1) * pixSizeX, B01 = (x1 - x0) * pixSizeY;
    int A12 = (y1 - y2) * pixSizeX, B12 = (x2 - x1) * pixSizeY;
    int A20 = (y2 - y0) * pixSizeX, B20 = (x0 - x2) * pixSizeY;
    
    int yStart = minY;
    int xStart = minX;

    int w0_row = edgeFunc(x1, y1, x2, y2, xStart, yStart);
    int w1_row = edgeFunc(x2, y2, x0, y0, xStart, yStart);
    int w2_row = edgeFunc(x0, y0, x1, y1, xStart, yStart);

    int area = edgeFunc(x0, y0, x1, y1, x2, y2);
    if (area < 0) {
        int tmp;
        tmp = A01; A01 = -A01; B01 = -B01;
        tmp = A12; A12 = -A12; B12 = -B12;
        tmp = A20; A20 = -A20; B20 = -B20;
        w0_row = -w0_row;
        w1_row = -w1_row;
        w2_row = -w2_row;
    }

    for (int y = minY; y <= maxY && y < sH; y += pixSizeY) {
        uint gridY = y & ~(pixSizeY - 1);
        if (gridY >= sH) gridY = sH - 1;

        uint8_t* row = buf + gridY * rowStride;

        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;

        for (int x = minX; x <= maxX && x < sW; x += pixSizeX) {
            uint gridX = x & ~(pixSizeX - 1);
            if (gridX >= sW) gridX = sW - 1;

            if ((w0 | w1 | w2) >= 0) {
                multiPixl(gridX, gridY, triColor, row);
            }

            w0 += A12;
            w1 += A20;
            w2 += A01;
        }

        w0_row += B12;
        w1_row += B20;
        w2_row += B01;
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