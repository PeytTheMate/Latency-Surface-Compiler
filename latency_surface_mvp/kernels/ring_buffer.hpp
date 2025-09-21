#pragma once
#include <cstdint>
#include <cstddef>
#include <atomic>
#include "../transforms/config.hpp"

// Tests contiguous writes and potential cache ping-pong 

// PREFETCH can help with larger entries 

// ALIGN_BYTES reduces line splits

template<typename T, size_t N>
struct MAYBE_ALIGN Ring {
  T buf[N];
  std::atomic<size_t> head{0}, tail{0};
};

template<typename T, size_t N>
inline void hot_ring_write(Ring<T,N>& r, const T* src, size_t cnt) {
  size_t h = r.head.load(std::memory_order_relaxed);
  #pragma clang loop unroll_count(UNROLL_FACTOR)

  for (size_t i=0;i<cnt;i++) {
    T* slot = &r.buf[(h + i)&(N - 1)];
    PREFETCH(slot + PREFETCH_DIST/sizeof(T));
    *slot = src[i];
  }
  r.head.store(h + cnt, std::memory_order_release);
}
