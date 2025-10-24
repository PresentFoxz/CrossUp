rot = [0, 0, 0]
cam = [0, 0, -3]
sX, sY, sW, sH = 0, 0, 400, 240
dts = 200
FOV = 100
CAMERA_Z = -3.0

colors = [
    (0, 0, 0),
    (85, 85, 85),
    (170, 170, 170),
    (255, 255, 255)
]

def rotate_and_project(vertex, math, sW, sH):
    camX, camY, camZ = cam
    rotX, rotY, rotZ = rot
    x, y, z = vertex

    x -= camX
    y -= camY
    z -= camZ

    xz = math.cos(rotY) * x - math.sin(rotY) * z
    zz = math.sin(rotY) * x + math.cos(rotY) * z
    x, z = xz, zz

    yz = math.cos(rotX) * y - math.sin(rotX) * z
    zz = math.sin(rotX) * y + math.cos(rotX) * z
    y, z = yz, zz

    if z <= 0.1:
        px, py = -1, -1
        return (px, py)

    px = int((x / z) * FOV + sW / 2)
    py = int((-y / z) * FOV + sH / 2)

    return (px, py)

def drawTris(triangle, pygame, screen):
    v1 = triangle[0]
    v2 = triangle[1]
    v3 = triangle[2]
    
    if not windingOrder(v1, v2, v3):
        return

    color = (0, 0, 0)
    width = 3

    pygame.draw.line(screen, color, (v1[0], v1[1]), (v2[0], v2[1]), width)
    pygame.draw.line(screen, color, (v3[0], v3[1]), (v1[0], v1[1]), width)
    pygame.draw.line(screen, color, (v2[0], v2[1]), (v3[0], v3[1]), width)


def windingOrder(p0, p1, p2):
    return (p0[0]*p1[1] - p0[1]*p1[0] +
        p1[0]*p2[1] - p1[1]*p2[0] +
        p2[0]*p0[1] - p2[1]*p0[0] > 0)

def edge_fn(a, b, c):
    return (c[0] - a[0])*(b[1] - a[1]) - (c[1] - a[1])*(b[0] - a[0])

def drawFilledTriangle(verts, c, xAmt, yAmt, pygame, screen):
    if not windingOrder(verts[0], verts[1], verts[2]):
        return
    
    min_x = max(min(v[0] for v in verts), 0)
    max_x = min(max(v[0] for v in verts), screen.get_width() - 1)
    min_y = max(min(v[1] for v in verts), 0)
    max_y = min(max(v[1] for v in verts), screen.get_height() - 1)

    area = edge_fn(verts[0], verts[1], verts[2])
    if area == 0:
        return
    
    pixels = pygame.PixelArray(screen)
    color_val = screen.map_rgb(colors[c])

    for y in range(min_y, max_y + 1, yAmt):
        for x in range(min_x, max_x + 1, xAmt):
            p = (x, y)
            w0 = edge_fn(verts[1], verts[2], p)
            w1 = edge_fn(verts[2], verts[0], p)
            w2 = edge_fn(verts[0], verts[1], p)
            if (w0 >= 0 and w1 >= 0 and w2 >= 0) or (w0 <= 0 and w1 <= 0 and w2 <= 0):
                pixels[x, y] = color_val

    del pixels
