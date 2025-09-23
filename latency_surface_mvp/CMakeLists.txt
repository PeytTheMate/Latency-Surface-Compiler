cmake_minimum_required(VERSION 3.20)
project(latency_surface_mvp CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Release)

# Toggle-able options with defaults that mirror transforms/config.hpp
if(NOT DEFINED UNROLL_FACTOR)
  set(UNROLL_FACTOR 1)
endif()
if(NOT DEFINED PREFETCH_DIST)
  set(PREFETCH_DIST 0)
endif()
if(NOT DEFINED BRANCH_FLATTEN)
  set(BRANCH_FLATTEN 0)
endif()
if(NOT DEFINED LAYOUT_AOS)
  set(LAYOUT_AOS 1)
endif()
if(NOT DEFINED ALIGN_BYTES)
  set(ALIGN_BYTES 0)
endif()

foreach(opt IN ITEMS UNROLL_FACTOR PREFETCH_DIST BRANCH_FLATTEN LAYOUT_AOS ALIGN_BYTES)
  add_compile_definitions(${opt}=${${opt}})
endforeach()

add_executable(bench src/bench_main.cpp)
target_include_directories(bench PRIVATE include kernels transforms)

# Friendly warnings + fast math off (we care about control/latency)
target_compile_options(bench PRIVATE -O3 -fno-exceptions -fno-rtti -fno-omit-frame-pointer -march=native)
