#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <numeric>
#include "../include/bench.hpp"
#include "../kernels/parser_fix.hpp"
#include "../kernels/ring_buffer.hpp"
#include "../kernels/order_book.hpp"

int main(int argc, char** argv) {
  const char* kernel = (argc > 1) ? argv[1] : "parser";
  const char* out = (argc > 2) ? argv[2] : "data/runs/out.csv";
  size_t batches = (argc > 3) ? std::stoul(argv[3]) : 20000;
  size_t iters   = (argc > 4) ? std::stoul(argv[4]) : 64;

  std::vector<uint32_t> samples;

  if (std::string(kernel) == "parser") {

    std::vector<uint8_t> buf(256);
    for (size_t i=0;i<buf.size();++i) buf[i] = "012345=|"[i%8];
    Msg m{buf.data(), buf.size()};
    bench([&](){ (void)hot_parser(m); }, batches, iters, samples);

  } else if (std::string(kernel)=="ring") {

    static Ring<uint64_t, 1<<12> r;
    std::vector<uint64_t> src(32, 42);
    bench([&](){ hot_ring_write(r, src.data(), src.size()); }, batches, iters, samples);

  } else if (std::string(kernel)=="obook") {
    constexpr int N=1024;

#if LAYOUT_AOS
    std::vector<QuoteAoS> b(N);
    bench([&](){ (void)hot_find_level(*(QuoteAoS(*) )b.data(), N, 123.4f); }, batches, iters, samples);

#else
    std::vector<float> px(N), qty(N); std::vector<uint32_t> id(N);
    QuoteSoA B{px.data(), qty.data(), id.data()};
    bench([&](){ (void)hot_find_level(B, N, 123.4f); }, batches, iters, samples);

#endif
  } else {
    std::cerr << "unknown kernel\n"; return 1;
  }

  std::ofstream ofs(out);
  ofs << "ns\n";
  for (auto v: samples) ofs << v << "\n";
  ofs.close();

  // Print quick stats to std out
  auto mean = double(std::accumulate(samples.begin(), samples.end(), 0ull))/samples.size();
  std::cerr << "samples=" << samples.size()<<" mean_ns=" << mean << "\n";
  return 0;
}
