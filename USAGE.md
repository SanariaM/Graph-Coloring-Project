# Usage (One Page)

## Lab server (build + run)
```bash
make
./graph_gen 1000 0.01 42 g1000.txt
./graph_gen 2000 0.01 42 g2000.txt
./graph_gen 5000 0.01 42 g5000.txt
./run_bench.sh
wc -l results.csv   # expected: 25
```

## Laptop (plot)
```bash
mkdir -p ~/Downloads/mpi_plots
scp mky@163.238.35.10:~/mpi_gc_repo/results.csv ~/Downloads/mpi_plots/
cd ~/Downloads/mpi_plots
python3 -m venv .venv
source .venv/bin/activate
python -m pip install matplotlib
cp ~/Downloads/mpi_gc_repo/plot_local.py .
python plot_local.py
```
