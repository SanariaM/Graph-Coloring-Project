#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Usage: ./graph_gen n p_edge seed output.txt
int main(int argc, char** argv) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s n p_edge seed output.txt\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[1]);
    double p = atof(argv[2]);
    unsigned seed = (unsigned)atoi(argv[3]);
    const char* out = argv[4];

    FILE* f = fopen(out, "w");
    if (!f) { perror("fopen"); return 1; }

    fprintf(f, "%d\n", n);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int val = 0;
            if (i == j) val = 0;
            else {
                unsigned a = (i < j) ? (unsigned)i : (unsigned)j;
                unsigned b = (i < j) ? (unsigned)j : (unsigned)i;

                // deterministic hash -> "random-ish" but reproducible
                unsigned x = (a * 2654435761u) ^ (b * 2246822519u) ^ seed;
                x ^= x >> 16; x *= 0x7feb352d; x ^= x >> 15; x *= 0x846ca68b; x ^= x >> 16;
                double r = (x / (double)UINT_MAX);
                val = (r < p) ? 1 : 0;
            }
            fprintf(f, "%d%c", val, (j == n-1) ? '\n' : ' ');
        }
    }

    fclose(f);
    return 0;
}
