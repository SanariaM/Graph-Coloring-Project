import sys

def read_graph(path):
    with open(path) as f:
        n = int(f.readline().strip())
        mat = []
        for _ in range(n):
            mat.append(list(map(int, f.readline().split())))
    return n, mat

def read_colors(path, n):
    with open(path) as f:
        cols = [int(line.strip()) for line in f if line.strip() != ""]
    if len(cols) != n:
        raise SystemExit(f"Color file has {len(cols)} lines; expected {n}.")
    return cols

def main():
    if len(sys.argv) != 3:
        print("Usage: python3 check_coloring.py graph.txt colors_out.txt")
        sys.exit(1)

    gpath, cpath = sys.argv[1], sys.argv[2]
    n, adj = read_graph(gpath)
    color = read_colors(cpath, n)

    bad = 0
    for i in range(n):
        if color[i] <= 0:
            bad += 1
        for j in range(i+1, n):
            if adj[i][j] == 1 and color[i] == color[j]:
                bad += 1

    if bad == 0:
        print("OK: coloring is valid")
    else:
        print(f"FAIL: found {bad} problems (uncolored vertices or conflicts)")
        sys.exit(2)

if __name__ == "__main__":
    main()
