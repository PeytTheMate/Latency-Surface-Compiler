# ⚡ Parallax - Latency-Surface MVP Compiler

**Parallax** comes from physics, where perspective reveals hidden structure and depth.  
Just as parallax gives us a new dimension of understanding, this project aims to run **in parallel** to standard compilers, exposing a **new perspective on optimization**.  

This is the beginning of an open-source **latency-focused compiler harness**, inspired by quant finance workloads.  
The goal is to autotune low-level compiler transforms (loop unrolling, memory layout, prefetching, block placement, register allocation) for **tail latency performance** — specifically **p999 and beyond**.

---

## 🔹 Motivation

In ultra-low latency systems (like HFT), **the average cycles don’t matter, the tails do**.  
During network microbursts, rare stalls (cache/TLB contention, branch mispredicts, spills) dominate **P99–P99.99 latency** and directly impact P&L.  

Traditional compilers optimize for throughput.  
**Parallax (Latency-Surface Compilation)** instead optimizes for:  
- **Tail risk (Quantiles / CVaR)**, not just mean latency  
- **Smoothness across execution paths**, flattening spikes in the latency surface  
- **Burst robustness**, ensuring decision times don’t blow up under pressure  

---

## 🔹 Overview

The project currently wraps a **CMake-driven micro-kernel benchmark harness**.  
It benchmarks three quant-style kernels:

- **Parser** — lightweight parsing workloads  
- **Ring Buffer** — queue-like data movement  
- **Order Book** — simplified market data kernel scanning price levels in both AoS and SoA layouts  

Execution is driven by `bench_main.cpp`, which batches iterations to stabilize measurements and writes per-call samples to CSV.  
Timing fidelity (`bench.hpp`) ensures cross-platform consistency and prevents compilers from optimizing away work.

An autotuning workflow (`run_search.py`) sweeps transform parameters, rebuilds executables, runs benchmarks, and records latency distributions.  
Post-processing (`analyze.py`) and visualization (`plot_report.py`) summarize results and render **ECDF curves** to inspect jitter.

---

## 🔹 Prerequisites

- **CMake** ≥ 3.20  
- **C++20 compiler** (tested with Clang/GCC)  
- **Python 3.x** with:
  - `pandas`
  - `matplotlib`

---

## 🔹 Build Instructions

Clone the repository and build the project:

```bash
cmake -S . -B build
cmake --build build
