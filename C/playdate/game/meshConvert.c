#include "meshConvert.h"

const static int maxColor = 31;

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

void allocateMeshes(VertAnims* mesh, int maxAnims, const int* framesPerAnim) {
    mesh->anims = pd_malloc(sizeof(AnimFrames*) * maxAnims);

    for (int a = 0; a < maxAnims; a++) {
        int frames = framesPerAnim[a];
            
        mesh->anims[a] = pd_malloc(sizeof(AnimFrames));
            
        mesh->anims[a]->meshModel = pd_malloc(sizeof(Mesh_t) * frames);
        for (int f = 0; f < frames; f++) {
            mesh->anims[a]->meshModel[f].verts = NULL;
            mesh->anims[a]->meshModel[f].tris = NULL;
            mesh->anims[a]->meshModel[f].bfc = NULL;
            mesh->anims[a]->meshModel[f].color = NULL;
            mesh->anims[a]->meshModel[f].triCount = 0;
            mesh->anims[a]->meshModel[f].vertCount = 0;
        }
        pd->system->logToConsole("Frames per Anim: %d\n", framesPerAnim[a]);
    }

    pd->system->logToConsole("Max Anims: %d\n", maxAnims);
}

static int pd_fgets(char* out, int maxLen, SDFile* file) {
    int i = 0;
    char c;

    while (i < maxLen - 1) {
        int r = pd->file->read(file, &c, 1);
        if (r <= 0) break;
        out[i++] = c;
        if (c == '\n') break;
    }

    out[i] = '\0';
    return i > 0;
}

static inline void buildTriangleEdges(Mesh_t* mesh) {
    mesh->edgeCount = mesh->triCount * 3;
    mesh->edges = pd_malloc(sizeof(Edge) * mesh->edgeCount);

    int ei = 0;
    
    for (int t = 0; t < mesh->triCount; t++) {
        int* tri = mesh->tris[t];

        mesh->edges[ei++] = (Edge){ .v0 = tri[0], .v1 = tri[1], .tri0 = t, .tri1 = -1 };
        mesh->edges[ei++] = (Edge){ .v0 = tri[1], .v1 = tri[2], .tri0 = t, .tri1 = -1 };
        mesh->edges[ei++] = (Edge){ .v0 = tri[2], .v1 = tri[0], .tri0 = t, .tri1 = -1 };
    }
    
    for (int i = 0; i < mesh->edgeCount; i++) {
        Edge* e1 = &mesh->edges[i];
        if (e1->tri1 != -1) continue;

        for (int j = i + 1; j < mesh->edgeCount; j++) {
            Edge* e2 = &mesh->edges[j];
            if (e2->tri1 != -1) continue;
            
            if ((e1->v0 == e2->v0 && e1->v1 == e2->v1) ||
                (e1->v0 == e2->v1 && e1->v1 == e2->v0)) {
                e1->tri1 = e2->tri0;
                e2->tri1 = e1->tri0;
                break;
            }
        }
    }
}

