#pragma once
#include <vector>
#include <cstdint>
#include <time.h>

inline uint64_t mono_ns() {
  timespec ts; clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
  return uint64_t(ts.tv_sec) * 1000000000ull + ts.tv_nsec;
}

template<class Fn>
/*
store ns per call
Avoid timer granularity/overhead at sub 100 ns scales 
Batching lets the sample represent the kernel, not the timer

Tail stats are data-hungry. You need thousands of samples to stabilize p99/p99.9. 
Mean is easy; jitter is not.
*/
inline void bench(Fn f, size_t batches, size_t iters_per_batch, std::vector<uint32_t>& samples) {
  samples.reserve(batches);

  // Warmup
  for (int i = 0; i < 100000; i++) f();
  for (size_t b = 0; b < batches; ++b) {
    auto t0 = mono_ns();

    for (size_t i = 0; i < iters_per_batch; i++) f();
    auto t1 = mono_ns();
    uint64_t dt = (t1 - t0) / iters_per_batch; // ns per call
    samples.push_back((uint32_t)dt);
  }
}
