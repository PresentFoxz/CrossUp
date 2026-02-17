#include "_3DMath.h"
const float epsilon = 0.5f;

int RES_SHIFT = 0;
void resShiftFix() {
    RES_SHIFT = 0;
    int r = resolution;
    while (r > 1) {
        r >>= 1;
        RES_SHIFT++;
    }
}

void project2D(int point[2], Vertex verts, float fov, float nearPlane) {
    float z = verts.z;
    if (z < nearPlane + epsilon) z = nearPlane + epsilon;

    float invZ = 1.0f / z;
    float scale = fov * invZ;
    
    float sx = sW_H + sX;
    float sy = sH_H + sY;

    point[0] = (int)(verts.x * scale + sx);
    point[1] = (int)(verts.y * -scale + sy);
}


void rotateVertexInPlace(Vertex* v, Vect3f camPos, float camMatrix[3][3]) {
    float x = (v->x - camPos.x); float y = (v->y - camPos.y); float z = (v->z - camPos.z);

    v->x = x * camMatrix[0][0] + y * camMatrix[1][0] + z * camMatrix[2][0];
    v->y = x * camMatrix[0][1] + y * camMatrix[1][1] + z * camMatrix[2][1];
    v->z = x * camMatrix[0][2] + y * camMatrix[1][2] + z * camMatrix[2][2];
}

