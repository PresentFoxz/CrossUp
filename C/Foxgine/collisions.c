#include "collisions.h"
CollisionChunks* collisionChunkSurfaces;
Triggers* triggers;

#define CHUNK_SIZE 50
#define TRI_EPSILON 0

int triggerCount = 0;
int chunkAmt = 0;

int minX; int minY; int minZ;
int maxX; int maxY; int maxZ;

static inline int getChunkPos(int dot) {
    return (dot / CHUNK_SIZE);
}

void resetCollisionSurface(Mesh_t mapArray) {
    collisionChunkSurfaces = NULL;
    chunkAmt = 0;

    Vect3f v0 = mapArray.verts[0];
    Vect3f v1 = mapArray.verts[1];
    Vect3f v2 = mapArray.verts[2];

    minX = (int)floorf(fminf(v0.x, fminf(v1.x, v2.x)));
    minY = (int)floorf(fminf(v0.y, fminf(v1.y, v2.y)));
    minZ = (int)floorf(fminf(v0.z, fminf(v1.z, v2.z)));

    maxX = (int)ceilf (fmaxf(v0.x, fmaxf(v1.x, v2.x)));
    maxY = (int)ceilf (fmaxf(v0.y, fmaxf(v1.y, v2.y)));
    maxZ = (int)ceilf (fmaxf(v0.z, fmaxf(v1.z, v2.z)));

    for (int i=0; i < mapArray.triCount; i++){
        int* tris = mapArray.tris[i];
        v0 = mapArray.verts[tris[0]];
        v1 = mapArray.verts[tris[1]];
        v2 = mapArray.verts[tris[2]];

        int minX_ = (int)(fminf(v0.x, fminf(v1.x, v2.x)));
        int minY_ = (int)(fminf(v0.y, fminf(v1.y, v2.y)));
        int minZ_ = (int)(fminf(v0.z, fminf(v1.z, v2.z)));

        int maxX_ = (int)(fmaxf(v0.x, fmaxf(v1.x, v2.x)));
        int maxY_ = (int)(fmaxf(v0.y, fmaxf(v1.y, v2.y)));
        int maxZ_ = (int)(fmaxf(v0.z, fmaxf(v1.z, v2.z)));

        if (minX_ < minX) minX = minX_;
        if (minY_ < minY) minY = minY_;
        if (minZ_ < minZ) minZ = minZ_;

        if (maxX_ > maxX) maxX = maxX_;
        if (maxY_ > maxY) maxY = maxY_;
        if (maxZ_ > maxZ) maxZ = maxZ_;
    }

    int minChunkX = getChunkPos((int)floorf(minX));
    int minChunkY = getChunkPos((int)floorf(minY));
    int minChunkZ = getChunkPos((int)floorf(minZ));

    int maxChunkX = getChunkPos((int)floorf(maxX));
    int maxChunkY = getChunkPos((int)floorf(maxY));
    int maxChunkZ = getChunkPos((int)floorf(maxZ));

    for (int cx = minChunkX; cx <= maxChunkX; cx++) {
        for (int cy = minChunkY; cy <= maxChunkY; cy++) {
            for (int cz = minChunkZ; cz <= maxChunkZ; cz++) {
                collisionChunkSurfaces = pd_realloc(collisionChunkSurfaces, sizeof(CollisionChunks) * (chunkAmt + 1));
                collisionChunkSurfaces[chunkAmt].pos = (Vect3f){cx, cy, cz};
                collisionChunkSurfaces[chunkAmt].amt = 0;
                collisionChunkSurfaces[chunkAmt].collisions = NULL;

                chunkAmt++;
            }
        }
    }
}

void resetTriggers() {
    triggers = NULL;
    triggerCount = 0;
}

void addTriggers(Vect3f pos, Vect3f size, int type, int id) {
    triggers = pd_realloc(triggers, (triggerCount + 1) * sizeof(Triggers));

    Triggers* trig = &triggers[triggerCount++];
    trig->pos = pos;
    trig->size = size;
    trig->type = type;
    trig->id = id;
}

