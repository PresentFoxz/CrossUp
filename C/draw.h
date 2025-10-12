#ifndef DRAW_H
#define DRAW_H
#include "library.h"

void project2D(int point[2], float verts[3], float fov, float nearPlane);
int windingOrder(int p0[2], int p1[2], int p2[2]);

int TriangleClipping(Vertex verts[3], clippedTri* outTri1, clippedTri* outTri2, float nearPlane, float farPlane);
void enforceCCW(float tri[3][3]);

void rotateVertex(float x, float y, float z, float rotMat[3][3], float out[3]);
void rotateVertexInPlace(Vertex* verts, Vect3f camPos, float rotMat[3][3]);
void computeCamMatrix(float m[3][3], float sinY, float cosY, float sinX, float cosX, float sinZ, float cosZ);
void computeRotScaleMatrix(float rotMat[3][3], float angleX, float angleY, float angleZ, float sx, float sy, float sz);
void RotationMatrix(float x, float y, float z, float sin1, float cos1, float sin2, float cos2, float sin3, float cos3, float rot[3]);

void staticLineDrawing(int p0[2], int p1[2], int color);

void drawFilledTrisZ(int tris[3][2], clippedTri fullTri, int triColor, qfixed16_t* zBuffer);
void drawFilledTrisNoZ(int tris[3][2], int triColor);

#endif