#include "meshConvert.h"

Vect3f computeNormal(Vect3f tri[3]) {
    Vect3f edge1, edge2;
    edge1.x = tri[1].x - tri[0].x;
    edge1.y = tri[1].y - tri[0].y;
    edge1.z = tri[1].z - tri[0].z;

    edge2.x = tri[2].x - tri[0].x;
    edge2.y = tri[2].y - tri[0].y;
    edge2.z = tri[2].z - tri[0].z;
    
    Vect3f normal;
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

void convertFileToMesh(const char* filename, Mesh_t* meshOut, int color, int invert, int outline, Vect3f size) {
    SDFile* fptr = pd->file->open(filename, kFileRead | kFileReadData);
    if (!fptr) {
        pd->system->logToConsole("Error: Could not open file %s\n", filename);
        return;
    }
    
    Vect3f* verts = NULL;
    int vertCount = 0;

    int (*tris)[3] = NULL;
    int triCount = 0;

    uint8_t* colorArr = NULL;

    char line[256];
    while (pd_fgets(line, sizeof(line), fptr)) {
        if (strncmp(line, "v ", 2) == 0) {
            float x, y, z;
            if (sscanf(line, "v %f %f %f", &x, &y, &z) == 3) {
                verts = pd_realloc(verts, sizeof(Vect3f) * (vertCount + 1));
                verts[vertCount].x = x  * size.x;
                verts[vertCount].y = y  * size.y;
                verts[vertCount].z = -z * size.z;
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
    meshOut->normal   = pd_malloc(sizeof(Vect3f) * triCount);

    for (int i = 0; i < triCount; i++) {
        meshOut->color[i] = colorArr[i];
        meshOut->bfc[i] = 1;


        Vect3f face[3] = {
            {verts[tris[i][0]].x, verts[tris[i][0]].y, verts[tris[i][0]].z},
            {verts[tris[i][1]].x, verts[tris[i][1]].y, verts[tris[i][1]].z},
            {verts[tris[i][2]].x, verts[tris[i][2]].y, verts[tris[i][2]].z},
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

int allocAnimModel(VertAnims* mesh, int maxAnims, const int* framesPerAnim, const char** names[], int color, int invert, int outline, Vect3f size) {
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