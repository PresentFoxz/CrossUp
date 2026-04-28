#ifndef _3DMATH_H
#define _3DMATH_H
#include "../allFiles/library.h"
#include "../allFiles/draw.h"

void project2D(int point[2], Vertex verts, float fov, float nearPlane);
int windingOrder2D(const int p0[2], const int p1[2], const int p2[2]);

int TriangleClipping(Vertex verts[3], clippedTri* outTri1, clippedTri* outTri2, float nearPlane, float farPlane);

void rotateVertex(Vector3f verts, float rotMat[3][3], Vector3f* vertsOut);
void rotateVertexInPlace(Vertex* verts, Vector3f camPos, float rotMat[3][3]);
void computeCamMatrix(float m[3][3], float pitchX, float yawY, float rollZ);
void computeRotScaleMatrix(float rotMat[3][3], float angleX, float angleY, float angleZ, float sx, float sy, float sz);

Vector3f computeNormal(Vector3f tri[3]);
void pushTri(Mesh_t* map, float x0,float y0,float z0, float x1,float y1,float z1, float x2,float y2,float z2, int wind, int color);

#endif