#pragma once

#ifndef LATENCY_SURFACE_HAVE_ORDER_BOOK

#include <cstdint>

// Minimal placeholder types so bench_main.cpp can still compile even if the
// real order book kernel is unavailable.  These definitions intentionally stay
// tiny; they should only be used in degenerate builds.
struct QuoteAoS {
  float     price;
  uint32_t  qty;
};

struct QuoteSoA {
  float*     price;
  float*     qty;
  uint32_t*  id;
};

static inline int hot_find_level(QuoteAoS*, int, float) {
  return -1;
}

static inline int hot_find_level(const QuoteSoA&, int, float) {
  return -1;
}

#endif  // LATENCY_SURFACE_HAVE_ORDER_BOOK