void convertFileToMesh(const char* filename, Mesh_t* meshOut, int color, int invert, int outline, Vector3f size) {
    SDFile* fptr = pd->file->open(filename, kFileRead | kFileReadData);
    if (!fptr) {
        pd->system->logToConsole("Error: Could not open file %s\n", filename);
        return;
    }

    Vector3f* rawVerts = NULL;
    int vertCount = 0;

    int (*tris)[3] = NULL;
    int triCount = 0;

    uint8_t* colorArr = NULL;

    char line[256];
    
    float minX = 1e30f, minY = 1e30f, minZ = 1e30f;
    float maxX = -1e30f, maxY = -1e30f, maxZ = -1e30f;

    while (pd_fgets(line, sizeof(line), fptr)) {
        if (strncmp(line, "v ", 2) == 0) {
            float x, y, z;
            if (sscanf(line, "v %f %f %f", &x, &y, &z) == 3) {
                rawVerts = pd_realloc(rawVerts, sizeof(Vector3f) * (vertCount + 1));
                rawVerts[vertCount].x = x;
                rawVerts[vertCount].y = y;
                rawVerts[vertCount].z = z;
                vertCount++;

                if (x < minX) minX = x; if (x > maxX) maxX = x;
                if (y < minY) minY = y; if (y > maxY) maxY = y;
                if (z < minZ) minZ = z; if (z > maxZ) maxZ = z;
            }
        }
    }
    
    Vector3f center;
    center.x = (minX + maxX) * 0.5f;
    center.y = (minY + maxY) * 0.5f;
    center.z = (minZ + maxZ) * 0.5f;

    pd->file->close(fptr);
    fptr = pd->file->open(filename, kFileRead | kFileReadData);
    if (!fptr) {
        pd->system->logToConsole("Error: Could not open file again %s\n", filename);
        return;
    }

    vertCount = 0;
    Vector3f* verts = NULL;

    while (pd_fgets(line, sizeof(line), fptr)) {
        if (strncmp(line, "v ", 2) == 0) {
            float x, y, z;
            if (sscanf(line, "v %f %f %f", &x, &y, &z) == 3) {
                verts = pd_realloc(verts, sizeof(Vector3f) * (vertCount + 1));
                verts[vertCount].x = (x - center.x) * size.x;
                verts[vertCount].y = (y - center.y) * size.y;
                verts[vertCount].z = -(z - center.z) * size.z;
                vertCount++;
            }
        }

        else if (strncmp(line, "f ", 2) == 0) {
            char* token = strtok(line + 2, " \t\n");
            int indices[4];
            int idx = 0;

            while (token && idx < 4) {
                char* slash = strchr(token, '/');
                if (slash) *slash = 0;
                indices[idx++] = atoi(token) - 1;
                token = strtok(NULL, " \t\n");
            }

            if (idx == 3) {
                tris = pd_realloc(tris, sizeof(int[3]) * (triCount + 1));
                colorArr = pd_realloc(colorArr, sizeof(int) * (triCount + 1));

                tris[triCount][0] = indices[0];
                tris[triCount][1] = invert ? indices[2] : indices[1];
                tris[triCount][2] = invert ? indices[1] : indices[2];

                colorArr[triCount] = randomInt(25, 254);
                triCount++;
            }

            else if (idx == 4) {
                tris = pd_realloc(tris, sizeof(int[3]) * (triCount + 2));
                colorArr = pd_realloc(colorArr, sizeof(int) * (triCount + 2));

                tris[triCount][0] = indices[0];
                tris[triCount][1] = invert ? indices[2] : indices[1];
                tris[triCount][2] = invert ? indices[1] : indices[2];
                colorArr[triCount] = randomInt(25, 254);

                tris[triCount + 1][0] = indices[0];
                tris[triCount + 1][1] = invert ? indices[3] : indices[2];
                tris[triCount + 1][2] = invert ? indices[2] : indices[3];
                colorArr[triCount + 1] = randomInt(25, 254);

                triCount += 2;
            }
        }
    }

    pd->file->close(fptr);
    
    meshOut->verts = verts;
    meshOut->vertCount = vertCount;
    meshOut->tris = tris;
    meshOut->triCount = triCount;

    meshOut->color = pd_malloc(sizeof(int) * triCount);
    meshOut->bfc   = pd_malloc(sizeof(int) * triCount);
    meshOut->normal = pd_malloc(sizeof(Vector3f) * triCount);

    for (int i = 0; i < triCount; i++) {
        meshOut->color[i] = colorArr[i];
        meshOut->bfc[i] = 1;

        Vector3f face[3] = {
            verts[tris[i][0]],
            verts[tris[i][1]],
            verts[tris[i][2]]
        };
        meshOut->normal[i] = computeNormal(face);
    }

    buildTriangleEdges(meshOut);
    pd_free(colorArr);
}

void convertFileToAtlas(const char* filename, textAtlas* atlasOut) {
    SDFile* fptr = pd->file->open(filename, kFileRead | kFileReadData);
    if (!fptr) {
        pd->system->logToConsole("Error: Could not open file %s\n", filename);
        return;
    }

    int pixelIndex = 0;
    int width = 0;
    int height = 0;
    int8_t* pixels = NULL;

    char line[4096];
    while (pd_fgets(line, sizeof(line), fptr)) {
        if (strncmp(line, "width ", 6) == 0) {
            width = atoi(line + 6);
        } else if (strncmp(line, "height ", 7) == 0) {
            height = atoi(line + 7);
        } else if (strncmp(line, "color ", 6) == 0) {
            if (width > 0 && height > 0 && pixels == NULL) {
                pixels = pd_malloc(width * height * sizeof(int8_t));
                if (!pixels) {
                    pd->system->logToConsole("Memory allocation failed\n");
                    fclose(fptr);
                    return;
                }
            }

            char* token = strtok(line + 6, " \t\n");
            while (token) {
                if (pixelIndex < width * height) {
                    pixels[pixelIndex++] = (int8_t)atoi(token);
                }
                token = strtok(NULL, " \t\n");
            }
        }
    }

    pd->file->close(fptr);

    if (pixels && pixelIndex == width * height) {
        atlasOut->pixels = pixels;
        atlasOut->w = width;
        atlasOut->h = height;
    } else {
        pd->system->logToConsole("Invalid .fox file format\n");
        pd_free(pixels);
    }
}

