#include "collisions.h"
CollisionSurface* collisionSurfaces;
int collisionCount = 0;

void resetCollisionSurface() {
    free(collisionSurfaces);
    collisionSurfaces = NULL;
    collisionCount = 0;
}

void addCollisionSurface(Vect3f v0, Vect3f v1, Vect3f v2, SurfaceType type) {
    collisionSurfaces = pd->system->realloc(collisionSurfaces, (collisionCount + 1) * sizeof(CollisionSurface));

    CollisionSurface *surf = &collisionSurfaces[collisionCount++];
    surf->v0 = v0; surf->v1 = v1; surf->v2 = v2;
    surf->type = type;
    
    float ux = v1.x - v0.x, uy = v1.y - v0.y, uz = v1.z - v0.z;
    float vx = v2.x - v0.x, vy = v2.y - v0.y, vz = v2.z - v0.z;

    surf->normal.x = uy * vz - uz * vy;
    surf->normal.y = uz * vx - ux * vz;
    surf->normal.z = ux * vy - uy * vx;

    float len = sqrtf(surf->normal.x*surf->normal.x + surf->normal.y*surf->normal.y + surf->normal.z*surf->normal.z);
    if (len > 0.0f) {
        surf->normal.x /= len;
        surf->normal.y /= len;
        surf->normal.z /= len;
    }

    if (type == SURFACE_NONE) {
        float ny = surf->normal.y;

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

VectMf cylinderInTriangle(Vect3f pos, float radius, float height) {
    VectMf pushPlayer = {0.0f, 0.0f, 0.0f, 0, 0, 0};
    float radius2 = radius * radius;

    for (int i = 0; i < collisionCount; i++) {
        CollisionSurface tri = collisionSurfaces[i];

        float minX = tri.v0.x < tri.v1.x ? (tri.v0.x < tri.v2.x ? tri.v0.x : tri.v2.x) : (tri.v1.x < tri.v2.x ? tri.v1.x : tri.v2.x);
        float maxX = tri.v0.x > tri.v1.x ? (tri.v0.x > tri.v2.x ? tri.v0.x : tri.v2.x) : (tri.v1.x > tri.v2.x ? tri.v1.x : tri.v2.x);
        float minZ = tri.v0.z < tri.v1.z ? (tri.v0.z < tri.v2.z ? tri.v0.z : tri.v2.z) : (tri.v1.z < tri.v2.z ? tri.v1.z : tri.v2.z);
        float maxZ = tri.v0.z > tri.v1.z ? (tri.v0.z > tri.v2.z ? tri.v0.z : tri.v2.z) : (tri.v1.z > tri.v2.z ? tri.v1.z : tri.v2.z);
        
        if (pos.x + radius < minX || pos.x - radius > maxX || pos.z + radius < minZ || pos.z - radius > maxZ) continue;
        
        float dx = pos.x - tri.v0.x;
        float dz = pos.z - tri.v0.z;

        float bottomDist = dx * tri.normal.x + (pos.y - tri.v0.y) * tri.normal.y + dz * tri.normal.z;
        float topDist    = dx * tri.normal.x + (pos.y + height - tri.v0.y) * tri.normal.y + dz * tri.normal.z;
        
        if (bottomDist > radius && topDist > radius) continue;
        if (bottomDist < -radius && topDist < -radius) continue;
        
        float t = 0.0f;
        if (bottomDist * topDist < 0.0f) t = bottomDist / (bottomDist - topDist);
        else if (fabsf(bottomDist) >= radius) t = 1.0f;

        float py = pos.y + t * height;
        float px = pos.x;
        float pz = pos.z;

        float dist = (px - tri.v0.x) * tri.normal.x + (py - tri.v0.y) * tri.normal.y + (pz - tri.v0.z) * tri.normal.z;
        if (dist * dist > radius2) continue;
        
        float projX = px - dist * tri.normal.x;
        float projY = py - dist * tri.normal.y;
        float projZ = pz - dist * tri.normal.z;
        
        float v0x = tri.v2.x - tri.v0.x, v0y = tri.v2.y - tri.v0.y, v0z = tri.v2.z - tri.v0.z;
        float v1x = tri.v1.x - tri.v0.x, v1y = tri.v1.y - tri.v0.y, v1z = tri.v1.z - tri.v0.z;
        float v2x = projX - tri.v0.x, v2y = projY - tri.v0.y, v2z = projZ - tri.v0.z;

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
                    pushPlayer.pos.y += penetration * tri.normal.y;
                    pushPlayer.floor = 1;
                    break;
                case SURFACE_CEILING:
                    pushPlayer.pos.y += penetration * tri.normal.y;
                    pushPlayer.ceiling = 1;
                    break;
                case SURFACE_WALL:
                    pushPlayer.pos.x += penetration * tri.normal.x;
                    pushPlayer.pos.z += penetration * tri.normal.z;
                    pushPlayer.wall = 1;
                    break;
            }
        }
    }

    return pushPlayer;
}