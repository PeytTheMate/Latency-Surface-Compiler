import json, statistics, math
from pathlib import Path


# Post-processing for autotuner results.
    # - Loads summary.json
    # - Finds best config per kernel
    # - Compares against baseline, reports % improvements (p99, p99.9, stdev)
    # - Prints concise JSON summary


ROOT = Path(__file__).resolve().parents[1]
summary = json.loads((ROOT/"data/reports/summary.json").read_text())

# For each kernel: pick configs on Pareto frontier (J minimal, size cap)
by_k = {}

for row in summary:

    k = row["kernel"]
    by_k.setdefault(k, []).append(row)

report = {}

for k, rows in by_k.items():

    rows_sorted = sorted(rows, key=lambda r: r["J"])
    best = rows_sorted[0]
    base = [r for r in rows if r["params"]==dict(u = 1,pf = 0,bf = 0,la = 1,al = 0)][0]

    report[k] = {
        "baseline": {"stats":base["stats"], "params":base["params"]},
        "best": {"stats":best["stats"], "params":best["params"]},
        "improvement_p99_%": 100.0*(base["stats"]["p99"] - best["stats"]["p99"])/base["stats"]["p99"],
        "improvement_p999_%": 100.0*(base["stats"]["p999"] - best["stats"]["p999"])/base["stats"]["p999"],
        "stdev_drop_%": 100.0*(base["stats"]["stdev"] - best["stats"]["stdev"])/base["stats"]["stdev"],
        "size_growth_x": best["stats"]["size_rel"]
    }

print(json.dumps(report, indent=2))