void addCollisionSurface(Vect3f v0, Vect3f v1, Vect3f v2, Vect3f normal, SurfaceType type) {
    CollisionSurface surf;

    surf.v0 = v0;
    surf.v1 = v1;
    surf.v2 = v2;

    surf.center.x = (v0.x + v1.x + v2.x) / 3.0f;
    surf.center.y = (v0.y + v1.y + v2.y) / 3.0f;
    surf.center.z = (v0.z + v1.z + v2.z) / 3.0f;
    surf.normal = normal;

    float ux = v1.x - v0.x, uy = v1.y - v0.y, uz = v1.z - v0.z;
    float vx = v2.x - v0.x, vy = v2.y - v0.y, vz = v2.z - v0.z;

    if (type == SURFACE_NONE) {
        float ny = surf.normal.y;
        if (ny > 0.7f)      surf.type = SURFACE_FLOOR;
        else if (ny < -0.7f) surf.type = SURFACE_CEILING;
        else                 surf.type = SURFACE_WALL;
    } else {
        surf.type = type;
    }

    surf.minX = (int)(fminf(v0.x, fminf(v1.x, v2.x)));
    surf.minY = (int)(fminf(v0.y, fminf(v1.y, v2.y)));
    surf.minZ = (int)(fminf(v0.z, fminf(v1.z, v2.z)));

    surf.maxX = (int)(fmaxf(v0.x, fmaxf(v1.x, v2.x)));
    surf.maxY = (int)(fmaxf(v0.y, fmaxf(v1.y, v2.y)));
    surf.maxZ = (int)(fmaxf(v0.z, fmaxf(v1.z, v2.z)));

    int minChunkX = getChunkPos(surf.minX);
    int minChunkY = getChunkPos(surf.minY);
    int minChunkZ = getChunkPos(surf.minZ);

    int maxChunkX = getChunkPos(surf.maxX);
    int maxChunkY = getChunkPos(surf.maxY);
    int maxChunkZ = getChunkPos(surf.maxZ);

    surf.v0x = v2.x - v0.x;
    surf.v0y = v2.y - v0.y;
    surf.v0z = v2.z - v0.z;

    surf.v1x = v1.x - v0.x;
    surf.v1y = v1.y - v0.y;
    surf.v1z = v1.z - v0.z;

    surf.dot00 = surf.v0x*surf.v0x + surf.v0y*surf.v0y + surf.v0z*surf.v0z;
    surf.dot01 = surf.v0x*surf.v1x + surf.v0y*surf.v1y + surf.v0z*surf.v1z;
    surf.dot11 = surf.v1x*surf.v1x + surf.v1y*surf.v1y + surf.v1z*surf.v1z;

    float denom = surf.dot00 * surf.dot11 - surf.dot01 * surf.dot01;
    if (fabsf(denom) < 0.00001f) { surf.invDenom = 0.0f; }
    else { surf.invDenom = 1.0f / denom; }

    for (int c=0; c < chunkAmt; c++) {
        int cx = collisionChunkSurfaces[c].pos.x;
        int cy = collisionChunkSurfaces[c].pos.y;
        int cz = collisionChunkSurfaces[c].pos.z;

        if (cx >= minChunkX && cx <= maxChunkX && cy >= minChunkY && cy <= maxChunkY && cz >= minChunkZ && cz <= maxChunkZ) {
            collisionChunkSurfaces[c].collisions = pd_realloc(collisionChunkSurfaces[c].collisions, sizeof(CollisionSurface) * (collisionChunkSurfaces[c].amt + 1));
            collisionChunkSurfaces[c].collisions[collisionChunkSurfaces[c].amt++] = surf;
        }
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
    if ((pos.x < minX || pos.x >= maxX) || (pos.y < minY || pos.y >= maxY) || (pos.z < minZ || pos.z >= maxZ)) { return (VectMf){0, 0, 0, -1, -1, -1}; }

    VectMf pushPlayer = {0};
    float effectiveRadius = radius + TRI_EPSILON;
    float radius2 = effectiveRadius * effectiveRadius;
    float pad = radius + 0.5f;

    float px = pos.x;
    float pz = pos.z;
    float pyBottom = pos.y;
    float pyTop    = pos.y + height;

    for (int c=0; c < chunkAmt; c++){
        int chunkPos[3] = {getChunkPos(pos.x), getChunkPos(pos.y), getChunkPos(pos.z)};
        if ((chunkPos[0] != collisionChunkSurfaces[c].pos.x) || (chunkPos[1] != collisionChunkSurfaces[c].pos.y) || (chunkPos[2] != collisionChunkSurfaces[c].pos.z)) continue;
        if (collisionChunkSurfaces[c].amt == 0) continue;
        
        CollisionSurface* collSurface = collisionChunkSurfaces[c].collisions;
        for (int i = 0; i < collisionChunkSurfaces[c].amt; i++) {
            CollisionSurface *tri = &collSurface[i];

            if (tri->normal.x == 0 && tri->normal.y == 0 && tri->normal.z == 0) continue;
            if (pos.x + pad < tri->minX || pos.x - pad > tri->maxX || pos.y + pad < tri->minY || pos.y - pad > tri->maxY || pos.z + pad < tri->minZ || pos.z - pad > tri->maxZ) continue;

            float dx = pos.x - tri->center.x;
            float dz = pos.z - tri->center.z;

            float bottomDist = dx * tri->normal.x + (pos.y - tri->center.y) * tri->normal.y + dz * tri->normal.z;
            float topDist = dx * tri->normal.x + ((pos.y + height) - tri->center.y) * tri->normal.y + dz * tri->normal.z;

            if (bottomDist > radius && topDist > radius) continue;
            if (bottomDist < -radius && topDist < -radius) continue;

            float t = 0.0f;
            if (bottomDist * topDist < 0.0f) t = bottomDist / (bottomDist - topDist);
            else t = (fabsf(bottomDist) < fabsf(topDist)) ? 0.0f : 1.0f;
            
            for (int h = 0; h < 2; h++) {
                float py = (h == 0) ? pyBottom : pyTop;
                float dist = (px - tri->center.x) * tri->normal.x + (py - tri->center.y) * tri->normal.y + (pz - tri->center.z) * tri->normal.z;
                if (dist * dist > radius2) continue;
                
                float projX = px - dist * tri->normal.x;
                float projY = py - dist * tri->normal.y;
                float projZ = pz - dist * tri->normal.z;
                
                float v2x = projX - tri->v0.x;
                float v2y = projY - tri->v0.y;
                float v2z = projZ - tri->v0.z;

                float dot02 = tri->v0x * v2x + tri->v0y * v2y + tri->v0z * v2z;
                float dot12 = tri->v1x * v2x + tri->v1y * v2y + tri->v1z * v2z;

                float u = (tri->dot11 * dot02 - tri->dot01 * dot12) * tri->invDenom;
                float v = (tri->dot00 * dot12 - tri->dot01 * dot02) * tri->invDenom;

                if (u < -TRI_EPSILON || v < -TRI_EPSILON || (u + v > 1.0f + TRI_EPSILON)) continue;

                float penetration = radius - (dist < 0 ? -dist : dist);
                if (tri->type == OUT_OF_BOUNDS) return (VectMf){0, 0, 0, -1, -1, -1};

                if (tri->type == SURFACE_FLOOR && h == 0 && dist < radius) {
                    pushPlayer.pos.y += penetration;
                    pushPlayer.floor = 1;
                } 
                else if (tri->type == SURFACE_CEILING && h == 1 && dist > -radius) {
                    pushPlayer.pos.y -= penetration; // only top
                    pushPlayer.ceiling = 1;
                } 
                else if (tri->type == SURFACE_WALL && fabsf(dist) < radius) {
                    pushPlayer.pos.x += penetration * tri->normal.x;
                    pushPlayer.pos.z += penetration * tri->normal.z;
                    pushPlayer.wall = 1;
                }
            }
        }

        break;
    }

    return pushPlayer;
}