void rotateVertex(Vect3f verts, float rotMat[3][3], Vect3f* vertsOut) {
    vertsOut->x = verts.x * rotMat[0][0] + verts.y * rotMat[0][1] + verts.z * rotMat[0][2];
    vertsOut->y = verts.x * rotMat[1][0] + verts.y * rotMat[1][1] + verts.z * rotMat[1][2];
    vertsOut->z = verts.x * rotMat[2][0] + verts.y * rotMat[2][1] + verts.z * rotMat[2][2];
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

static inline qfixed32_t edgeFuncF(qfixed32_t x0, qfixed32_t y0, qfixed32_t x1, qfixed32_t y1, qfixed32_t px, qfixed32_t py) { return (py - y0) * (x1 - x0) - (px - x0) * (y1 - y0); }

static inline void drawSpanLowRes(int y, int xStart, int xEnd, int color) {
    if (y < 0 || y >= sH_L) return;

    if (xStart > xEnd) {
        int t = xStart;
        xStart = xEnd;
        xEnd = t;
    }

    if (xEnd < 0 || xStart >= sW_L) return;

    if (xStart < 0) xStart = 0;
    if (xEnd >= sW_L) xEnd = sW_L - 1;

    for (int x = xStart; x <= xEnd; x++) {
        setPixScnBuf(x, y, color);
    }
}

void drawFilledTris(int tris[3][2], int triColor) {
    int x0 = tris[0][0] / resolution;
    int y0 = tris[0][1] / resolution;
    int x1 = tris[1][0] / resolution;
    int y1 = tris[1][1] / resolution;
    int x2 = tris[2][0] / resolution;
    int y2 = tris[2][1] / resolution;

    if (y0 > y1) { swapInt(&y0, &y1); swapInt(&x0, &x1); }
    if (y1 > y2) { swapInt(&y1, &y2); swapInt(&x1, &x2); }
    if (y0 > y1) { swapInt(&y0, &y1); swapInt(&x0, &x1); }

    if (y0 == y2) return;

    int totalHeight = y2 - y0;

    qfixed16_t dx02 = TO_FIXED16(x2 - x0);
    qfixed16_t dx01 = TO_FIXED16(x1 - x0);
    qfixed16_t dx12 = TO_FIXED16(x2 - x1);

    qfixed16_t step02 = totalHeight ? dx02 / totalHeight : 0;
    qfixed16_t step01 = (y1 - y0) ? dx01 / (y1 - y0) : 0;
    qfixed16_t step12 = (y2 - y1) ? dx12 / (y2 - y1) : 0;

    qfixed16_t curx02 = TO_FIXED16(x0);
    qfixed16_t curx01 = TO_FIXED16(x0);
    qfixed16_t curx12 = TO_FIXED16(x1);

    for (int y = y0; y < y1; y++) {
        drawSpanLowRes(y, FROM_FIXED16(curx01), FROM_FIXED16(curx02), triColor);
        curx01 += step01;
        curx02 += step02;
    }

    for (int y = y1; y <= y2; y++) {
        drawSpanLowRes(y, FROM_FIXED16(curx12), FROM_FIXED16(curx02), triColor);
        curx12 += step12;
        curx02 += step02;
    }
}


void drawTexturedTris(int tris[3][2], float uvs[3][2], int8_t* texture, int texW, int texH) {
    qfixed32_t x0 = TO_FIXED32(tris[0][0]), y0 = TO_FIXED32(tris[0][1]);
    qfixed32_t x1 = TO_FIXED32(tris[1][0]), y1 = TO_FIXED32(tris[1][1]);
    qfixed32_t x2 = TO_FIXED32(tris[2][0]), y2 = TO_FIXED32(tris[2][1]);

    qfixed32_t u0 = TO_FIXED32(uvs[0][0] * (texW - 1));
    qfixed32_t v0 = TO_FIXED32(uvs[0][1] * (texH - 1));
    qfixed32_t u1 = TO_FIXED32(uvs[1][0] * (texW - 1));
    qfixed32_t v1 = TO_FIXED32(uvs[1][1] * (texH - 1));
    qfixed32_t u2 = TO_FIXED32(uvs[2][0] * (texW - 1));
    qfixed32_t v2 = TO_FIXED32(uvs[2][1] * (texH - 1));

    qfixed32_t minX = x0 < x1 ? (x0 < x2 ? x0 : x2) : (x1 < x2 ? x1 : x2);
    qfixed32_t maxX = x0 > x1 ? (x0 > x2 ? x0 : x2) : (x1 > x2 ? x1 : x2);
    qfixed32_t minY = y0 < y1 ? (y0 < y2 ? y0 : y2) : (y1 < y2 ? y1 : y2);
    qfixed32_t maxY = y0 > y1 ? (y0 > y2 ? y0 : y2) : (y1 > y2 ? y1 : y2);
    
    if (minX < 0) minX = 0;
    if (minY < 0) minY = 0;
    if (maxX >= TO_FIXED32(sW)) maxX = TO_FIXED32(sW - 1);
    if (maxY >= TO_FIXED32(sH)) maxY = TO_FIXED32(sH - 1);

    qfixed32_t A01 = TO_FIXED32((y0 - y1) * resolution), B01 = TO_FIXED32((x1 - x0) * resolution);
    qfixed32_t A12 = TO_FIXED32((y1 - y2) * resolution), B12 = TO_FIXED32((x2 - x1) * resolution);
    qfixed32_t A20 = TO_FIXED32((y2 - y0) * resolution), B20 = TO_FIXED32((x0 - x2) * resolution);

    qfixed32_t w0_row = edgeFuncF(x1, y1, x2, y2, minX, minY);
    qfixed32_t w1_row = edgeFuncF(x2, y2, x0, y0, minX, minY);
    qfixed32_t w2_row = edgeFuncF(x0, y0, x1, y1, minX, minY);

    qfixed32_t area = edgeFuncF(x0, y0, x1, y1, x2, y2);
    if (area == 0) return;

    if (area < 0) {
        A01 = -A01; B01 = -B01;
        A12 = -A12; B12 = -B12;
        A20 = -A20; B20 = -B20;
        w0_row = -w0_row;
        w1_row = -w1_row;
        w2_row = -w2_row;
    }

    qfixed32_t areaFP = TO_FIXED32(area);
    qfixed32_t step = TO_FIXED32(resolution);

    for (qfixed32_t y = minY; y <= maxY; y += step) {
        qfixed32_t w0 = w0_row;
        qfixed32_t w1 = w1_row;
        qfixed32_t w2 = w2_row;

        for (qfixed32_t x = minX; x <= maxX; x += step) {
            if ((w0 | w1 | w2) >= 0) {
                int64_t u_acc = (int64_t)multiply32(w0, u0) + multiply32(w1, u1) + multiply32(w2, u2);
                int64_t v_acc = (int64_t)multiply32(w0, v0) + multiply32(w1, v1) + multiply32(w2, v2);

                qfixed32_t u = divide32(u_acc, areaFP);
                qfixed32_t v = divide32(v_acc, areaFP);

                int iu = FROM_FIXED32(u);
                int iv = FROM_FIXED32(v);

                if (iu < 0) iu = 0;
                if (iv < 0) iv = 0;
                if (iu >= texW) iu = texW - 1;
                if (iv >= texH) iv = texH - 1;

                int gx = FROM_FIXED32(x) / resolution;
                int gy = FROM_FIXED32(y) / resolution;
                
                if (gx < 0) gx = 0;
                if (gy < 0) gy = 0;
                if (gx >= sW_L) gx = sW_L - 1;
                if (gy >= sH_L) gy = sH_L - 1;

                int8_t shade = texture[iv * texW + iu];
                if (shade != -1) setPixScnBuf(gx, gy, shade);
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
    x0 >>= RES_SHIFT;
    y0 >>= RES_SHIFT;
    x1 >>= RES_SHIFT;
    y1 >>= RES_SHIFT;

    int dx = x1 - x0;
    int dy = y1 - y0;

    int sx = (dx >= 0) ? 1 : -1;
    int sy = (dy >= 0) ? 1 : -1;
    
    dx = dx >= 0 ? dx : -dx;
    dy = dy >= 0 ? dy : -dy;
    
    int err = dx - dy;

    int index = y0 * sW_L + x0;

    while (1) {
        if ((unsigned)x0 < sW_L && (unsigned)y0 < sH_L) scnBuf[index] = color;

        if (x0 == x1 && y0 == y1) break;

        int e2 = err << 1;

        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
            index += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
            index += sy * sW_L;
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

    if (inAmt == 0) return 0;

    if (inAmt == 3) {
        *outTri1 = (clippedTri){verts[0], verts[1], verts[2]};
        return 1;
    }

    int in0 = inScreen[0];
    int in1 = inScreen[1];
    int out0 = outScreen[0];
    int out1 = outScreen[1];

    if (inAmt == 1) {
        float plane0 = (verts[out0].z < nearPlane) ? nearPlane : farPlane;
        float plane1 = (verts[out1].z < nearPlane) ? nearPlane : farPlane;

        float t0 = (plane0 - verts[out0].z) / (verts[in0].z - verts[out0].z);
        float t1 = (plane1 - verts[out1].z) / (verts[in0].z - verts[out1].z);

        cross0.x = verts[out0].x + t0 * (verts[in0].x - verts[out0].x);
        cross0.y = verts[out0].y + t0 * (verts[in0].y - verts[out0].y);
        cross0.z = verts[out0].z + t0 * (verts[in0].z - verts[out0].z);

        cross1.x = verts[out1].x + t1 * (verts[in0].x - verts[out1].x);
        cross1.y = verts[out1].y + t1 * (verts[in0].y - verts[out1].y);
        cross1.z = verts[out1].z + t1 * (verts[in0].z - verts[out1].z);

        *outTri1 = (clippedTri){verts[in0], cross0, cross1};
        return 1;
    }
    
    if (inAmt == 2) {
        float plane = (verts[out0].z < nearPlane) ? nearPlane : farPlane;

        float t0 = (plane - verts[out0].z) / (verts[in0].z - verts[out0].z);
        float t1 = (plane - verts[out0].z) / (verts[in1].z - verts[out0].z);

        cross0.x = verts[out0].x + t0 * (verts[in0].x - verts[out0].x);
        cross0.y = verts[out0].y + t0 * (verts[in0].y - verts[out0].y);
        cross0.z = verts[out0].z + t0 * (verts[in0].z - verts[out0].z);

        cross1.x = verts[out0].x + t1 * (verts[in1].x - verts[out0].x);
        cross1.y = verts[out0].y + t1 * (verts[in1].y - verts[out0].y);
        cross1.z = verts[out0].z + t1 * (verts[in1].z - verts[out0].z);

        *outTri1 = (clippedTri){verts[in0], verts[in1], cross0};
        *outTri2 = (clippedTri){verts[in1], cross1, cross0};
        return 2;
    }

    return 0;
}

void drawImg(int screenX, int screenY, float depth, int tX, int tY, int tW, int tH, int8_t* texture, int texW, int texH, float projDist) {
    int minX = tW < tX ? tW : tX;
    int maxX = tW > tX ? tW : tX;
    int minY = tH < tY ? tH : tY;
    int maxY = tH > tY ? tH : tY;

    if (minX >= texW || minY >= texH) return;
    if (maxX >= texW) maxX = texW - 1;
    if (maxY >= texH) maxY = texH - 1;

    if (depth < 0.001f) depth = 0.001f;
    float scale = (projDist / depth);

    int spriteW = maxX - minX + 1;
    int spriteH = maxY - minY + 1;

    if (spriteW <= 0 || spriteH <= 0) return;

    int scaledW = (int)(spriteW * scale);
    int scaledH = (int)(spriteH * scale);

    if (scaledW <= 0 || scaledH <= 0) return;

    int drawX = screenX - scaledW / 2.0f;
    int drawY = screenY - scaledH / 1.2f;

    for (int j = 0; j < scaledH; j++) {
        for (int i = 0; i < scaledW; i++) {
            int gx = (drawX + i) / resolution;
            int gy = (drawY + j) / resolution;

            if (gx < 0 || gy < 0 || gx >= sW_L || gy >= sH_L) continue;

            int texX = minX + (i * spriteW) / scaledW;
            int texY = minY + (j * spriteH) / scaledH;

            int8_t color = texture[texY * texW + texX];
            if (color != -1) setPixScnBuf(gx, gy, color);
        }
    }
}

void drawImgNoScale(int x, int y, int tX, int tY, int tW, int tH, int8_t* texture, int texW, int texH) {
    if (tX >= texW || tY >= texH) return;
    if (tW >= texW) tW = texW - 1;
    if (tH >= texH) tH = texH - 1;

    int regionW = tW - tX;
    int regionH = tH - tY;

    int drawX = x - regionW / 2;
    int drawY = y - regionH / 2;
    
    for (int j = 0; j < regionH; j++) {
        for (int i = 0; i < regionW; i++) {
            int texX = (tX + i);
            int texY = (tY + j);

            int8_t color = texture[texY * texW + texX];
            int gx = drawX + i;
            int gy = drawY + j;

            if (gx < 0 || gy < 0 || gx >= sW_L || gy >= sH_L) continue;
            setPixScnBuf(gx / resolution, gy / resolution, color);
        }
    }
}