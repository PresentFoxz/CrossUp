#include "collisions.h"
CollisionSurface* collisionSurfaces;
int collisionCount = 0;

void resetCollisionSurface() {
    collisionCount = 0;
    collisionSurfaces = pd->system->realloc(NULL, collisionCount);
}

void addCollisionSurface(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, SurfaceType type) {
    collisionSurfaces = pd->system->realloc(collisionSurfaces, (collisionCount + 1) * sizeof(CollisionSurface));

    CollisionSurface *surf = &collisionSurfaces[collisionCount++];
    surf->x1 = x1; surf->y1 = y1; surf->z1 = z1;
    surf->x2 = x2; surf->y2 = y2; surf->z2 = z2;
    surf->x3 = x3; surf->y3 = y3; surf->z3 = z3;
    surf->type = type;
    
    float ux = x2 - x1, uy = y2 - y1, uz = z2 - z1;
    float vx = x3 - x1, vy = y3 - y1, vz = z3 - z1;

    surf->normalX = uy * vz - uz * vy;
    surf->normalY = uz * vx - ux * vz;
    surf->normalZ = ux * vy - uy * vx;

    float len = sqrtf(surf->normalX*surf->normalX + surf->normalY*surf->normalY + surf->normalZ*surf->normalZ);
    if (len > 0.0f) {
        surf->normalX /= len;
        surf->normalY /= len;
        surf->normalZ /= len;
    }

    if (type == SURFACE_NONE) {
        float ny = surf->normalY;

        if (ny > 0.7f) {
            surf->type = SURFACE_FLOOR;
        } 
        else if (ny < -0.7f) {
            surf->type = SURFACE_CEILING;
        } 
        else {
            surf->type = SURFACE_WALL;
        }
    } else {
        surf->type = type;
    }
}

VectMf cylinderInTriangle(float cx, float cy, float cz, float radius, float height) {
    VectMf pushPlayer = {0.0f, 0.0f, 0.0f, 0, 0, 0};
    float radius2 = radius * radius;

    for (int i = 0; i < collisionCount; i++) {
        CollisionSurface tri = collisionSurfaces[i];

        float minX = tri.x1 < tri.x2 ? (tri.x1 < tri.x3 ? tri.x1 : tri.x3) : (tri.x2 < tri.x3 ? tri.x2 : tri.x3);
        float maxX = tri.x1 > tri.x2 ? (tri.x1 > tri.x3 ? tri.x1 : tri.x3) : (tri.x2 > tri.x3 ? tri.x2 : tri.x3);
        float minZ = tri.z1 < tri.z2 ? (tri.z1 < tri.z3 ? tri.z1 : tri.z3) : (tri.z2 < tri.z3 ? tri.z2 : tri.z3);
        float maxZ = tri.z1 > tri.z2 ? (tri.z1 > tri.z3 ? tri.z1 : tri.z3) : (tri.z2 > tri.z3 ? tri.z2 : tri.z3);
        
        if (cx + radius < minX || cx - radius > maxX || cz + radius < minZ || cz - radius > maxZ) continue;
        
        float dx = cx - tri.x1;
        float dz = cz - tri.z1;

        float bottomDist = dx * tri.normalX + (cy - tri.y1) * tri.normalY + dz * tri.normalZ;
        float topDist    = dx * tri.normalX + (cy + height - tri.y1) * tri.normalY + dz * tri.normalZ;
        
        if (bottomDist > radius && topDist > radius) continue;
        if (bottomDist < -radius && topDist < -radius) continue;
        
        float t = 0.0f;
        if (bottomDist * topDist < 0.0f) t = bottomDist / (bottomDist - topDist);
        else if (fabsf(bottomDist) >= radius) t = 1.0f;

        float py = cy + t * height;
        float px = cx;
        float pz = cz;

        float dist = (px - tri.x1) * tri.normalX + (py - tri.y1) * tri.normalY + (pz - tri.z1) * tri.normalZ;
        if (dist * dist > radius2) continue;
        
        float projX = px - dist * tri.normalX;
        float projY = py - dist * tri.normalY;
        float projZ = pz - dist * tri.normalZ;
        
        float v0x = tri.x3 - tri.x1, v0y = tri.y3 - tri.y1, v0z = tri.z3 - tri.z1;
        float v1x = tri.x2 - tri.x1, v1y = tri.y2 - tri.y1, v1z = tri.z2 - tri.z1;
        float v2x = projX - tri.x1, v2y = projY - tri.y1, v2z = projZ - tri.z1;

        float dot00 = v0x*v0x + v0y*v0y + v0z*v0z;
        float dot01 = v0x*v1x + v0y*v1y + v0z*v1z;
        float dot11 = v1x*v1x + v1y*v1y + v1z*v1z;
        float dot02 = v0x*v2x + v0y*v2y + v0z*v2z;
        float dot12 = v1x*v2x + v1y*v2y + v1z*v2z;

        float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
        float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
        float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

        if (u >= 0 && v >= 0 && (u + v <= 1)) {
            float penetration = radius - fabsf(dist);

            if (tri.type == OUT_OF_BOUNDS) {
                return (VectMf){0.0f, 0.0f, 0.0f, -1, -1, -1};
            }

            switch(tri.type) {
                case SURFACE_FLOOR:
                    pushPlayer.pos.y += penetration * tri.normalY;
                    pushPlayer.floor = 1;
                    break;
                case SURFACE_CEILING:
                    pushPlayer.pos.y += penetration * tri.normalY;
                    pushPlayer.cieling = 1;
                    break;
                case SURFACE_WALL:
                    pushPlayer.pos.x += penetration * tri.normalX;
                    pushPlayer.pos.z += penetration * tri.normalZ;
                    pushPlayer.wall = 1;
                    break;
            }
        }
    }

    return pushPlayer;
}