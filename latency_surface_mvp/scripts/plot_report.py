import pandas as pd, numpy as np, json, matplotlib.pyplot as plt
from pathlib import Path

# Visualization of latency distributions
    # - For each kernel: baseline vs best config
    # - Plots ECDF curves (cumulative distribution of per call latency)
    # - Saves plots to data/reports/{}.png

# ECDFs are the most convincing way to demonstrate jitter reduction.

ROOT = Path(__file__).resolve().parents[1]
summary = json.loads((ROOT/"data/reports/summary.json").read_text())


def load_series(path):
    df = pd.read_csv(path)
    return df["ns"].values


def ecdf(xs):
    x = np.sort(xs)
    y = np.arange(1, len(x)+1)/len(x)
    return x,y

def find_paths(kernel, params):
    # reconstruct filename
    def tag(p): return f"u{p['u']}_p{p['pf']}_b{p['bf']}_l{p['la']}_a{p['al']}"
    base = ROOT/f"data/runs/{kernel}_base.csv"
    best = None
    for row in summary:
        if row["kernel"]==kernel and row["params"]==params:
            best = ROOT/f"data/runs/{kernel}_{tag(params)}.csv"
            break
    return base, best

# pick best per kernel (by J)
by_k = {}
for row in summary:
    k = row["kernel"]
    by_k.setdefault(k, []).append(row)

for k, rows in by_k.items():
    best = sorted(rows, key=lambda r: r["J"])[0]
    base_path, best_path = find_paths(k, best["params"])
    xb = load_series(base_path);  x1 = load_series(best_path)
    # ECDF
    xb_x, xb_y = ecdf(xb); x1_x, x1_y = ecdf(x1)

    plt.figure()
    plt.plot(xb_x, xb_y, label="baseline")
    plt.plot(x1_x, x1_y, label="best")
    plt.xlabel("Latency per call (ns)")
    plt.ylabel("ECDF")
    plt.title(f"ECDF: {k}")
    plt.legend()
    plt.tight_layout()
    out = ROOT/f"data/reports/{k}_ecdf.png"
    plt.savefig(out, dpi=160)
    plt.close()
