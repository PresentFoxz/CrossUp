#include "_3DMath.h"
const float epsilon = 0.5f;

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

void rotateVertexInPlace(Vertex* v, Vector3f camPos, float camMatrix[3][3]) {
    float x = (v->x - camPos.x); float y = (v->y - camPos.y); float z = (v->z - camPos.z);

    v->x = x * camMatrix[0][0] + y * camMatrix[1][0] + z * camMatrix[2][0];
    v->y = x * camMatrix[0][1] + y * camMatrix[1][1] + z * camMatrix[2][1];
    v->z = x * camMatrix[0][2] + y * camMatrix[1][2] + z * camMatrix[2][2];
}

void rotateVertex(Vector3f verts, float rotMat[3][3], Vector3f* vertsOut) {
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

int windingOrder2D(const int p0[2], const int p1[2], const int p2[2]) {
    long cross = (long)p0[0]*p1[1] - (long)p0[1]*p1[0] + (long)p1[0]*p2[1] - (long)p1[1]*p2[0] + (long)p2[0]*p0[1] - (long)p2[1]*p0[0];
    return cross > 0;
}

int windingOrder3D(const Vector3f* v0, const Vector3f* v1, const Vector3f* v2) {
    long cross = (long)v0->x * (long)v1->y - (long)v0->y * (long)v1->x + (long)v1->x * (long)v2->y - (long)v1->y * (long)v2->x + (long)v2->x * (long)v0->y - (long)v2->y * (long)v0->x;
    return cross > 0;
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

Vector3f computeNormal(Vector3f tri[3]) {
    Vector3f edge1, edge2;
    edge1.x = tri[1].x - tri[0].x;
    edge1.y = tri[1].y - tri[0].y;
    edge1.z = tri[1].z - tri[0].z;

    edge2.x = tri[2].x - tri[0].x;
    edge2.y = tri[2].y - tri[0].y;
    edge2.z = tri[2].z - tri[0].z;
    
    Vector3f normal;
    normal.x = edge1.y * edge2.z - edge1.z * edge2.y;
    normal.y = edge1.z * edge2.x - edge1.x * edge2.z;
    normal.z = edge1.x * edge2.y - edge1.y * edge2.x;
    
    float len = sqrtf(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
    if (len != 0.0f) {
        normal.x /= len;
        normal.y /= len;
        normal.z /= len;
    }

    return normal;
}

const static int maxColor = 31;
void pushTri(Mesh_t* map, float x0,float y0,float z0, float x1,float y1,float z1, float x2,float y2,float z2, int wind, int color) {
    int base = map->vertCount;

    if (wind == -1) {
        float tx = x0, ty = y0, tz = z0;
        x0 = x2; y0 = y2; z0 = z2;
        x2 = tx; y2 = ty; z2 = tz;
    }
    map->verts = pd_realloc(map->verts, sizeof(Vector3f) * (base + 3));
    map->verts[base + 0] = (Vector3f){x0,y0,-z0};
    map->verts[base + 1] = (Vector3f){x1,y1,-z1};
    map->verts[base + 2] = (Vector3f){x2,y2,-z2};
    map->vertCount += 3;

    int triIndex = map->triCount;
    map->tris = pd_realloc(map->tris, sizeof(int[3]) * (triIndex + 1));
    map->tris[triIndex][0] = base + 0;
    map->tris[triIndex][1] = base + 1;
    map->tris[triIndex][2] = base + 2;

    Vector3f face[3] = {
        {map->verts[map->tris[triIndex][0]].x, map->verts[map->tris[triIndex][0]].y, map->verts[map->tris[triIndex][0]].z},
        {map->verts[map->tris[triIndex][1]].x, map->verts[map->tris[triIndex][1]].y, map->verts[map->tris[triIndex][1]].z},
        {map->verts[map->tris[triIndex][2]].x, map->verts[map->tris[triIndex][2]].y, map->verts[map->tris[triIndex][2]].z},
    };

    map->bfc = pd_realloc(map->bfc, sizeof(int) * (triIndex + 1));
    map->color = pd_realloc(map->color, sizeof(uint8_t) * (triIndex + 1));
    map->normal = pd_realloc(map->normal, sizeof(Vector3f) * (triIndex + 1));

    if (wind == 0) { map->bfc[triIndex] = 0; } else { map->bfc[triIndex] = 1; }
    map->normal[triIndex] = computeNormal(face);
    
    int c = color;
    if (c < 0) c = 0;
    if (c > maxColor) c = maxColor;
    map->color[triIndex] = (uint8_t)((c * 255) / maxColor);

    map->triCount++;
}