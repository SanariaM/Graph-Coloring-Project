CC=mpicc
CFLAGS=-O2 -Wall

all: graph_gen alg1 alg2

graph_gen: graph_gen.c
	$(CC) $(CFLAGS) -o graph_gen graph_gen.c

alg1: alg1_partition.c
	$(CC) $(CFLAGS) -o alg1 alg1_partition.c

alg2: alg2_mis.c
	$(CC) $(CFLAGS) -o alg2 alg2_mis.c

clean:
	rm -f graph_gen alg1 alg2 *.o *.txt results.csv *.png results_* *.out
