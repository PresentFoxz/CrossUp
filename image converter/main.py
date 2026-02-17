from PIL import Image
import os

def newPixels(new_pixels):
    new_img = Image.new("RGBA", (width, height))
    new_img.putdata(new_pixels)

    new_name = file.replace(".png", "_gray16.png")
    new_img.save(os.path.join(script_folder, new_name))

    print(f"Saved {new_name}")

def newFox(new_pixelIDX):
    fox_name = file.replace(".png", ".fox")
    fox_path = os.path.join(script_folder, fox_name)

    chunk_size = 100
    with open(fox_path, "w") as f:
        f.write(f"width {width}\n")
        f.write(f"height {height}\n")

        for i in range(0, len(new_pixelIDX), chunk_size):
            row = new_pixelIDX[i:i+chunk_size]
            row_str = " ".join(str(int(pixel)) for pixel in row)
            f.write("color " + row_str + "\n")

    print(f"Saved {fox_name}")

raylibShadeLUT = []
for shade in range(16):
    gray = (shade * 255) // 15
    raylibShadeLUT.append((gray, gray, gray, 255))

script_folder = os.path.dirname(os.path.abspath(__file__))
png_files = [f for f in os.listdir(script_folder) if f.lower().endswith(".png")]

if not png_files:
    print("No PNG files found.")
    input("Press Enter to exit...")
    exit()

for file in png_files:
    if file.endswith("_gray16.png"):
        continue

    path = os.path.join(script_folder, file)

    try:
        img = Image.open(path).convert("RGBA")
    except Exception as e:
        print(f"Failed to open {file}: {e}")
        continue

    width, height = img.size
    new_pixels = []
    new_pixelIDX = []

    for pixel in img.getdata():
        r, g, b, a = pixel

        brightness = (r + g + b) // 3
        shade_index = (brightness * 15) // 255

        if a <= 0:
            new_pixelIDX.append(-1)
            new_pixels.append((0, 0, 0, 0))
        else:
            new_pixelIDX.append(shade_index)
            new_pixels.append(raylibShadeLUT[shade_index])

    newPixels(new_pixels)
    newFox(new_pixelIDX)

print("\nDone.")
input("Press Enter to exit...")
