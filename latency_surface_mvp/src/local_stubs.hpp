#pragma once
#include <cstdint>

// Minimal placeholder type so bench_main.cpp compiles.
// Replace with the real definition later if/when available.
struct QuoteAoS {
  float price;
  uint32_t qty;
};

// Minimal stub so hot_find_level is declared and available.
static inline uint64_t hot_find_level(QuoteAoS* data, int n, float v) {
  (void)data;
  (void)n;
  (void)v;
  return 0ULL;
}
