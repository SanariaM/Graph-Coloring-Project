#pragma once
#include <mpi.h>

typedef struct {
    double comp;
    double comm;
} timing_t;

static inline double now() { return MPI_Wtime(); }
static inline void add_comp(timing_t* t, double dt) { t->comp += dt; }
static inline void add_comm(timing_t* t, double dt) { t->comm += dt; }
