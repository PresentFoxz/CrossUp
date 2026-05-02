#include "collisions.h"
CollisionChunks* collisionChunkSurfaces;
Triggers* triggers;

#define CHUNK_SIZE 50
#define TRI_EPSILON 0
#define FIX_BORDER 40

int triggerCount = 0;
int chunkAmt = 0;

int minX_Stored = INT_MAX; int minY_Stored = INT_MAX; int minZ_Stored = INT_MAX;
int maxX_Stored = INT_MIN; int maxY_Stored = INT_MIN; int maxZ_Stored = INT_MIN;

static inline int getChunkPos(int dot) {
    return (dot / CHUNK_SIZE);
}

void resetCollisionSurface() {
    collisionChunkSurfaces = NULL;
    chunkAmt = 0;
}

void fixSurfaces(Mesh_t mapArray, Vector2f pos) {
    int minX = INT_MAX, minY = INT_MAX, minZ = INT_MAX;
    int maxX = INT_MIN, maxY = INT_MIN, maxZ = INT_MIN;

    for (int i = 0; i < mapArray.triCount; i++) {
        int* tris = mapArray.tris[i];

        Vector3f v0 = mapArray.verts[tris[0]];
        Vector3f v1 = mapArray.verts[tris[1]];
        Vector3f v2 = mapArray.verts[tris[2]];

        v0.x += pos.x;
        v1.x += pos.x;
        v2.x += pos.x;

        v0.z += pos.z;
        v1.z += pos.z;
        v2.z += pos.z;

        int minX_ = (int)floorf(fminf(v0.x, fminf(v1.x, v2.x)));
        int minY_ = (int)floorf(fminf(v0.y, fminf(v1.y, v2.y)));
        int minZ_ = (int)floorf(fminf(v0.z, fminf(v1.z, v2.z)));

        int maxX_ = (int)ceilf(fmaxf(v0.x, fmaxf(v1.x, v2.x)));
        int maxY_ = (int)ceilf(fmaxf(v0.y, fmaxf(v1.y, v2.y)));
        int maxZ_ = (int)ceilf(fmaxf(v0.z, fmaxf(v1.z, v2.z)));

        if (minX_ < minX) minX = minX_;
        if (minY_ < minY) minY = minY_;
        if (minZ_ < minZ) minZ = minZ_;

        if (maxX_ > maxX) maxX = maxX_;
        if (maxY_ > maxY) maxY = maxY_;
        if (maxZ_ > maxZ) maxZ = maxZ_;
    }

    if (minX < minX_Stored) minX_Stored = minX;
    if (minY < minY_Stored) minY_Stored = minY;
    if (minZ < minZ_Stored) minZ_Stored = minZ;

    if (maxX > maxX_Stored) maxX_Stored = maxX;
    if (maxY > maxY_Stored) maxY_Stored = maxY;
    if (maxZ > maxZ_Stored) maxZ_Stored = maxZ;
}

void collisionChunks() {
    int minChunkX = getChunkPos(minX_Stored);
    int minChunkY = getChunkPos(minY_Stored);
    int minChunkZ = getChunkPos(minZ_Stored);

    int maxChunkX = getChunkPos(maxX_Stored);
    int maxChunkY = getChunkPos(maxY_Stored);
    int maxChunkZ = getChunkPos(maxZ_Stored);

    int checkSurface = 0;
    for (int cx = minChunkX; cx <= maxChunkX; cx++) {
        for (int cy = minChunkY; cy <= maxChunkY; cy++) {
            for (int cz = minChunkZ; cz <= maxChunkZ; cz++) {
                CollisionChunks* coll = &collisionChunkSurfaces[checkSurface];
                if (coll && (coll->pos.x == cx && coll->pos.y == cy && coll->pos.z == cz)) continue;
                collisionChunkSurfaces = pd_realloc(collisionChunkSurfaces, sizeof(CollisionChunks) * (chunkAmt + 1));
                collisionChunkSurfaces[chunkAmt].pos = (Vector3f){cx, cy, cz};
                collisionChunkSurfaces[chunkAmt].amt = 0;
                collisionChunkSurfaces[chunkAmt].collisions = NULL;

                checkSurface++;
                chunkAmt++;
            }
        }
    }
}

