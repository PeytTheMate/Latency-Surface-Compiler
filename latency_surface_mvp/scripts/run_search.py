import os, subprocess, itertools, csv, json, statistics, shutil, time, math
from pathlib import Path
import pandas as pd
import numpy as np

# If we want speed later, we can switch to Bayesian optimization, this is just for MVP
# Grid search autotuner for latency surface MVP
    # - Iterates over discrete transform configs (unroll, prefetch, and such)
    # - Compiles each variant via CMake, runs benchmarks, collects CSVs
    # - Computes p50/p90/p99/p99.9/stdev + code size
    # - Applies weighted jitter-aware objective (favor p99.9) under size cap
# Outputs summary.json with per-kernel results
# Design intent: keep search simple & reproducible on a laptop.

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT/"build"
DATA  = ROOT/"data/runs"
DATA.mkdir(parents = True, exist_ok = True)

KERNELS = ["parser","ring","obook"]

UNROLLS   = [1,2,4,8]
PREFETCH  = [0,32,64]
FLATTEN   = [0,1]
LAYOUT    = [0,1]
ALIGN     = [0,64]

def percentiles(arr, ps = [50,90,99,99.9]):
  a = np.sort(np.array(arr))
  out = {}

  for p in ps:
    k = (len(a)-1)*p/100.0
    f = math.floor(k); c = math.ceil(k)

    if f==c: 
      v = a[int(k)]

    else: 
      v = a[f]*(c-k)+a[c]*(k-f)

    out[f"p{p}"]=float(v)

  return out

def compile_variant(params):
  cmd = ["cmake",
         f"-DUNROLL_FACTOR={params['u']}",
         f"-DPREFETCH_DIST={params['pf']}",
         f"-DBRANCH_FLATTEN={params['bf']}",
         f"-DLAYOUT_AOS={params['la']}",
         f"-DALIGN_BYTES={params['al']}",
         ".."]
  
  subprocess.run(cmd, cwd=BUILD, check=True, stdout=subprocess.DEVNULL)
  subprocess.run(["cmake","--build","-j"], cwd=BUILD, check=True, stdout=subprocess.DEVNULL)

def code_size_bytes():
  exe = BUILD/"bench"
  return os.path.getsize(exe)

def run_kernel(kernel, outfile):
  exe = BUILD/"bench"
  subprocess.run([str(exe), kernel, str(outfile), "20000", "64"], check=True, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

def evaluate(csv_path):
  with open(csv_path) as f:
    rdr = csv.DictReader(f)
    xs = [int(r["ns"]) for r in rdr]
  stats = percentiles(xs)
  stats["stdev"]=float(statistics.pstdev(xs))

  return stats

def score(obj, size, size_cap=1.5):
  # Multi-objective: minimize J = 3*p99.9 + 1.5*p99 + 0.5*stdev, with size cap (relative to baseline)
  if obj["size_rel"] > size_cap: 
    return 1e18
  
  return 3.0*obj["p99.9"] + 1.5*obj["p99"] + 0.5*obj["stdev"]

def main():
  BUILD.mkdir(exist_ok=True)
  subprocess.run(["cmake",".."], cwd=BUILD, check=True)
  # Baseline (near-zero transforms)
  base = dict(u=1,pf=0,bf=0,la=1,al=0)
  compile_variant(base)
  base_size = code_size_bytes()

  results = []
  for kernel in KERNELS:
    # Run baseline
    base_csv = DATA/f"{kernel}_base.csv"
    run_kernel(kernel, base_csv)
    base_stats = evaluate(base_csv)
    base_stats["size"]=base_size
    base_stats["size_rel"]=1.0
    results.append({"kernel":kernel,"params":base,"stats":base_stats,"J":score(base_stats, base_size)})

    for u,pf,bf,la,al in itertools.product(UNROLLS,PREFETCH,FLATTEN,LAYOUT,ALIGN):
      
      params=dict(u=u,pf=pf,bf=bf,la=la,al=al)
      if params==base: 
        continue
      
      compile_variant(params)
      sz = code_size_bytes()
      out_csv = DATA/f"{kernel}_u{u}_p{pf}_b{bf}_l{la}_a{al}.csv"
      run_kernel(kernel, out_csv)
      s = evaluate(out_csv)
      s["size"] = sz
      s["size_rel"] = sz/base_size
      J = score(s, sz)
      results.append({"kernel":kernel,"params":params,"stats":s,"J":J})

  # Save summary
  out = ROOT/"data/reports/summary.json"
  out.parent.mkdir(parents=True, exist_ok=True)
  with open(out,"w") as f: json.dump(results, f, indent=2)
  print(f"Wrote {out}")

if __name__=="__main__":
  main()
