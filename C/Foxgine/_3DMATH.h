#ifndef _3DMATH_H
#define _3DMATH_H
#include "../allFiles/library.h"
#include "../allFiles/mesh.h"
#include "../allFiles/structs.h"
#include "../allFiles/draw.h"

void project2D(int point[2], Vertex verts, float fov, float nearPlane);
int windingOrder(const int p0[2], const int p1[2], const int p2[2]);

int TriangleClipping(Vertex verts[3], clippedTri* outTri1, clippedTri* outTri2, float nearPlane, float farPlane);
void enforceCCW(float tri[3][3]);

void rotateVertex(Vect3f verts, float rotMat[3][3], Vect3f* vertsOut);
void rotateVertexInPlace(Vertex* verts, Vect3f camPos, float rotMat[3][3]);
void computeCamMatrix(float m[3][3], float pitchX, float yawY, float rollZ);
void computeRotScaleMatrix(float rotMat[3][3], float angleX, float angleY, float angleZ, float sx, float sy, float sz);

void staticLineDrawing(int p0[2], int p1[2], int color);

void drawFilledTris(int tris[3][2], int triColor);
void drawTexturedTris(int tris[3][2], float uvs[3][2], int8_t* texture, int texW, int texH);
void drawImg(int screenX, int screenY, float depth, int tX, int tY, int tW, int tH, int8_t* texture, int texW, int texH, float projDist);
void drawImgNoScale(int x, int y, int tX, int tY, int tW, int tH, int8_t* texture, int texW, int texH);

int backfaceCullView(Vertex* verts, int flipped);
void drawLine(int x0, int y0, int x1, int y1, int8_t color);
void resShiftFix();

#endif