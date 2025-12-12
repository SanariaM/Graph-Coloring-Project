# Parallel Graph Coloring Using MPI (C + MPI)

This repository contains two MPI-based parallel graph-coloring implementations and a reproducible benchmark workflow that produces **separate computation and communication timing** and generates graphs for each dataset.

## What’s included

- `alg1_partition.c` — **Alg1 (Partitioning Strategy)**: each rank owns a block of vertices, greedily colors eligible local vertices using priorities, then synchronizes colors across ranks each round.
- `alg2_mis.c` — **Alg2 (MIS / Divide-and-Conquer)**: repeatedly finds a Maximal Independent Set (MIS), colors it, removes it, and repeats until all vertices are colored.
- `graph_gen.c` — generates random adjacency-matrix graph files (reproducible with seed).
- `run_bench.sh` — runs the full experiment matrix and writes `results.csv`.
- `plot_local.py` — creates PNG graphs from `results.csv` (**run on your laptop**).
- `check_coloring.py` — optional correctness check (conflict-free coloring).
- `Makefile` — builds all binaries.

## Input format

Graph file is an adjacency matrix:
- line 1: `n`
- next `n` lines: `n` integers (`0/1`) separated by spaces

## Output format (one line per run)

Each MPI run prints one CSV row:

`alg,n,p,total,comp,comm,rounds,colors`

- **total**: end-to-end runtime (seconds)
- **comp**: time spent in local computation (loops that decide colors/MIS, etc.)
- **comm**: time spent inside MPI calls (Bcast/Allgather/Allreduce/Reduce, etc.)
- **rounds**: number of synchronization rounds / iterations
- **colors**: number of colors used (max color id)

**Important:** we report the **MAX over ranks** for `total/comp/comm` (critical-path time).

Speedup is computed in plotting as:
`speedup(p) = T(p=1) / T(p)` (per algorithm, per dataset size).

## Reproducible workflow (lab server → laptop graphs)

### A) Build on the lab server

```bash
ssh mky@163.238.35.10
cd ~/mpi_gc_repo   # or wherever you copied the folder
make
```

### B) Generate datasets (n = 1000, 2000, 5000)

```bash
./graph_gen 1000 0.01 42 g1000.txt
./graph_gen 2000 0.01 42 g2000.txt
./graph_gen 5000 0.01 42 g5000.txt
```

### C) Run a single sanity test (recommended)

```bash
mpirun --oversubscribe --bind-to none -np 2 ./alg1 g1000.txt 123
mpirun --oversubscribe --bind-to none -np 2 ./alg2 g1000.txt 123
```

### D) Run the full benchmark matrix (p = 1, 2, 4, 8; both algs; all 3 sizes)

```bash
./run_bench.sh
wc -l results.csv   # expected: 25 (header + 24 runs)
head results.csv
```

### E) Copy results to your laptop

Run this on your **laptop terminal** (NOT inside SSH):

```bash
mkdir -p ~/Downloads/mpi_plots
scp mky@163.238.35.10:~/mpi_gc_repo/results.csv ~/Downloads/mpi_plots/
```

### F) Plot the graphs on your laptop (PNG output)

> Note: many lab servers do not have plotting tools (no matplotlib/gnuplot). Plot locally.

```bash
cd ~/Downloads/mpi_plots
python3 -m venv .venv
source .venv/bin/activate
python -m pip install matplotlib
cp ~/Downloads/mpi_gc_repo/plot_local.py .
python plot_local.py
ls -1 *.png
open comp_n1000.png
```

This generates per dataset size `n`:
- `total_n{n}.png`
- `comp_n{n}.png`
- `comm_n{n}.png`
- `speedup_n{n}.png`
- `colors_n{n}.png`

## Optional: correctness check (conflict-free coloring)

Save colors (rank 0 writes the file):

```bash
mpirun --oversubscribe --bind-to none -np 4 ./alg1 g1000.txt 123 colors_alg1_n1000.txt
python3 check_coloring.py g1000.txt colors_alg1_n1000.txt
```

Same for Alg2:

```bash
mpirun --oversubscribe --bind-to none -np 4 ./alg2 g1000.txt 123 colors_alg2_n1000.txt
python3 check_coloring.py g1000.txt colors_alg2_n1000.txt
```
