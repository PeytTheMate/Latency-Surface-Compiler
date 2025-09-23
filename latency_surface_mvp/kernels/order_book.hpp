#pragma once

#include <cstdint>

#include "../transforms/config.hpp"

// Signal to bench_main.cpp that the real kernel implementation is available so
// the placeholder stub can stay dormant.
#ifndef LATENCY_SURFACE_HAVE_ORDER_BOOK
#define LATENCY_SURFACE_HAVE_ORDER_BOOK 1
#endif

namespace latency_surface {

// --
// Quote layouts used by the order book micro-kernel.  The AoS layout keeps
// price/qty/id together, whereas the SoA layout maintains three parallel
// arrays.  Both support the same query routine below.
// --

struct MAYBE_ALIGN QuoteAoS {
  float     price;
  uint32_t  qty;
  uint32_t  id;
};

struct QuoteSoA {
  float*     price;
  float*     qty;
  uint32_t*  id;
};

namespace detail {

// Helper that performs a branch-flatten friendly select between two values.
inline int select_idx(bool take_new, int candidate, int current) {
#if BRANCH_FLATTEN
  return FLATTENED_SELECT(take_new, candidate, current);
#else
  return take_new ? candidate : current;
#endif
}

inline uint32_t select_qty(bool take_new, uint32_t candidate, uint32_t current) {
#if BRANCH_FLATTEN
  return FLATTENED_SELECT(take_new, candidate, current);
#else
  return take_new ? candidate : current;
#endif
}

}  // namespace detail

// Return the (index, qty) pair for the deepest price level whose price is less
// than or equal to `target`.  The result is encoded as a 32-bit integer to keep
// the hot path compact: the upper 16 bits carry the index (or 0xFFFF if no
// level matched) and the lower 16 bits carry the quantity at that level.
inline int encode_result(int idx, uint32_t qty) {
  const uint32_t idx_bits = (idx < 0) ? 0xFFFFu : static_cast<uint32_t>(idx & 0xFFFF);
  const uint32_t qty_bits = qty & 0xFFFFu;
  return static_cast<int>((idx_bits << 16) | qty_bits);
}

// -- AoS kernel ------------------------------------------------------------
inline int hot_find_level(QuoteAoS* book, int n, float target) {
  int best_idx = -1;
  uint32_t best_qty = 0;

  #pragma clang loop unroll_count(UNROLL_FACTOR)
  for (int i = 0; i < n; ++i) {
    const float px = book[i].price;
    const bool take = (px <= target);

    best_idx = detail::select_idx(take, i, best_idx);
    // When a new best level is selected, also keep its quantity.  If we keep
    // the previous best, preserve the existing quantity.
    best_qty = detail::select_qty(take, book[i].qty, best_qty);
  }

  return encode_result(best_idx, best_qty);
}

// -- SoA kernel ------------------------------------------------------------
inline int hot_find_level(const QuoteSoA& book, int n, float target) {
  int best_idx = -1;
  uint32_t best_qty = 0;

  #pragma clang loop unroll_count(UNROLL_FACTOR)
  for (int i = 0; i < n; ++i) {
    const float px = book.price[i];
    const bool take = (px <= target);

    best_idx = detail::select_idx(take, i, best_idx);
    const uint32_t qty = static_cast<uint32_t>(book.qty[i]);
    best_qty = detail::select_qty(take, qty, best_qty);
  }

  return encode_result(best_idx, best_qty);
}

}  // namespace latency_surface

// Preserve the historical unqualified names for ease of inclusion in the
// benchmark driver.
using QuoteAoS = latency_surface::QuoteAoS;
using QuoteSoA = latency_surface::QuoteSoA;
using latency_surface::hot_find_level;

// Retain the helper binary search used by analyzer utilities.
inline int upper_bound_idx(const int* book, int n, int target) {
  int lo = 0;
  int hi = n - 1;

  while (lo <= hi) {
    int mid = (lo + hi) >> 1;
    bool le = (book[mid] <= target);

#if BRANCH_FLATTEN
    lo = FLATTENED_SELECT(le, mid + 1, lo);
    hi = FLATTENED_SELECT(le, hi,        mid - 1);
#else
    if (le) {
      lo = mid + 1;
    } else {
      hi = mid - 1;
    }
#endif
  }
  return hi;  // last <= target (or -1 if none)
}
