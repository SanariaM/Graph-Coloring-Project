#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "timers.h"

static int* read_and_bcast_graph(const char* path, int* n_out, int rank, timing_t* T) {
    int n = 0;
    int* adj = NULL;

    if (rank == 0) {
        FILE* f = fopen(path, "r");
        if (!f) { perror("fopen"); MPI_Abort(MPI_COMM_WORLD, 1); }
        if (fscanf(f, "%d", &n) != 1) { fprintf(stderr, "bad file\n"); MPI_Abort(MPI_COMM_WORLD, 1); }
        adj = (int*)malloc((size_t)n*(size_t)n*sizeof(int));
        for (int i = 0; i < n*n; i++) {
            if (fscanf(f, "%d", &adj[i]) != 1) { fprintf(stderr, "bad matrix\n"); MPI_Abort(MPI_COMM_WORLD, 1); }
        }
        fclose(f);
    }

    double t0 = now();
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    add_comm(T, now() - t0);

    if (rank != 0) adj = (int*)malloc((size_t)n*(size_t)n*sizeof(int));

    t0 = now();
    MPI_Bcast(adj, n*n, MPI_INT, 0, MPI_COMM_WORLD);
    add_comm(T, now() - t0);

    *n_out = n;
    return adj;
}

static void block_range(int n, int p, int r, int* start, int* end) {
    int base = n / p;
    int rem  = n % p;
    int s = r * base + (r < rem ? r : rem);
    int e = s + base + (r < rem ? 1 : 0);
    *start = s; *end = e;
}

static void write_colors_if_requested(int rank, const char* out_path, int n, const int* color) {
    if (!out_path || rank != 0) return;
    FILE* f = fopen(out_path, "w");
    if (!f) { perror("fopen(colors)"); return; }
    for (int i = 0; i < n; i++) fprintf(f, "%d\n", color[i]);
    fclose(f);
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, P;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &P);

    if (argc < 3) {
        if (rank == 0) fprintf(stderr, "Usage: %s graph.txt seed [colors_out.txt]\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    const char* graph_path = argv[1];
    int seed = atoi(argv[2]);
    const char* colors_out = (argc >= 4) ? argv[3] : NULL;

    timing_t T = {0,0};
    int n;
    int* adj = read_and_bcast_graph(graph_path, &n, rank, &T);

    int start, end;
    block_range(n, P, rank, &start, &end);

    int* color  = (int*)calloc((size_t)n, sizeof(int));
    int* active = (int*)malloc((size_t)n * sizeof(int));
    double* pr  = (double*)malloc((size_t)n * sizeof(double));
    int* inMIS  = (int*)calloc((size_t)n, sizeof(int));

    for (int i = 0; i < n; i++) active[i] = 1;

    int* counts = (int*)malloc((size_t)P * sizeof(int));
    int* displs  = (int*)malloc((size_t)P * sizeof(int));
    for (int r = 0; r < P; r++) {
        int s,e;
        block_range(n, P, r, &s, &e);
        counts[r] = e - s;
        displs[r] = s;
    }

    double total_start = now();
    int current_color = 1;
    int rounds = 0;

    while (1) {
        rounds++;

        int local_active = 0;
        for (int i = start; i < end; i++) local_active += active[i];
        int global_active = 0;

        double t0 = now();
        MPI_Allreduce(&local_active, &global_active, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        add_comm(&T, now() - t0);
        if (global_active == 0) break;

        double c0 = now();
        for (int v = 0; v < n; v++) pr[v] = -1.0;
        for (int v = start; v < end; v++) {
            if (!active[v]) continue;
            unsigned x = (unsigned)(v * 2654435761u) ^ (unsigned)(seed + 97*current_color);
            x ^= x >> 16; x *= 0x7feb352d; x ^= x >> 15; x *= 0x846ca68b; x ^= x >> 16;
            pr[v] = x / (double)UINT_MAX;
        }
        add_comp(&T, now() - c0);

        double* pr_local = (double*)malloc((size_t)(end-start) * sizeof(double));
        for (int i = start; i < end; i++) pr_local[i-start] = pr[i];

        t0 = now();
        MPI_Allgatherv(pr_local, end-start, MPI_DOUBLE,
                       pr, counts, displs, MPI_DOUBLE,
                       MPI_COMM_WORLD);
        add_comm(&T, now() - t0);
        free(pr_local);

        c0 = now();
        memset(inMIS, 0, (size_t)n * sizeof(int));
        for (int v = start; v < end; v++) {
            if (!active[v]) continue;

            int ok = 1;
            for (int u = 0; u < n; u++) {
                if (!adj[v*n + u]) continue;
                if (!active[u]) continue;
                if (pr[u] > pr[v] || (pr[u] == pr[v] && u > v)) { ok = 0; break; }
            }
            if (ok) inMIS[v] = 1;
        }
        add_comp(&T, now() - c0);

        int* inMIS_local = (int*)malloc((size_t)(end-start) * sizeof(int));
        for (int i = start; i < end; i++) inMIS_local[i-start] = inMIS[i];

        t0 = now();
        MPI_Allgatherv(inMIS_local, end-start, MPI_INT,
                       inMIS, counts, displs, MPI_INT,
                       MPI_COMM_WORLD);
        add_comm(&T, now() - t0);
        free(inMIS_local);

        c0 = now();
        for (int v = start; v < end; v++) {
            if (inMIS[v]) { color[v] = current_color; active[v] = 0; }
        }
        add_comp(&T, now() - c0);

        int* color_local  = (int*)malloc((size_t)(end-start) * sizeof(int));
        int* active_local = (int*)malloc((size_t)(end-start) * sizeof(int));
        for (int i = start; i < end; i++) { color_local[i-start] = color[i]; active_local[i-start] = active[i]; }

        t0 = now();
        MPI_Allgatherv(color_local, end-start, MPI_INT, color, counts, displs, MPI_INT, MPI_COMM_WORLD);
        MPI_Allgatherv(active_local, end-start, MPI_INT, active, counts, displs, MPI_INT, MPI_COMM_WORLD);
        add_comm(&T, now() - t0);

        free(color_local);
        free(active_local);

        current_color++;
    }

    double total_time = now() - total_start;

    double comp_max, comm_max, total_max;
    MPI_Reduce(&T.comp, &comp_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&T.comm, &comm_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&total_time, &total_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    int local_max_color = 0;
    for (int i = start; i < end; i++) if (color[i] > local_max_color) local_max_color = color[i];
    int colors_used = 0;
    MPI_Reduce(&local_max_color, &colors_used, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    write_colors_if_requested(rank, colors_out, n, color);

    if (rank == 0) {
        printf("alg2,%d,%d,%.6f,%.6f,%.6f,%d,%d\n",
               n, P, total_max, comp_max, comm_max, rounds, colors_used);
        fflush(stdout);
    }

    free(counts); free(displs);
    free(color); free(active); free(pr); free(inMIS); free(adj);
    MPI_Finalize();
    return 0;
}
