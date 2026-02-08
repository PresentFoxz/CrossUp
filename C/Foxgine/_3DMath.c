#include "_3DMath.h"

void project2D(int point[2], float verts[3], float fov, float nearPlane) {
    float z = verts[2];
    if (z < nearPlane) z = nearPlane;

    float invZ = 1.0f / z;
    float scale = fov * invZ;

    point[0] = (int)(verts[0] * scale + (sW_H + sX));
    point[1] = (int)(-verts[1] * scale + (sH_H + sY));
}

void rotateVertexInPlace(Vertex* v, Vect3f camPos, float camMatrix[3][3]) {
    float x = v->x - camPos.x;
    float y = v->y - camPos.y;
    float z = v->z - camPos.z;

    float rx = x * camMatrix[0][0] + y * camMatrix[1][0] + z * camMatrix[2][0];
    float ry = x * camMatrix[0][1] + y * camMatrix[1][1] + z * camMatrix[2][1];
    float rz = x * camMatrix[0][2] + y * camMatrix[1][2] + z * camMatrix[2][2];

    v->x = rx;
    v->y = ry;
    v->z = rz;
}

void rotateVertex(float x, float y, float z, float rotMat[3][3], float out[3]) {
    out[0] = x * rotMat[0][0] + y * rotMat[0][1] + z * rotMat[0][2];
    out[1] = x * rotMat[1][0] + y * rotMat[1][1] + z * rotMat[1][2];
    out[2] = x * rotMat[2][0] + y * rotMat[2][1] + z * rotMat[2][2];
}

int backfaceCullCamera(Vertex* v0, Vertex* v1, Vertex* v2, Vect3f cam, int flipped) {
    Vect3f e1 = {v1->x - v0->x, v1->y - v0->y, v1->z - v0->z};
    Vect3f e2 = {v2->x - v0->x, v2->y - v0->y, v2->z - v0->z};
    
    Vect3f normal = { e1.y * e2.z - e1.z * e2.y, e1.z * e2.x - e1.x * e2.z, e1.x * e2.y - e1.y * e2.x };
    Vect3f center = { (v0->x + v1->x + v2->x) / 3.0f, (v0->y + v1->y + v2->y) / 3.0f, (v0->z + v1->z + v2->z) / 3.0f };
    
    Vect3f view = { center.x - cam.x, center.y - cam.y, center.z - cam.z };
    float dot = normal.x * view.x + normal.y * view.y + normal.z * view.z;
    
    return flipped ? (dot > 0.0f) : (dot < 0.0f);
}