int allocAnimModel(VertAnims* mesh, int maxAnims, const int* framesPerAnim, const char** names[], int color, int invert, int outline, Vector3f size) {
    int highest = 0;
    allocateMeshes(mesh, maxAnims, framesPerAnim);
    for (int i = 0; i < maxAnims; i++) {
        mesh->anims[i]->frames = framesPerAnim[i];
        for (int f = 0; f < framesPerAnim[i]; f++) {
            convertFileToMesh(names[i][f], &mesh->anims[i]->meshModel[f], color, invert, outline, size);

            int triCount = mesh->anims[i]->meshModel[f].triCount;
            if (triCount > highest) highest = triCount;

            pd->system->logToConsole("Tri Count: %d\n", triCount);
        }
    }

    if (outline) highest *= 2;
    return highest;
}

void allocAnimAtlas(textAnimsAtlas* atlas, int maxAnims, const int* framesPerAnim, const char* names[]) {
    atlas->animation = pd_malloc(sizeof(textAtlasFrames*) * maxAnims);
    atlas->count = maxAnims;

    for (int a = 0; a < maxAnims; a++) {
        int frames = framesPerAnim[a];
        
        atlas->animation[a] = pd_malloc(sizeof(textAtlasFrames));
        atlas->animation[a]->frames = frames;

        pd->system->logToConsole("Anim: %d | Frames: %d\n", a, frames);
        
        convertFileToAtlas(names[a], &atlas->animation[a]->animData);
    }

    pd->system->logToConsole("Model Atlas Allocation Complete\n");
}

static void initMesh(Mesh_t* m) {
    m->verts = NULL;
    m->tris = NULL;
    m->normal = NULL;
    m->edges = NULL;
    m->color = NULL;
    m->bfc = NULL;

    m->vertCount = 0;
    m->triCount = 0;
    m->edgeCount = 0;
}