void resetTriggers() {
    triggers = NULL;
    triggerCount = 0;
}

void addTriggers(Vector3f pos, Vector3f size, int type, int id) {
    triggers = pd_realloc(triggers, (triggerCount + 1) * sizeof(Triggers));

    Triggers* trig = &triggers[triggerCount++];
    trig->pos = pos;
    trig->size = size;
    trig->type = type;
    trig->id = id;
}

void addCollisionSurface(Vector3f v0, Vector3f v1, Vector3f v2, Vector3f normal, SurfaceType type) {
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
        if (ny > 0.7f)       { surf.type = SURFACE_FLOOR;   pd->system->logToConsole("Floor"); }
        else if (ny < -0.7f) { surf.type = SURFACE_CEILING; pd->system->logToConsole("Cieling"); }
        else                 { surf.type = SURFACE_WALL;    pd->system->logToConsole("Wall"); }
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

Triggers cylinderInTrigger(Vector3f pos, float radius, float height) {
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

VectMf cylinderInTriangle(Vector3f pos, float radius, float height) {
    // int inside =
    //     pos.x >= (minX_Stored - FIX_BORDER) && pos.x <= (maxX_Stored + FIX_BORDER) &&
    //     pos.y >= (minY_Stored - FIX_BORDER) && pos.y <= (maxY_Stored + FIX_BORDER) &&
    //     pos.z >= (minZ_Stored - FIX_BORDER) && pos.z <= (maxZ_Stored + FIX_BORDER);

    // if (!inside) return (VectMf){0, 0, 0, -1, -1, -1};
    
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
            
            bool hit = false;
            for (int h = 0; h < 2; h++) {
                float py = (h == 0) ? pyBottom : pyTop;
                if (pos.x + pad < tri->minX || pos.x - pad > tri->maxX || py + pad < tri->minY || py - pad > tri->maxY || pos.z + pad < tri->minZ || pos.z - pad > tri->maxZ) continue;

                float dot = (px - tri->center.x) * tri->normal.x + (py - tri->center.y) * tri->normal.y + (pz - tri->center.z) * tri->normal.z;
                if (dot * dot > radius2) continue;
                
                float projX = px - dot * tri->normal.x;
                float projY = py - dot * tri->normal.y;
                float projZ = pz - dot * tri->normal.z;
                
                float v2x = projX - tri->v0.x;
                float v2y = projY - tri->v0.y;
                float v2z = projZ - tri->v0.z;

                float dot02 = tri->v0x * v2x + tri->v0y * v2y + tri->v0z * v2z;
                float dot12 = tri->v1x * v2x + tri->v1y * v2y + tri->v1z * v2z;

                float u = (tri->dot11 * dot02 - tri->dot01 * dot12) * tri->invDenom;
                float v = (tri->dot00 * dot12 - tri->dot01 * dot02) * tri->invDenom;

                if (u < -TRI_EPSILON || v < -TRI_EPSILON || (u + v > 1.0f + TRI_EPSILON)) continue;

                float penetration = radius - dot;
                if (tri->type == OUT_OF_BOUNDS) return (VectMf){0, 0, 0, -1, -1, -1};

                if (h == 0) {
                    if (tri->type == SURFACE_FLOOR && dot < radius) {
                        pushPlayer.pos.y += penetration;
                        pushPlayer.floor = 1;

                        hit = true;
                    } 
                } else {
                    if (tri->type == SURFACE_CEILING && dot < radius) {
                        pushPlayer.pos.y -= penetration;
                        pushPlayer.ceiling = 1;

                        hit = true;
                    } 
                }
                
                if (tri->type == SURFACE_WALL && fabsf(dot) < radius) {
                    pushPlayer.pos.x += penetration * tri->normal.x;
                    pushPlayer.pos.z += penetration * tri->normal.z;
                    pushPlayer.wall = 1;

                    hit = true;
                }
            }
            if (hit) return pushPlayer;
        }

        break;
    }

    return pushPlayer;
}