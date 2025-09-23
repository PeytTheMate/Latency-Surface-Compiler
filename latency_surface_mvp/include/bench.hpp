// --
// Generic timing harness for micro-kernels.
// - macOS (Apple Silicon): uses mach_absolute_time() with timebase conversion
// - Linux/others: uses CLOCK_MONOTONIC_RAW
// - Batches iterations to avoid timer granularity issues
// - Produces a vector of per-call latency samples (ns)
// Notes:
// * If kernel might be optimized away by the compiler, return a value
//   from the lambda and feed it to do_not_optimize_away() inside the lambda.
//   Example:
//     uint32_t sink = 0;
//     bench([&](){ sink ^= hot_parser(m); do_not_optimize_away(sink); },
//           batches, iters, samples);
// --

#pragma once
#include <cstdint>
#include <vector>

#if defined(__APPLE__)
  #include <mach/mach_time.h>
  inline uint64_t mono_ns() {
    static mach_timebase_info_data_t ti = {0, 0};
    if (ti.denom == 0) {
      // Query once numer/denom convert absolute ticks to ns.
      (void)mach_timebase_info(&ti);
    }
    // NOTE: multiplication first is safe; 64-bit is enough for long runs
    return (mach_absolute_time() * ti.numer) / ti.denom;
  }
#else
  #include <time.h>
  inline uint64_t mono_ns() {
    timespec ts;
    // CLOCK_MONOTONIC_RAW avoids time adjustments (Linux). Falls back to
    // CLOCK_MONOTONIC on platforms where RAW is unavailable (handled by libc).
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return static_cast<uint64_t>(ts.tv_sec) * 1000000000ull + static_cast<uint64_t>(ts.tv_nsec);
  }
#endif

// Optional utility to prevent the compiler from optimizing away computed values
// Use inside your benchmark lambda if needed.
// Example: do_not_optimize_away(result);
#if defined(__GNUC__) || defined(__clang__)
  template <typename T>
  inline void do_not_optimize_away(const T& value) {
    asm volatile("" : : "g"(value) : "memory");
  }
#else
  template <typename T>
  inline void do_not_optimize_away(const T& /*value*/) {}
#endif

// Bench: runs `batches` samples; each sample is the avg per-call latency over
// `iters_per_batch` calls to `f`. The per-call latency (ns) for each batch is
// pushed into `samples`.
template <class Fn>
inline void bench(Fn&& f,
                  size_t batches,
                  size_t iters_per_batch,
                  std::vector<uint32_t>& samples) {
  samples.clear();
  samples.reserve(batches);

  // Warm-up to reach steady state (caches, branch predictors, etc.)
  for (int i = 0; i < 100000; ++i) {
    f();
  }

  // Main measurement loop: batch to reduce timer noise for fast kernels.
  for (size_t b = 0; b < batches; ++b) {
    const uint64_t t0 = mono_ns();
    for (size_t i = 0; i < iters_per_batch; ++i) {
      f();
    }
    const uint64_t t1 = mono_ns();

    const uint64_t denom = (iters_per_batch == 0) ? 1 : iters_per_batch;
    const uint64_t per_call_ns = (t1 - t0) / denom;

    // Clamp to 32-bit for compact storage; per-call should fit comfortably.
    samples.push_back(static_cast<uint32_t>(per_call_ns));
  }
}
