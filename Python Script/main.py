import pygame
import sys
import draw
import math
import keyboard
import json
import random
import os

SCREEN_WIDTH = 400
SCREEN_HEIGHT = 240

pygame.init()
screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
pygame.display.set_caption("Model Editor")
clock = pygame.time.Clock()

verts = []
tris = []
normals = []
color = []
orderedTris = []

check = False

def load_obj(filename):
    global verts, tris, color, orderedTris
    scale = 8.0
    verts = []
    tris = []
    color = []
    orderedTris = []

    with open(filename, 'r') as f:
        for line in f:
            if line.startswith('v '):
                parts = line.strip().split()
                x, y, z = map(float, parts[1:4])
                verts.append([x, y, z])
            elif line.startswith('f '):
                parts = line.strip().split()[1:]
                if len(parts) == 3:
                    tri = []
                    for part in parts:
                        idx = part.split('/')[0]
                        tri.append(int(idx) - 1)
                    tris.append(tri)
                    color.append(random.randint(0, 3))
                if len(parts) == 4:
                    quad = []
                    for part in parts:
                        idx = part.split('/')[0]
                        quad.append(int(idx) - 1)
                    tris.append([quad[0], quad[1], quad[2]])
                    tris.append([quad[0], quad[2], quad[3]])
                    color.append(random.randint(0, 3))
                    color.append(random.randint(0, 3))
                else:
                    pass
    
    for i in range(len(verts)):
        verts[i][0] *= scale
        verts[i][1] *= scale
        verts[i][2] *= -scale
    for triIndex, tri in enumerate(tris):
        triVerts = [verts[idx] for idx in tri]
        orderedTris.append(triVerts)
    
    print("Vertices: ", verts)
    print("Triangles: ", tris)
    print("Colors: ", color)
    print("Ordered Triangles: ", orderedTris)

def loadModelData():
    global verts, tris, color, orderedTris

    orderedTris = []
    
    with open("model.json", "r") as file:
        data = json.load(file)
        verts = data["verts"]
        tris = data["tris"]
        color = data["colors"]
        print("Vert data loaded:", verts)
        print("Tri data loaded:", tris)
        print("Color data loaded:", color)
    
    for triIndex, tri in enumerate(tris):
        triVerts = [verts[idx] for idx in tri]
        orderedTris.append(triVerts)

def exportModelDataJson():
    global verts, tris, color

    data = {
        "verts": verts,
        "tris": tris,
        "colors": color
    }

    with open("model_export.json", "w") as file:
        json.dump(data, file, indent=4)

    print("Model data exported to model_export.json")

def exportModelDataH():
    global verts, tris, color, orderedTris

    if os.path.exists("model.h"):
        os.remove("model.h")
    
    flatVerts = [v for tri in orderedTris for v in tri]
    
    def to_c_array(lst, is_float=False):
        items = []
        for item in lst:
            if isinstance(item, list) or isinstance(item, tuple):
                items.append(to_c_array(item, is_float))
            else:
                items.append(f"{item}f" if is_float else str(item))
        return "{" + ", ".join(items) + "}"
    
    data_str = to_c_array(flatVerts, is_float=True)
    color_str = to_c_array(color)

    with open("model.h", "w") as file:
        file.write("#ifndef MODEL_H\n")
        file.write("#define MODEL_H\n")
        file.write('#include "mesh.h"\n\n')
        file.write("static const Mesh model = {\n")
        file.write(f"    .data = (Vect3m[]) {data_str},\n")
        file.write(f"    .color = (int[]) {color_str},\n")
        file.write(f"    .count = (int) {len(tris)},\n")
        file.write("};\n\n")
        file.write("#endif\n")
    
    print("Model data exported to model.h")

def moveCam():
    yaw = draw.rot[1]
    move_delta = 0.03
    if keyboard.is_pressed("w"):
        draw.cam[0] += move_delta * math.sin(yaw)
        draw.cam[2] += move_delta * math.cos(yaw)
    if keyboard.is_pressed("s"):
        draw.cam[0] -= move_delta * math.sin(yaw)
        draw.cam[2] -= move_delta * math.cos(yaw)
    if keyboard.is_pressed("a"):
        draw.cam[0] -= move_delta * math.cos(yaw)
        draw.cam[2] += move_delta * math.sin(yaw)
    if keyboard.is_pressed("d"):
        draw.cam[0] += move_delta * math.cos(yaw)
        draw.cam[2] -= move_delta * math.sin(yaw)
    
    if keyboard.is_pressed("q"):
        draw.cam[1] -= move_delta
    if keyboard.is_pressed("e"):
        draw.cam[1] += move_delta
        
    if keyboard.is_pressed("up"):
        draw.rot[0] += 0.1
    if keyboard.is_pressed("down"):
        draw.rot[0] -= 0.1
    if keyboard.is_pressed("left"):
        draw.rot[1] -= 0.1
    if keyboard.is_pressed("right"):
        draw.rot[1] += 0.1

def swapVerts(i, j):
    global tris, orderedTris
    for tri in tris:
        tmp = tri[i]
        tri[i] = tri[j]
        tri[j] = tmp

    orderedTris = []
    for triIndex, tri in enumerate(tris):
        triVerts = [verts[idx] for idx in tri]
        orderedTris.append(triVerts)

loadModelData()
while True:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            pygame.quit()
            sys.exit()

    screen.fill((30, 30, 30))

    moveCam()

    if keyboard.is_pressed("r"):
        loadModelData()
    if keyboard.is_pressed("o"):
        load_obj("model.obj")
        print("Tri Count: ", len(tris))
        print("Vert Count: ", len(verts))
    if keyboard.is_pressed("p"):
        print("Saving Model...")

        exportModelDataJson()
        exportModelDataH()

        print("Saved Model!")
    
    if keyboard.is_pressed("z") and check:
        swapVerts(1, 2)
        check = False
    else:
        check = True

    for triIndex, tri in enumerate(tris):
        projected_verts = [draw.rotate_and_project(verts[idx], math, SCREEN_WIDTH, SCREEN_HEIGHT) for idx in tri]
        if not len(color) > triIndex:
            c = 0
        else:
            c = color[triIndex]

        skip = False
        for (x, y) in projected_verts:
            if x == -1 and y == -1:
                skip = True
                break
            if x < 0 or x >= SCREEN_WIDTH or y < 0 or y >= SCREEN_HEIGHT:
                skip = True
                break
        if not skip:
            draw.drawFilledTriangle(projected_verts, c, 2, 2, pygame, screen)
            draw.drawTris(projected_verts, pygame, screen)

    pygame.display.flip()
    clock.tick(60)

if __name__ == "__main__":
    main()
