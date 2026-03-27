#ifndef _3DMATH_H
#define _3DMATH_H
#include "../allFiles/library.h"
#include "../allFiles/draw.h"

void project2D(int point[2], Vertex verts, float fov, float nearPlane);
int windingOrder2D(const int p0[2], const int p1[2], const int p2[2]);
int windingOrder3D(const Vect3f* v0, const Vect3f* v1, const Vect3f* v2);

int TriangleClipping(Vertex verts[3], clippedTri* outTri1, clippedTri* outTri2, float nearPlane, float farPlane);

void rotateVertex(Vect3f verts, float rotMat[3][3], Vect3f* vertsOut);
void rotateVertexInPlace(Vertex* verts, Vect3f camPos, float rotMat[3][3]);
void computeCamMatrix(float m[3][3], float pitchX, float yawY, float rollZ);
void computeRotScaleMatrix(float rotMat[3][3], float angleX, float angleY, float angleZ, float sx, float sy, float sz);

#endif