#pragma once
#include <cstdint>

// If your build doesn't define these, provide harmless defaults so the header
// can compile both with and without "flattened" branch updates.
#ifndef BRANCH_FLATTEN
#define BRANCH_FLATTEN 0
#endif

#ifndef FLATTENED_SELECT
// Fallback: just use the ternary when not doing branch-flatten tricks.
#define FLATTENED_SELECT(cond, a, b) ((cond) ? (a) : (b))
#endif

// Return the index of the last element <= target in a sorted array `book[0..n)`.
// If all elements are > target, returns -1.
// Works in both the normal (branchy) path and a "flattened" (branchless-ish) path.

inline int upper_bound_idx(const int* book, int n, int target) {
  int lo = 0;
  int hi = n - 1;

  while (lo <= hi) {
    int mid = (lo + hi) >> 1;
    bool le = (book[mid] <= target);

#if BRANCH_FLATTEN
    // Flattened update using your select primitive
    // lo' = le ? mid+1 : lo
    // hi' = le ? hi    : mid-1
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
  return hi; // last <= target (or -1 if none)
}
