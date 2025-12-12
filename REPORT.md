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

## Discussion ideas
**- Which algorithm is faster for each `n` and why?**

- n=1000: Alg2 is faster for p=1,2,4, but at p=8 Alg1 becomes faster in your data. A clean explanation: Alg2 has fewer rounds (less work) so it wins at small–medium p, but at higher p the communication overhead starts to dominate, and Alg1 may have lower/steadier overhead (or scales better at that p).

- n=2000 and n=5000: Write it like this: “For n=2000 (and n=5000), the faster algorithm is the one with the smaller total time across p. The reason is mainly the balance between (1) how many rounds/iterations the algorithm needs and (2) how much synchronization/communication it performs per round.
**- Does speedup improve from p=1→2→4→8 or plateau?**
- In an ideal case, speedup increases as p increases.

- In your runs, speedup typically improves from p=1 to p=2, but then often plateaus (or even gets worse at p=4) when the communication cost becomes a big fixed overhead. That’s exactly what a “communication-limited” parallel program looks like.

**- Is runtime dominated by computation or communication as `p` increases?**
- Your data strongly suggests: as p increases, computation time drops, but communication time stays large/flat, so the runtime becomes communication-dominated at higher p.
  
- That explains the plateau: once communication dominates, adding processes doesn’t help much.

**- How many colors are used by each algorithm and does it change with `p`?**
- For n=1000 in your sample rows: Alg1 uses 8 colors and Alg2 uses 14 colors, and these values look stable across p (not changing with process count).
  
- In general: the #colors is mostly determined by the algorithm’s selection strategy (priorities / MIS selection) and the random seed, not by p—so it usually does not change much with p (unless the parallelization changes tie-breaking behavior).
