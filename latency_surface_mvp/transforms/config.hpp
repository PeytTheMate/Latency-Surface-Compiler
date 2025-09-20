#pragma once

#ifndef UNROLL_FACTOR
#define UNROLL_FACTOR 1
#endif

#ifndef PREFETCH_DIST
#define PREFETCH_DIST 0
#endif

#ifndef BRANCH_FLATTEN
#define BRANCH_FLATTEN 0
#endif

#ifndef LAYOUT_AOS
#define LAYOUT_AOS 1
#endif

#ifndef ALIGN_BYTES
#define ALIGN_BYTES 0
#endif

#if ALIGN_BYTES
  #define MAYBE_ALIGN alignas(ALIGN_BYTES)
#else
  #define MAYBE_ALIGN
#endif

// tiny helpers
#if BRANCH_FLATTEN
  #define FLATTENED_SELECT(cond, a, b) ((-(int)(!!(cond)) & ((a)^(b))) ^ (b))
#else
  #define FLATTENED_SELECT(cond, a, b) ((cond) ? (a) : (b))
#endif

#if PREFETCH_DIST
  #if defined(__GNUC__) || defined(__clang__)
    #define PREFETCH(ptr) __builtin_prefetch((ptr), 0, 3)
  #else
    #define PREFETCH(ptr) do{}while(0)
  #endif
#else
  #define PREFETCH(ptr) do{}while(0)
#endif