void computeCamMatrix(float m[3][3], float pitchX, float yawY, float rollZ) {
    float sinY = sinf(yawY),   cosY = cosf(yawY);
    float sinX = sinf(pitchX), cosX = cosf(pitchX);
    float sinZ = sinf(rollZ),  cosZ = cosf(rollZ);
    
    m[0][0] = cosY * cosZ + sinY * sinX * sinZ;
    m[0][1] = -cosY * sinZ + sinY * sinX * cosZ;
    m[0][2] = sinY * cosX;

    m[1][0] = cosX * sinZ;
    m[1][1] = cosX * cosZ;
    m[1][2] = -sinX;

    m[2][0] = -sinY * cosZ + cosY * sinX * sinZ;
    m[2][1] = sinY * sinZ + cosY * sinX * cosZ;
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

int windingOrder(const int p0[2], const int p1[2], const int p2[2]) {
    long cross = (long)p0[0]*p1[1] - (long)p0[1]*p1[0] + (long)p1[0]*p2[1] - (long)p1[1]*p2[0] + (long)p2[0]*p0[1] - (long)p2[1]*p0[0];
    return cross > 0;
}

static inline int edgeFunc(int x0, int y0, int x1, int y1, int px, int py) { return (py - y0) * (x1 - x0) - (px - x0) * (y1 - y0); }

void drawFilledTris(int tris[3][2], int triColor) {
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
    
    minX = (minX / resolution) * resolution;
    minY = (minY / resolution) * resolution;
    maxX = ((maxX + resolution - 1) / resolution) * resolution;
    maxY = ((maxY + resolution - 1) / resolution) * resolution;

    int A01 = (y0 - y1) * resolution, B01 = (x1 - x0) * resolution;
    int A12 = (y1 - y2) * resolution, B12 = (x2 - x1) * resolution;
    int A20 = (y2 - y0) * resolution, B20 = (x0 - x2) * resolution;

    int w0_row = edgeFunc(x1, y1, x2, y2, minX, minY);
    int w1_row = edgeFunc(x2, y2, x0, y0, minX, minY);
    int w2_row = edgeFunc(x0, y0, x1, y1, minX, minY);

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

    int32_t w0, w1, w2;
    uint gridX, gridY;

    if ((maxX - minX) < resolution) maxX = minX + resolution;
    if ((maxY - minY) < resolution) maxY = minY + resolution;

    for (int y = minY; y <= maxY && y < sH; y += resolution) {
        gridY = (y & ~(resolution - 1));
        if (gridY >= sH) gridY = sH - 1;

        w0 = w0_row;
        w1 = w1_row;
        w2 = w2_row;

        for (int x = minX; x <= maxX && x < sW; x += resolution) {
            gridX = (x & ~(resolution - 1));
            if (gridX >= sW) gridX = sW - 1;

            if ((w0 | w1 | w2) >= 0) {
                int gx = gridX / resolution;
                int gy = gridY / resolution;

                setPixScnBuf(gx, gy, triColor);
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

void drawTexturedTris(int tris[3][2], float uvs[3][2], int* texture, int texW, int texH) {
    int x0 = tris[0][0], y0 = tris[0][1];
    int x1 = tris[1][0], y1 = tris[1][1];
    int x2 = tris[2][0], y2 = tris[2][1];

    int u0 = (int)(uvs[0][0] * (texW - 1)), v0 = (int)(uvs[0][1] * (texH - 1));
    int u1 = (int)(uvs[1][0] * (texW - 1)), v1 = (int)(uvs[1][1] * (texH - 1));
    int u2 = (int)(uvs[2][0] * (texW - 1)), v2 = (int)(uvs[2][1] * (texH - 1));
    
    int minX = x0 < x1 ? (x0 < x2 ? x0 : x2) : (x1 < x2 ? x1 : x2);
    int maxX = x0 > x1 ? (x0 > x2 ? x0 : x2) : (x1 > x2 ? x1 : x2);
    int minY = y0 < y1 ? (y0 < y2 ? y0 : y2) : (y1 < y2 ? y1 : y2);
    int maxY = y0 > y1 ? (y0 > y2 ? y0 : y2) : (y1 > y2 ? y1 : y2);

    if (minX < 0) minX = 0;
    if (minY < 0) minY = 0;
    if (maxX >= sW) maxX = sW - 1;
    if (maxY >= sH) maxY = sH - 1;

    minX = (minX / resolution) * resolution;
    minY = (minY / resolution) * resolution;
    maxX = ((maxX + resolution - 1) / resolution) * resolution;
    maxY = ((maxY + resolution - 1) / resolution) * resolution;
    
    int A01 = (y0 - y1) * resolution, B01 = (x1 - x0) * resolution;
    int A12 = (y1 - y2) * resolution, B12 = (x2 - x1) * resolution;
    int A20 = (y2 - y0) * resolution, B20 = (x0 - x2) * resolution;
    
    int cx = minX + (resolution >> 1);
    int cy = minY + (resolution >> 1);

    int w0_row = edgeFunc(x1, y1, x2, y2, cx, cy);
    int w1_row = edgeFunc(x2, y2, x0, y0, cx, cy);
    int w2_row = edgeFunc(x0, y0, x1, y1, cx, cy);

    int area = edgeFunc(x0, y0, x1, y1, x2, y2);
    if (area == 0) return;
    
    if (area < 0) {
        area = -area;

        int tmp;
        tmp = A01; A01 = -A01; B01 = -B01;
        tmp = A12; A12 = -A12; B12 = -B12;
        tmp = A20; A20 = -A20; B20 = -B20;

        w0_row = -w0_row;
        w1_row = -w1_row;
        w2_row = -w2_row;
    }

    int32_t w0, w1, w2;
    uint gridX, gridY;

    if ((maxX - minX) < resolution) maxX = minX + resolution;
    if ((maxY - minY) < resolution) maxY = minY + resolution;

    for (int y = minY; y <= maxY && y < sH; y += resolution) {
        gridY = (y & ~(resolution - 1));
        if (gridY >= sH) gridY = sH - 1;

        w0 = w0_row;
        w1 = w1_row;
        w2 = w2_row;

        for (int x = minX; x <= maxX && x < sW; x += resolution) {
            gridX = (x & ~(resolution - 1));
            if (gridX >= sW) gridX = sW - 1;

            if ((w0 | w1 | w2) >= 0) {
                int u = (int)(((int64_t)w0 * u0 + (int64_t)w1 * u1 + (int64_t)w2 * u2) / area);
                int v = (int)(((int64_t)w0 * v0 + (int64_t)w1 * v1 + (int64_t)w2 * v2) / area);

                if (u >= 0 && v >= 0 && u < texW && v < texH) {
                    int shade = texture[v * texW + u];
                    if (shade != -1) {
                        int gx = gridX / resolution;
                        int gy = gridY / resolution;

                        setPixScnBuf(gx, gy, shade);
                    }
                }
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

void drawLine(int x0, int y0, int x1, int y1, int8_t color) {
    x0 /= resolution;
    y0 /= resolution;
    x1 /= resolution;
    y1 /= resolution;

    int dx = x1 - x0;
    int dy = y1 - y0;

    int sx = (dx >= 0) ? 1 : -1;
    int sy = (dy >= 0) ? 1 : -1;

    dx = dx >= 0 ? dx : -dx;
    dy = dy >= 0 ? dy : -dy;

    int err = dx - dy;

    while (1) {
        if ((unsigned)x0 < sW_L && (unsigned)y0 < sH_L) {
            scnBuf[y0 * sW_L + x0] = color;
        }

        if (x0 == x1 && y0 == y1) break;

        int e2 = err << 1;

        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
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
        return 0;
    } else if (inAmt == 3) {
        outTri1->t1 = verts[0];
        outTri1->t2 = verts[1];
        outTri1->t3 = verts[2];
        return 1;
    } else if (inAmt == 1) {
        int in0 = inScreen[0];
        int out0 = outScreen[0];
        int out1 = outScreen[1];

        float t0 = (nearPlane - verts[out0].z) / (verts[in0].z - verts[out0].z);
        float t1 = (nearPlane - verts[out1].z) / (verts[in0].z - verts[out1].z);
        
        cross0.x = verts[out0].x + t0 * (verts[in0].x - verts[out0].x);
        cross0.y = verts[out0].y + t0 * (verts[in0].y - verts[out0].y);
        cross0.z = verts[out0].z + t0 * (verts[in0].z - verts[out0].z);

        cross1.x = verts[out1].x + t1 * (verts[in0].x - verts[out1].x);
        cross1.y = verts[out1].y + t1 * (verts[in0].y - verts[out1].y);
        cross1.z = verts[out1].z + t1 * (verts[in0].z - verts[out1].z);

        cross0.u = verts[out0].u + t0 * (verts[in0].u - verts[out0].u);
        cross0.v = verts[out0].v + t0 * (verts[in0].v - verts[out0].v);

        cross1.u = verts[out1].u + t1 * (verts[in0].u - verts[out1].u);
        cross1.v = verts[out1].v + t1 * (verts[in0].v - verts[out1].v);
        
        outTri1->t1 = verts[in0];
        outTri1->t2 = cross0;
        outTri1->t3 = cross1;

        return 1;
    } else if (inAmt == 2) {
        int in0 = inScreen[0];
        int in1 = inScreen[1];
        int out0 = outScreen[0];

        float t0 = (nearPlane - verts[out0].z) / (verts[in0].z - verts[out0].z);
        float t1 = (nearPlane - verts[out0].z) / (verts[in1].z - verts[out0].z);
        
        cross0.x = verts[out0].x + t0 * (verts[in0].x - verts[out0].x);
        cross0.y = verts[out0].y + t0 * (verts[in0].y - verts[out0].y);
        cross0.z = verts[out0].z + t0 * (verts[in0].z - verts[out0].z);

        cross1.x = verts[out0].x + t1 * (verts[in1].x - verts[out0].x);
        cross1.y = verts[out0].y + t1 * (verts[in1].y - verts[out0].y);
        cross1.z = verts[out0].z + t1 * (verts[in1].z - verts[out0].z);
        
        cross0.u = verts[out0].u + t0 * (verts[in0].u - verts[out0].u);
        cross0.v = verts[out0].v + t0 * (verts[in0].v - verts[out0].v);

        cross1.u = verts[out0].u + t1 * (verts[in1].u - verts[out0].u);
        cross1.v = verts[out0].v + t1 * (verts[in1].v - verts[out0].v);
        
        outTri1->t1 = verts[in0];
        outTri1->t2 = verts[in1];
        outTri1->t3 = cross0;

        outTri2->t1 = verts[in1];
        outTri2->t2 = cross1;
        outTri2->t3 = cross0;

        return 2;
    }

    return 0;
}