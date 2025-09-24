# Latency-Surface MVP Compiler

This is the beginning of my open-source latency-focused compiler harness, designed around micro-kernels inspired by quant finance workloads. The goal is to autotune low-level compiler transforms (like loop unrolling, memory layout, prefetching) for tail latency performance, specifically tailoring to **p999 latency**.

---

## Overview

The project wraps a CMake-driven micro-kernel benchmark harness. It benchmarks three kernels:

- **Parser** — lightweight parsing workloads  
- **Ring Buffer** — queue-like data movement  
- **Order Book** — a simplified market data kernel scanning price levels in both AoS and SoA layouts  

These kernels are executed via `bench_main.cpp`, which batches iterations to stabilize measurements and writes per-call samples to CSV for downstream analysis. Timing fidelity is handled in `bench.hpp`, which ensures cross-platform consistency and prevents the compiler from optimizing away work.

An autotuning workflow (`run_search.py`) sweeps transform parameters, rebuilds the benchmark executable, runs tests, and records latency distributions. Post-processing (`analyze.py`) and visualization (`plot_report.py`) summarize results and render ECDF curves to inspect jitter.

---

## Prerequisites

- **CMake** ≥ 3.20  
- **C++20 compiler** (tested with Clang/GCC)  
- **Python 3.x** with:
  - `pandas`
  - `matplotlib`

---

## Build Instructions

Clone the repository and build the project:

```bash
cmake -S . -B build
cmake --build build
