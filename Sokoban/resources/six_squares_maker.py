side = 0.5
short_side = 0.2

perm = [0,1,2]

lines = ["mtllib cube.mtl\no Cube"]
for face in range(6):
    if (face < 3):
        dims = [short_side, short_side, -side]
    else:
        dims = [short_side, short_side, side]
    for vertex in range(4):
        lines.append(" ".join(["v", *[str(dims[(i + face) % 3]) for i in range(3)]]))
        if vertex % 2 ^ (face // 3):
            dims[0] *= -1
        else:
            dims[1] *= -1

file = open("six_squares.obj", "w")
file.write("\n".join(lines))
file.write("""
vt 1.0 1.0
vt 1.0 0.0
vt 0.0 0.0
vt 0.0 1.0
vn 0.0 0.0 -1.0
vn 0.0 -1.0 0.0
vn -1.0 0.0 0.0
vn 0.0 0.0 1.0
vn 0.0 1.0 0.0
vn 1.0 0.0 0.0
usemtl Material
s off\n""")
for f in range(6):
    pieces = ["f"]
    for v in range(1,5):
        pieces.append(str(4*f + v) + "/" + str(v) + "/" + str(f+1))
    file.write(" ".join(pieces) + "\n")

file.close()
