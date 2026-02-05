#include "meshConvert.h"

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

static void buildTriangleEdges(Mesh_t* mesh) {
    mesh->edgeCount = mesh->triCount * 3;
    mesh->edges = pd_malloc(sizeof(Edge) * mesh->edgeCount);

    int ei = 0;

    for (int t = 0; t < mesh->triCount; t++) {
        int* tri = mesh->tris[t];

        mesh->edges[ei++] = (Edge){ .v0 = tri[0], .v1 = tri[1] };
        mesh->edges[ei++] = (Edge){ .v0 = tri[1], .v1 = tri[2] };
        mesh->edges[ei++] = (Edge){ .v0 = tri[2], .v1 = tri[1] };
    }
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

void convertFileToMesh(const char* filename, Mesh_t* meshOut, int color, int invert, int outline) {
    SDFile* fptr = pd->file->open(filename, kFileRead | kFileReadData);
    if (!fptr) {
        pd->system->logToConsole("Error: Could not open file %s\n", filename);
        return;
    }
    
    Vect3f* verts = NULL;
    int vertCount = 0;

    int (*tris)[3] = NULL;
    int triCount = 0;

    int* colorArr = NULL;

    char line[256];
    while (pd_fgets(line, sizeof(line), fptr)) {
        if (strncmp(line, "v ", 2) == 0) {
            float x, y, z;
            if (sscanf(line, "v %f %f %f", &x, &y, &z) == 3) {
                verts = pd_realloc(verts, sizeof(Vect3f) * (vertCount + 1));
                verts[vertCount].x = x;
                verts[vertCount].y = y;
                verts[vertCount].z = -z;
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

                colorArr[triCount] = randomInt(0, 15);
                triCount++;
            }
            
            else if (idx == 4) {
                tris = pd_realloc(tris, sizeof(int[3]) * (triCount + 2));
                colorArr = pd_realloc(colorArr, sizeof(int) * (triCount + 2));
                
                tris[triCount][0] = indices[0];
                tris[triCount][1] = invert ? indices[2] : indices[1];
                tris[triCount][2] = invert ? indices[1] : indices[2];
                colorArr[triCount] = randomInt(0, 15);
                
                tris[triCount + 1][0] = indices[0];
                tris[triCount + 1][1] = invert ? indices[3] : indices[2];
                tris[triCount + 1][2] = invert ? indices[2] : indices[3];
                colorArr[triCount + 1] = randomInt(0, 15);

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

    for (int i = 0; i < triCount; i++) {
        meshOut->color[i] = colorArr[i];
        meshOut->bfc[i] = 1;
    }

    meshOut->flipped = invert;
    meshOut->outline = outline;

    // buildTriangleEdges(meshOut);
}

int allocAnimModel(VertAnims* mesh, int maxAnims, const int* framesPerAnim, const char** names[], int color, int invert, int outline) {
    int highest = 0;
    allocateMeshes(mesh, maxAnims, framesPerAnim);
    for (int i = 0; i < maxAnims; i++) {
        mesh->anims[i]->frames = framesPerAnim[i];
        for (int f = 0; f < framesPerAnim[i]; f++) {
            convertFileToMesh(names[i][f], &mesh->anims[i]->meshModel[f], color, invert, outline);

            int triCount = mesh->anims[i]->meshModel[f].triCount;
            if (triCount > highest) highest = triCount;

            pd->system->logToConsole("Tri Count: %d\n", triCount);
        }
    }

    if (outline) highest *= 2;
    return highest;
}