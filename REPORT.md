# Parallel Graph Coloring Using MPI — Report Notes (Template)

## Goal
Implement and evaluate parallel graph coloring using MPI, and compare two strategies (Alg1 vs Alg2) while measuring **total**, **computation**, and **communication** time separately.

## Algorithms
**Alg1 (Partitioning Strategy).** Vertices are partitioned across ranks. In each iteration, each rank attempts to color eligible local vertices (greedy choice) and then synchronizes the global color array. The algorithm repeats until all vertices are colored.

**Alg2 (MIS / Divide-and-Conquer).** The algorithm repeatedly selects a Maximal Independent Set (MIS) from the active vertices, assigns a new color to that MIS, removes those vertices, and repeats until no active vertices remain.

## Experimental setup
- Graph sizes: `n = 1000, 2000, 5000`
- Process counts: `p = 1, 2, 4, 8`
- Graphs are generated using `graph_gen` with a fixed seed for reproducibility.

## Metrics recorded
Each run outputs:

`alg,n,p,total,comp,comm,rounds,colors`

- **total (s):** end-to-end runtime (MPI_Wtime)
- **comp (s):** time in local computation regions (no MPI calls)
- **comm (s):** time spent in MPI calls (broadcast/gather/reduce/etc.)
- **rounds:** number of iterations until termination
- **colors:** number of colors used

We report `total/comp/comm` as the **maximum across ranks** (critical-path time).

**Speedup** is computed as:
`speedup(p) = T(p=1) / T(p)` for each algorithm and each graph size.

## What plots to include (minimum)
For each `n`:
- total vs processes
- computation vs processes
- communication vs processes
- speedup vs processes
(Optional) colors vs processes

## Reproduction steps (copy/paste)
1) Build: `make`
2) Generate inputs: `./graph_gen ...`
3) Run benchmark: `./run_bench.sh` → `results.csv`
4) Copy results to laptop and run `plot_local.py` → PNG graphs

## Discussion ideas (what to write in a few sentences)
- Which algorithm is faster for each `n` and why?
- Does speedup improve from p=1→2→4→8 or plateau?
- Is runtime dominated by computation or communication as `p` increases?
- How many colors are used by each algorithm and does it change with `p`?
