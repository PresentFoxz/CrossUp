#include "meshConvert.h"

void allocateMeshes(VertAnims* mesh, int entCount, int maxAnims, int* framesPerAnim) {
    for (int e = 0; e < entCount; e++) {
        mesh[e].anims = malloc(sizeof(AnimFrames*) * maxAnims);

        for (int a = 0; a < maxAnims; a++) {
            int frames = framesPerAnim[a];
            
            mesh[e].anims[a] = malloc(sizeof(AnimFrames));
            
            mesh[e].anims[a]->meshModel = malloc(sizeof(Mesh_t) * frames);
            for (int f = 0; f < frames; f++) {
                mesh[e].anims[a]->meshModel[f].data = NULL;
                mesh[e].anims[a]->meshModel[f].bfc = NULL;
                mesh[e].anims[a]->meshModel[f].color = NULL;
                mesh[e].anims[a]->meshModel[f].count = 0;
            }

            mesh[e].anims[a]->frames = malloc(sizeof(int) * frames);
            for (int f = 0; f < frames; f++) {
                mesh[e].anims[a]->frames[f] = 1;
            }
            printf("Frames per Anim: %d\n", framesPerAnim[a]);
        }

        printf("Allocated meshes %d\n", (e+1));
        printf("Max Anims: %d\n", maxAnims);
    }
}

void convertFileToMesh(const char* filename, Mesh_t* meshOut, int color) {
    FILE* fptr = fopen(filename, "r");
    if (!fptr) {
        printf("Error: Could not open file %s\n", filename);
        return;
    }
    
    Vect3f* verts = NULL;
    int vertCount = 0;

    int (*tris)[3] = NULL;
    int triCount = 0;

    int* colorArr = NULL;

    char line[256];
    while (fgets(line, sizeof(line), fptr)) {
        if (strncmp(line, "v ", 2) == 0) {
            float x, y, z;
            if (sscanf(line, "v %f %f %f", &x, &y, &z) == 3) {
                verts = realloc(verts, sizeof(Vect3f) * (vertCount + 1));
                verts[vertCount].x = x;
                verts[vertCount].y = y;
                verts[vertCount].z = -z;
                vertCount++;
            }
        } else if (strncmp(line, "f ", 2) == 0) {
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
                tris = realloc(tris, sizeof(int[3]) * (triCount + 1));
                colorArr = realloc(colorArr, sizeof(int) * (triCount + 1));
                tris[triCount][0] = indices[0];
                tris[triCount][1] = indices[2];
                tris[triCount][2] = indices[1];
                colorArr[triCount] = randomInt(0, 3);
                triCount++;
            } else if (idx == 4) {
                tris = realloc(tris, sizeof(int[3]) * (triCount + 2));
                colorArr = realloc(colorArr, sizeof(int) * (triCount + 2));

                tris[triCount][0] = indices[0];
                tris[triCount][1] = indices[2];
                tris[triCount][2] = indices[1];
                colorArr[triCount] = randomInt(0, 3);

                tris[triCount + 1][0] = indices[0];
                tris[triCount + 1][1] = indices[3];
                tris[triCount + 1][2] = indices[2];
                colorArr[triCount + 1] = randomInt(0, 3);

                triCount += 2;
            }
        }
    }

    meshOut->data = malloc(sizeof(Vect3f) * triCount * 3);
    meshOut->bfc  = malloc(sizeof(int) * triCount);
    meshOut->color = malloc(sizeof(int) * triCount);
    meshOut->count = triCount;

    for (int t = 0; t < triCount; t++) {
        for (int v = 0; v < 3; v++) {
            meshOut->data[t * 3 + v] = verts[tris[t][v]];
        }
        meshOut->bfc[t] = 1;
        if (color != -1) { meshOut->color[t] = colorArr[t]; } else { meshOut->color[t] = 3; }
    }

    free(verts);
    free(tris);
    free(colorArr);
    fclose(fptr);
}