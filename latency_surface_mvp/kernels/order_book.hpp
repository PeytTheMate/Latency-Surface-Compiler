#pragma once
#include <cstdint>

// defaults for flattening hooks
#ifndef BRANCH_FLATTEN
#define BRANCH_FLATTEN 0
#endif

#ifndef FLATTENED_SELECT
#define FLATTENED_SELECT(cond, a, b) ((cond) ? (a) : (b))
#endif

// Return index of last element <= target in sorted book[0..n).
// Returns -1 if all elements > target.
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
  return hi;
}
