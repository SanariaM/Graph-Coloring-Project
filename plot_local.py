import csv
import matplotlib.pyplot as plt

# results.csv columns:
# alg,n,p,total,comp,comm,rounds,colors

rows = []
with open("results.csv", newline="") as f:
    for row in csv.DictReader(f):
        row["n"] = int(row["n"])
        row["p"] = int(row["p"])
        row["total"] = float(row["total"])
        row["comp"]  = float(row["comp"])
        row["comm"]  = float(row["comm"])
        row["rounds"] = int(row["rounds"])
        row["colors"] = int(row["colors"])
        rows.append(row)

# speedup baseline per (alg,n) at p=1
base = {(r["alg"], r["n"]): r["total"] for r in rows if r["p"] == 1}
for r in rows:
    r["speedup"] = base[(r["alg"], r["n"])] / r["total"]

for n in sorted(set(r["n"] for r in rows)):
    sub = [r for r in rows if r["n"] == n]
    for metric in ["total", "comp", "comm", "speedup", "colors"]:
        plt.figure()
        for alg in ["alg1", "alg2"]:
            a = sorted([r for r in sub if r["alg"] == alg], key=lambda x: x["p"])
            plt.plot([x["p"] for x in a], [x[metric] for x in a], marker="o", label=alg)
        plt.xlabel("Processes (p)")
        plt.ylabel(metric)
        plt.title(f"{metric} vs processes (n={n})")
        plt.grid(True)
        plt.legend()
        plt.savefig(f"{metric}_n{n}.png", dpi=200, bbox_inches="tight")
        plt.close()

print("DONE: created PNG files")