static void pushTri(Mesh_t* map, float x0,float y0,float z0, float x1,float y1,float z1, float x2,float y2,float z2, int wind, int color) {
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

static void writeChunkData(Mesh_t* map, WorldChunks* chunk, WaterSlice** water, int* waterAmt) {
    float x0, z0;
    float x1, z1;
    float x2, z2;
    int yMin, yMax;

    initMesh(map);
    for (int sCount=0; sCount < chunk->sectorCount; sCount++) {
        SectorSlice* slice = &chunk->sectors[sCount];

        int count = slice->count;
        if (count < 3) continue;

        int wind = -slice->normal;
        int color = slice->pallete;
        yMin = slice->y[0];
        yMax = slice->y[1];

        if (slice->type == 1) {
            int minX = INT_MAX; int minZ = INT_MAX;
            int maxX = INT_MIN; int maxZ = INT_MIN;
            for (int s = 0; s < count; s++) {
                int x = slice->points[s].x; int z = slice->points[s].z;

                if (minX > x) minX = x;
                if (maxX < x) maxX = x;

                if (minZ > z) minZ = z;
                if (maxZ < z) maxZ = z;
            }

            *water = pd_realloc(*water, sizeof(WaterSlice) * (*waterAmt + 1));
            (*water)[*waterAmt++] = (WaterSlice) {
                .y = yMin,
                .min = (Vector2i){minX, minZ},
                .max = (Vector2i){maxX, maxZ},
                .lines = NULL
            };

            color = 3;
        }

        for (int s = 1; s < count - 1; s++) {
            x0 = slice->points[0].x; z0 = slice->points[0].z;
            x1 = slice->points[s].x; z1 = slice->points[s].z;
            x2 = slice->points[s + 1].x; z2 = slice->points[s + 1].z;

            pushTri(map, x0,yMax,z0, x1,yMax,z1, x2,yMax,z2, wind, color);
            if (yMin != yMax) pushTri(map, x0,yMin,z0, x2,yMin,z2, x1,yMin,z1, wind, color);
        }
    }

    for (int wCount=0; wCount < chunk->wallCount; wCount++) {
        WallSlice* slice = &chunk->walls[wCount];
        
        int wind = -slice->normal;
        int color = slice->pallete;
        yMin = slice->y[0];
        yMax = slice->y[1];

        if (yMin == yMax) continue;

        float x0 = slice->points[0].x; float z0 = slice->points[0].z;
        float x1 = slice->points[1].x; float z1 = slice->points[1].z;

        if (x0 > x1) {
            float tmpX = x0; float tmpZ = z0;
            x0 = x1; z0 = z1;
            x1 = tmpX; z1 = tmpZ;
        }

        if (z0 > z1) {
            float tmpX = x0; float tmpZ = z0;
            x0 = x1; z0 = z1;
            x1 = tmpX; z1 = tmpZ;
        }

        pushTri(map, x0, yMin, z0, x1, yMin, z1, x1, yMax, z1, wind, color);
        pushTri(map, x0, yMin, z0, x1, yMax, z1, x0, yMax, z0, wind, color);
    }
}

static void readChunkData(SDFile* fptr, WorldChunks* chunk) {
    int xMin = 0, zMin = 0;
    int xMax = 0, zMax = 0;
    int x0 = 0, z0 = 0;
    int x1 = 0, z1 = 0;

    int yMin = 0, yMax = 0;
    int yMin_ = 0, yMax_ = 0;

    int pVal = 0;
    int normal = 0;
    int splitType = 0;
    int sVal = 0; int wVal = 0; int oVal = 0; int eVal = 0;

    SliceType type = SLICE_NONE;
    SectorSlice* sector = NULL;
    WallSlice* wall = NULL;

    char line[4096];
    while (pd_fgets(line, sizeof(line), fptr)) {
        if (strncmp(line, "chunk ", 6) == 0) {
            sscanf(line + 6, "%d %d %d %d", &xMin, &zMin, &xMax, &zMax);
            xMin *= 8; zMin *= 8;
            xMax *= 8; zMax *= 8;
        } else if (strncmp(line, "i ", 2) == 0) {
            sscanf(line + 2, "%d %d %d %d", &sVal, &wVal, &oVal, &eVal);
            
            chunk->sectors  = pd_malloc(sizeof(SectorSlice) * sVal);
            chunk->walls    = pd_malloc(sizeof(WallSlice) * wVal);
            chunk->objects  = pd_malloc(sizeof(ObjectSlice) * oVal);
            chunk->entities = pd_malloc(sizeof(ObjectSlice) * eVal);

            chunk->sectorCount = 0;
            chunk->wallCount = 0;
            chunk->objectCount = 0;
            chunk->entityCount = 0;

            type = SLICE_NONE;
        } else if (strncmp(line, "s ", 2) == 0) {
            if (type != SLICE_SECTOR) {
                sector = &chunk->sectors[chunk->sectorCount++];

                sector->points = NULL;
                sector->y[0] = 0;
                sector->y[1] = 0;
                sector->pallete = 0;
                sector->normal = 0;
                sector->count = 0;
                sector->type = 0;

                type = SLICE_SECTOR;
            }

            sscanf(line + 2, "%d %d", &x0, &z0);

            sector->points = pd_realloc(sector->points, sizeof(Vector2i) * (sector->count + 1));
            sector->points[sector->count++] = (Vector2i){ .x = x0, .z = z0 };
        } else if (strncmp(line, "w ", 2) == 0) {
            if (type != SLICE_WALL) {
                wall = &chunk->walls[chunk->wallCount++];

                wall->points[0] = (Vector2i){0,0};
                wall->points[1] = (Vector2i){0,0};
                wall->y[0] = 0;
                wall->y[1] = 0;
                wall->pallete = 0;
                wall->normal = 0;
                wall->type = 0;

                type = SLICE_WALL;
            }

            sscanf(line + 2, "%d %d %d %d", &x0, &z0, &x1, &z1);

            wall->points[0] = (Vector2i){ .x = x0, .z = z0 };
            wall->points[1] = (Vector2i){ .x = x1, .z = z1 };
        } else if (strncmp(line, "p ", 2) == 0) {
            sscanf(line + 2, "%d", &pVal);

            if (type == SLICE_SECTOR) { sector->pallete = pVal; }
            else if (type == SLICE_WALL) { wall->pallete = pVal; }
        } else if (strncmp(line, "n ", 2) == 0) {
            sscanf(line + 2, "%d", &normal);

            if (type == SLICE_SECTOR) { sector->normal = normal; }
            else if (type == SLICE_WALL) { wall->normal = normal; }
        } else if (strncmp(line, "t ", 2) == 0) {
            sscanf(line + 2, "%d", &splitType);
            
            if (type == SLICE_SECTOR) { sector->type = splitType; }
        } else if (strncmp(line, "f ", 2) == 0) {
            sscanf(line + 2, "%d %d", &yMin, &yMax);

            if (type == SLICE_SECTOR) {
                sector->y[0] = yMin;
                sector->y[1] = yMax;
                sector = NULL;
            } else if (type == SLICE_WALL) {
                wall->y[0] = yMin;
                wall->y[1] = yMax;
                wall = NULL;
            }

            if (yMin < yMin_) yMin_ = yMin;
            if (yMax > yMax_) yMax_ = yMax;

            type = SLICE_NONE;
        } else if (strncmp(line, "ok", 2) == 0) {
            chunk->chunkPos = (Vector3f){ .x = (float)xMin, .y = 0.0f, .z = (float)-zMin};
            chunk->chunkWHD = (Vector3f){ .x = (float)(xMax - xMin), .y = (float)yMax_, .z = (float)(zMax - zMin)};
            break;
        }
    }

    pd->system->logToConsole("Counts [ Sector: %d | Wall: %d | Entity: %d | Object: %d ]", chunk->sectorCount, chunk->wallCount, chunk->entityCount, chunk->objectCount);
    if (chunk->sectorCount <= 0 && chunk->wallCount <= 0 && chunk->entityCount <= 0 && chunk->objectCount <= 0) return;

    pd->system->logToConsole("Sectors");
    for (int i=0; i < chunk->sectorCount; i++) {
        sector = &chunk->sectors[i];
        Vector2i* p = sector->points;

        for (int c=0; c < sector->count; c++) { pd->system->logToConsole("Memory - Sector = [ X: %d | Z: %d ]", p[c].x, p[c].z); }
        pd->system->logToConsole("Memory - count: %d", sector->count);
        pd->system->logToConsole("Memory - Pallete: %d", sector->pallete);
        pd->system->logToConsole("Memory - yMin: %d | yMax: %d", sector->y[0], sector->y[1]);
        pd->system->logToConsole("Memory - Nomral: %d", sector->normal);
        if (sector->type == 1) pd->system->logToConsole("Water Sector!");
    }

    pd->system->logToConsole("\nWalls");
    for (int i=0; i < chunk->wallCount; i++) {
        wall = &chunk->walls[i];
        Vector2i p[2] = {wall->points[0], wall->points[1]};

        pd->system->logToConsole("Memory - Wall = [ x0: %d | z0: %d | x1: %d | z1: %d ]", p[0].x, p[0].z, p[1].x, p[1].z);
        pd->system->logToConsole("Memory - Pallete: %d", wall->pallete);
        pd->system->logToConsole("Memory - yMin: %d | yMax: %d", wall->y[0], wall->y[1]);
        pd->system->logToConsole("Memory - Nomral: %d", wall->normal);
    }
}

static int readChunkCount(SDFile* fptr) {
    char line[256];
    for (int i=0; i < 2; i++) {
        if (!pd_fgets(line, sizeof(line), fptr)) {
            pd->file->close(fptr);
            return 0;
        }
    }
    return atoi(line);
}

Mesh_Chunks* readMapData(const char* filename, int* outSectorAmt, WaterSlice** water, int* waterAmt) {
    Mesh_Chunks* chunks = NULL;
    *outSectorAmt = 0;

    SDFile* fptr = pd->file->open(filename, kFileRead | kFileReadData);
    if (!fptr) return NULL;

    int chunkAmt = readChunkCount(fptr);
    if (chunkAmt <= 0) return NULL;

    WorldChunks* points = pd_malloc(sizeof(WorldChunks));

    chunks = pd_malloc(sizeof(Mesh_Chunks) * chunkAmt);
    memset(chunks, 0, sizeof(Mesh_Chunks) * chunkAmt);

    *waterAmt = 0;
    for (int i=0; i < chunkAmt; i++) {
        memset(points, 0, sizeof(WorldChunks));
        readChunkData(fptr, points);

        memset(&chunks[i].map, 0, sizeof(Mesh_t));
        chunks[i].pos = points->chunkPos;
        chunks[i].whd = points->chunkWHD;
        writeChunkData(&chunks[i].map, points, water, waterAmt);
    }
    
    *outSectorAmt = chunkAmt;
    pd->file->close(fptr);
    return chunks;
}