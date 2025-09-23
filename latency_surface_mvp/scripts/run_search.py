import os, subprocess, itertools, csv, json, statistics, math
from pathlib import Path

# If we want speed later, we can switch to Bayesian optimization, this is just for MVP
# Grid search autotuner for latency surface MVP
    # - Iterates over discrete transform configs (unroll, prefetch, and such)
    # - Compiles each variant via CMake, runs benchmarks, collects CSVs
    # - Computes p50/p90/p99/p999/stdev + code size
    # - Applies weighted jitter-aware objective (favor p999) under size cap
# Outputs summary.json with per-kernel results
# Design intent: keep search simple & reproducible on a laptop.

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / "build"
DATA  = ROOT / "data" / "runs"
DATA.mkdir(parents=True, exist_ok=True)


def parse_int_list(env_key, default):
    raw = os.environ.get(env_key)
    if not raw:
        return default
    parts = [p.strip() for p in raw.split(",") if p.strip()]
    if not parts:
        return default
    try:
        return [int(p) for p in parts]
    except ValueError as exc:
        raise ValueError(f"Environment variable {env_key} must be a comma-separated list of integers") from exc

KERNELS = ["parser", "ring", "obook"]

UNROLLS  = parse_int_list("LSC_UNROLLS",  [1,2,4,8])
PREFETCH = parse_int_list("LSC_PREFETCH", [0,32,64])
FLATTEN  = parse_int_list("LSC_FLATTEN",  [0,1])
LAYOUT   = parse_int_list("LSC_LAYOUT",   [0,1])
ALIGN    = parse_int_list("LSC_ALIGN",    [0,64])

BATCHES = int(os.environ.get("LSC_BATCHES", "20000"))
ITERS   = int(os.environ.get("LSC_ITERS", "64"))

def compile_variant(params, jobs: int):
  # Configure with variant flags
  cfg = [
    "cmake",
    f"-DUNROLL_FACTOR={params['u']}",
    f"-DPREFETCH_DIST={params['pf']}",
    f"-DBRANCH_FLATTEN={params['bf']}",
    f"-DLAYOUT_AOS={params['la']}",
    f"-DALIGN_BYTES={params['al']}",
    ".."
  ]
  subprocess.run(cfg, cwd=BUILD, check=True)
  # Build immediately after reconfigure
  subprocess.run(["cmake", "--build", ".", "--parallel", str(jobs)], cwd=BUILD, check=True)

def code_size_bytes():
  exe = BUILD / "bench"
  return os.path.getsize(exe)

def run_kernel(kernel, outfile):
    exe = BUILD / "bench"
    res = subprocess.run([str(exe), kernel, str(outfile), str(BATCHES), str(ITERS)],
                         cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if res.returncode != 0:
        raise RuntimeError(f"bench failed for {kernel}:\nSTDOUT:\n{res.stdout}\nSTDERR:\n{res.stderr}")


def evaluate(csv_path):
    vals = []
    with open(csv_path) as f:
        first = f.readline().strip()
        if first == "ns":
            vals = [int(row["ns"]) for row in csv.DictReader([first] + f.readlines())]
        else:
            # treat file as plain newline-separated integers; include first line
            vals = [int(first)] + [int(line.strip()) for line in f if line.strip()]
    vals.sort()
    n = len(vals)
    def q(p): 
        i = max(0, min(n-1, math.ceil(p*n)-1))
        return vals[i]
    return {
        "p50": q(0.50), "p90": q(0.90), "p99": q(0.99), "p999": q(0.999),
        "stdev": statistics.pstdev(vals), "mean": statistics.mean(vals)
    }

def score(stats, size):
    """Weighted jitter-aware objective (favor p999 latency, penalize large binaries)."""
    jitter_penalty = stats["p999"] + 0.1 * stats["stdev"]
    size_penalty = size / 1e6  # scale MB
    return jitter_penalty + size_penalty


def main():
  BUILD.mkdir(exist_ok=True)
  # One-time baseline configure (generates build system)
  subprocess.run(["cmake", ".."], cwd=BUILD, check=True)

  jobs = os.cpu_count() or 4

  base = dict(u=1, pf=0, bf=0, la=1, al=0)
  compile_variant(base, jobs)
  base_size = code_size_bytes()

  results = []

  # Baseline runs for each kernel reuse the same binary.
  for kernel in KERNELS:
    base_csv = DATA / f"{kernel}_base.csv"
    run_kernel(kernel, base_csv)
    base_stats = evaluate(base_csv)
    base_stats["size"] = base_size
    base_stats["size_rel"] = 1.0
    results.append({
      "kernel": kernel,
      "params": base.copy(),
      "stats": base_stats,
      "J": score(base_stats, base_size)
    })

  # Iterate over the search space once, compiling each variant and then
  # benchmarking it across all kernels.
  for u, pf, bf, la, al in itertools.product(UNROLLS, PREFETCH, FLATTEN, LAYOUT, ALIGN):
    params = dict(u=u, pf=pf, bf=bf, la=la, al=al)
    if params == base:
      continue

    compile_variant(params, jobs)
    sz = code_size_bytes()
    suffix = f"u{u}_p{pf}_b{bf}_l{la}_a{al}"

    for kernel in KERNELS:
      out_csv = DATA / f"{kernel}_{suffix}.csv"
      run_kernel(kernel, out_csv)
      stats = evaluate(out_csv)
      stats["size"] = sz
      stats["size_rel"] = sz / base_size if base_size else 1.0
      results.append({
        "kernel": kernel,
        "params": params.copy(),
        "stats": stats,
        "J": score(stats, sz)
      })

  # Save summary
  out = ROOT / "data/reports/summary.json"
  out.parent.mkdir(parents=True, exist_ok=True)
  with open(out, "w") as f:
    json.dump(results, f, indent=2)
  print(f"Wrote {out}")

if __name__=="__main__":
  main()
