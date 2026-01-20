#include "collisions.h"
CollisionSurface* collisionSurfaces;
Triggers* triggers;

int triggerCount = 0;
int collisionCount = 0;

void resetCollisionSurface() {
    free(collisionSurfaces);
    collisionSurfaces = NULL;
    collisionCount = 0;
}

void resetTriggers() {
    free(triggers);
    triggers = NULL;
    triggerCount = 0;
}

void addTriggers(Vect3f pos, Vect3f size, int type, int id) {
    triggers = realloc(triggers, (triggerCount + 1) * sizeof(Triggers));

    Triggers* trig = &triggers[triggerCount++];
    trig->pos = pos;
    trig->size = size;
    trig->type = type;
    trig->id = id;
}

void addCollisionSurface(Vect3f v0, Vect3f v1, Vect3f v2, SurfaceType type) {
    collisionSurfaces = realloc(collisionSurfaces, (collisionCount + 1) * sizeof(CollisionSurface));

    CollisionSurface *surf = &collisionSurfaces[collisionCount++];
    surf->v0 = v0;
    surf->v1 = v1;
    surf->v2 = v2;

    surf->center.x = (v0.x + v1.x + v2.x) / 3.0f;
    surf->center.y = (v0.y + v1.y + v2.y) / 3.0f;
    surf->center.z = (v0.z + v1.z + v2.z) / 3.0f;

    float ux = v1.x - v0.x, uy = v1.y - v0.y, uz = v1.z - v0.z;
    float vx = v2.x - v0.x, vy = v2.y - v0.y, vz = v2.z - v0.z;

    surf->normal.x = uy * vz - uz * vy;
    surf->normal.y = uz * vx - ux * vz;
    surf->normal.z = ux * vy - uy * vx;

    float len = sqrtf(
        surf->normal.x * surf->normal.x +
        surf->normal.y * surf->normal.y +
        surf->normal.z * surf->normal.z
    );

    if (len < 0.0001f) {
        surf->normal = (Vect3f){0, 0, 0};
        surf->type = SURFACE_NONE;
        return;
    }

    surf->normal.x /= len;
    surf->normal.y /= len;
    surf->normal.z /= len;

    if (type == SURFACE_NONE) {
        float ny = surf->normal.y;
        if (ny > 0.7f)      surf->type = SURFACE_FLOOR;
        else if (ny < -0.7f) surf->type = SURFACE_CEILING;
        else                 surf->type = SURFACE_WALL;
    } else {
        surf->type = type;
    }
}

Triggers cylinderInTrigger(Vect3f pos, float radius, float height) {
    float radius2 = radius * radius;

    for (int i = 0; i < triggerCount; i++) {
        Triggers *trig = &triggers[i];
        
        float minY = trig->pos.y - trig->size.y * 0.5f;
        float maxY = trig->pos.y + trig->size.y * 0.5f;

        if (pos.y > maxY + height || pos.y + height < minY) continue;
        
        float minX = trig->pos.x - trig->size.x * 0.5f;
        float maxX = trig->pos.x + trig->size.x * 0.5f;
        float minZ = trig->pos.z - trig->size.z * 0.5f;
        float maxZ = trig->pos.z + trig->size.z * 0.5f;
        
        float closestX = pos.x < minX ? minX : (pos.x > maxX ? maxX : pos.x);
        float closestZ = pos.z < minZ ? minZ : (pos.z > maxZ ? maxZ : pos.z);

        float dx = pos.x - closestX;
        float dz = pos.z - closestZ;

        if ((dx * dx + dz * dz) > radius2) continue;

        return *trig;
    }

    return (Triggers){0};
}


VectMf cylinderInTriangle(Vect3f pos, float radius, float height) {
    VectMf pushPlayer = {0};
    float radius2 = radius * radius;

    for (int i = 0; i < collisionCount; i++) {
        CollisionSurface *tri = &collisionSurfaces[i];

        if (tri->normal.x == 0 && tri->normal.y == 0 && tri->normal.z == 0)
            continue;

        float minX = fminf(tri->v0.x, fminf(tri->v1.x, tri->v2.x));
        float maxX = fmaxf(tri->v0.x, fmaxf(tri->v1.x, tri->v2.x));
        float minZ = fminf(tri->v0.z, fminf(tri->v1.z, tri->v2.z));
        float maxZ = fmaxf(tri->v0.z, fmaxf(tri->v1.z, tri->v2.z));

        // PADDED broad-phase (important!)
        float pad = radius + 0.01f;
        if (pos.x + pad < minX || pos.x - pad > maxX || pos.z + pad < minZ || pos.z - pad > maxZ) continue;

        float dx = pos.x - tri->center.x;
        float dz = pos.z - tri->center.z;

        float bottomDist = dx * tri->normal.x + (pos.y - tri->center.y) * tri->normal.y + dz * tri->normal.z;
        float topDist = dx * tri->normal.x + (pos.y + height - tri->center.y) * tri->normal.y + dz * tri->normal.z;

        if (bottomDist > radius && topDist > radius) continue;
        if (bottomDist < -radius && topDist < -radius) continue;

        float t = 0.0f;
        if (bottomDist * topDist < 0.0f) t = bottomDist / (bottomDist - topDist);
        else t = (fabsf(bottomDist) < fabsf(topDist)) ? 0.0f : 1.0f;

        float px = pos.x;
        float py = pos.y + t * height;
        float pz = pos.z;

        float dist = (px - tri->center.x) * tri->normal.x + (py - tri->center.y) * tri->normal.y + (pz - tri->center.z) * tri->normal.z;

        if (dist * dist > radius2) continue;

        float projX = px - dist * tri->normal.x;
        float projY = py - dist * tri->normal.y;
        float projZ = pz - dist * tri->normal.z;

        float v0x = tri->v2.x - tri->v0.x;
        float v0y = tri->v2.y - tri->v0.y;
        float v0z = tri->v2.z - tri->v0.z;

        float v1x = tri->v1.x - tri->v0.x;
        float v1y = tri->v1.y - tri->v0.y;
        float v1z = tri->v1.z - tri->v0.z;

        float v2x = projX - tri->v0.x;
        float v2y = projY - tri->v0.y;
        float v2z = projZ - tri->v0.z;

        float dot00 = v0x*v0x + v0y*v0y + v0z*v0z;
        float dot01 = v0x*v1x + v0y*v1y + v0z*v1z;
        float dot11 = v1x*v1x + v1y*v1y + v1z*v1z;
        float dot02 = v0x*v2x + v0y*v2y + v0z*v2z;
        float dot12 = v1x*v2x + v1y*v2y + v1z*v2z;

        float denom = dot00 * dot11 - dot01 * dot01;
        if (fabsf(denom) < 0.00001f) continue;

        float invDenom = 1.0f / denom;
        float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
        float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

        if (u >= 0 && v >= 0 && (u + v <= 1)) {
            float penetration = radius - fabsf(dist);

            if (tri->type == OUT_OF_BOUNDS) {
                return (VectMf){0, 0, 0, -1, -1, -1};
            }

            switch (tri->type) {
                case SURFACE_FLOOR:
                case SURFACE_CEILING:
                    pushPlayer.pos.y += penetration * tri->normal.y;
                    pushPlayer.floor |= (tri->type == SURFACE_FLOOR);
                    pushPlayer.ceiling |= (tri->type == SURFACE_CEILING);
                    break;

                case SURFACE_WALL:
                    pushPlayer.pos.x += penetration * tri->normal.x;
                    pushPlayer.pos.z += penetration * tri->normal.z;
                    pushPlayer.wall = 1;
                    break;
            }
        }
    }

    return pushPlayer;
}