#pragma once
#include <cstdint>
#include <cstddef>
#include "../transforms/config.hpp"

struct MAYBE_ALIGN Msg { uint8_t* ptr; size_t len; };

inline uint32_t hot_parser(const Msg& m) {
  // Parse a few tag = value fields, branchy and cache touchy
  const uint8_t* p = m.ptr;
  const uint8_t* end = m.ptr + m.len;
  uint32_t checksum = 0;

  #pragma clang loop unroll_count(UNROLL_FACTOR)

  for (; p+8 <= end; p+=1) {
    uint8_t c = *p;
    checksum += c;

    // flatten a small branch - is digit ?
    uint8_t is_digit = (c >= '0' && c <= '9');
    uint8_t val = FLATTENED_SELECT(is_digit, (uint8_t)(c - '0'), (uint8_t)0);
    checksum += val * 3;
  }
  return checksum;
}
