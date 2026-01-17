import os
import shutil
import subprocess
import time
import random

verts = []
tris = []
normals = []
color = []
orderedTris = []

check = False

BASE_DIR = os.getcwd()
DIR = os.path.join(BASE_DIR, "models")

def createHLoader(loaded, filename):
    base_path = os.path.join(BASE_DIR, "C/hFiles")
    loader_path = os.path.join(base_path, "fileLoader.h")
    os.makedirs(base_path, exist_ok=True)

    with open(loader_path, "a") as file:
        if loaded == 0:
            file.write("#ifndef FILELOADER_H\n")
            file.write("#define FILELOADER_H\n\n")
            file.write(f"#include \"../models/{filename}\"\n")
        elif loaded == 1:
            file.write(f"#include \"../models/{filename}\"\n")
        elif loaded == 2:
            file.write(f"\n#endif")

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

def exportModelDataH():
    global verts, tris, color, orderedTris

    if os.path.exists("model.h"):
        os.remove("model.h")
    
    flatVerts = [v for tri in orderedTris for v in tri]
    bfc = [1 for v in flatVerts]
    
    def to_c_array(lst, is_float=False):
        items = []
        for item in lst:
            if isinstance(item, list) or isinstance(item, tuple):
                items.append(to_c_array(item, is_float))
            else:
                if is_float and isinstance(item, float) and not item.is_integer():
                    items.append(f"{item}f")
                elif is_float and isinstance(item, float):
                    items.append(f"{int(item)}.0f")
                else:
                    items.append(str(item))
        return "{" + ", ".join(items) + "}"
    
    data_str = to_c_array(flatVerts, is_float=True)
    bfc_str = to_c_array(bfc)
    color_str = to_c_array(color)

    with open("model.h", "w") as file:
        file.write("#ifndef MODEL_H\n")
        file.write("#define MODEL_H\n")
        file.write('#include "mesh.h"\n\n')
        file.write("static const Mesh_t model = {\n")
        file.write(f"    .data = (Vect3m[]) {data_str},\n")
        file.write(f"    .bfc = (int[]) {bfc_str},\n")
        file.write(f"    .color = (int[]) {color_str},\n")
        file.write(f"    .count = (int) {len(tris)},\n")
        file.write("};\n\n")
        file.write("#endif\n")
    
    print("Model data exported to model.h")

def main():
    user_input = input("How many folders do you have in the models folder? (make sure they are named 0+ so they are in order for the animations): ")
    user_delete = input("Do you want to delete all existing .h files in the hFiles folder? (y/n): ")

    hfiles_path = os.path.join(BASE_DIR, "C/hFiles")
    if user_delete.lower() == 'y':
        if os.path.isdir(hfiles_path):
            shutil.rmtree(hfiles_path)
            print("Existing .h files deleted.")
        os.makedirs(hfiles_path, exist_ok=True)
        print("hFiles directory created.")

    try:
        count = int(user_input)
    except ValueError:
        print("Please enter a valid number.")
        return

    data = []
    save_paths = []
    save_names = []

    for i in range(count):
        folder_path = os.path.join(DIR, str(i))
        if not os.path.isdir(folder_path):
            print(f"Folder not found: {folder_path}")
            continue

        paths = [os.path.join(folder_path, f) for f in os.listdir(folder_path)]
        names = [os.path.splitext(f)[0] for f in os.listdir(folder_path)]

        for idx, file in enumerate(paths):
            load_obj(file)
            path_to_folder = os.path.join(BASE_DIR, f"C/hFiles/{i}")
            os.makedirs(path_to_folder, exist_ok=True)
            exportModelDataH()

        save_names.append(names)
        data.append(len(paths))

    loaded = 0
    print(save_names)
    for i in range(len(data)):
        for z, name in enumerate(save_names[i]):
            createHLoader(loaded, f"{i}/{name}.h")
            loaded = 1
    createHLoader(2, f"{i}/{name}.h")

    print("All models processed. Running external program...")

    time.sleep(500)

main()