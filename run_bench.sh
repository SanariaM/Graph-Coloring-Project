#!/usr/bin/env bash
set -u

MPI="mpirun --oversubscribe --bind-to none"

echo "alg,n,p,total,comp,comm,rounds,colors" > results.csv

for n in 1000 2000 5000; do
  g="g${n}.txt"
  if [[ ! -f "$g" ]]; then
    echo "Missing $g. Generate it first with ./graph_gen ..." >&2
    exit 1
  fi

  for p in 1 2 4 8; do
    $MPI -np $p ./alg1 $g 123 >> results.csv || echo "alg1 failed n=$n p=$p" >&2
    $MPI -np $p ./alg2 $g 123 >> results.csv || echo "alg2 failed n=$n p=$p" >&2
  done
done

echo "DONE: results.csv created"
