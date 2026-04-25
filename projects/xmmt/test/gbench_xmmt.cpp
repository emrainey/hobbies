#include <benchmark/benchmark.h>

#include "xmmt/xmat.hpp"
#include "xmmt/xmmt.hpp"

using namespace basal::literals;

namespace {

using point2 = xmmt::point_<xmmt::double2, 2>;
using vector2 = xmmt::vector_<xmmt::double2, 2>;
using pixel4 = xmmt::pixel_<xmmt::double4, 4>;
using xmat4 = xmmt::xmat_<4, 4>;

static void BM_PointTranslate(benchmark::State& state) {
    point2 p{1.0_p, 2.0_p};
    vector2 v{0.25_p, -0.5_p};
    for (auto _ : state) {
        p += v;
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(BM_PointTranslate);

static void BM_PointScale(benchmark::State& state) {
    point2 p{1.0_p, 2.0_p};
    constexpr auto s = 1.125_p;
    for (auto _ : state) {
        auto q = p * s;
        benchmark::DoNotOptimize(q);
    }
}
BENCHMARK(BM_PointScale);

static void BM_XMatAdd(benchmark::State& state) {
    xmat4 A{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    xmat4 B{16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    for (auto _ : state) {
        auto C = A;
        C += B;
        benchmark::DoNotOptimize(C);
    }
}
BENCHMARK(BM_XMatAdd);

static void BM_XMatScale(benchmark::State& state) {
    xmat4 A{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    constexpr double s = 0.5;
    for (auto _ : state) {
        auto B = A * s;
        benchmark::DoNotOptimize(B);
    }
}
BENCHMARK(BM_XMatScale);

static void BM_PixelHadamard(benchmark::State& state) {
    pixel4 a{0.1_p, 0.2_p, 0.3_p, 0.4_p};
    pixel4 b{0.9_p, 0.8_p, 0.7_p, 0.6_p};
    for (auto _ : state) {
        auto c = a * b;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_PixelHadamard);

static void BM_PixelClamp(benchmark::State& state) {
    pixel4 p{1.5_p, -0.2_p, 0.75_p, 2.0_p};
    for (auto _ : state) {
        auto q = p;
        q.clamp();
        benchmark::DoNotOptimize(q);
    }
}
BENCHMARK(BM_PixelClamp);

}  // namespace

BENCHMARK_MAIN();
