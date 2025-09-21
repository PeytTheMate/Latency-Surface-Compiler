#pragma once
#include <cstdint>
#include <cstddef>
#include <algorithm>
#include "../transforms/config.hpp"


struct QuoteAoS { float px; float qty; uint32_t id; };
struct QuoteSoA { float* px; float* qty; uint32_t* id; };

template<typename Book>
inline int hot_find_level(Book& b, int n, float target) {
  int lo = 0;
  hi = n - 1;

  #pragma clang loop unroll_count(UNROLL_FACTOR)

  while (lo <= hi) {
    int mid = (lo + hi) >> 1;

#if LAYOUT_AOS
    float px = ((QuoteAoS*)&b)[mid].px;

#else
    float px = b.px[mid];

#endif
    // branch flattening for cmp
    bool le = (px <= target);
    lo = FLATTENED_SELECT(le, mid + 1, lo);
    hi = FLATTENED_SELECT(le, hi, mid - 1);

  }
  return hi;
}
