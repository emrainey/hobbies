#include "benchmark/benchmark.h"
#include <raytrace/raytrace.hpp>

using namespace iso::literals;
using namespace raytrace;
using namespace raytrace::operators;

// Plane Intersection Benchmark
static void BM_PlaneIntersections(benchmark::State& state) {
    raytrace::ray r{raytrace::point{0, 0, 2}, -R3::basis::Z};
    raytrace::objects::plane obj;
    for (auto _ : state) {
        volatile auto hit = obj.intersect(r).intersect;
        benchmark::DoNotOptimize(hit);
    }
}
BENCHMARK(BM_PlaneIntersections);

// Cuboid Intersection Benchmark
static void BM_CuboidIntersections(benchmark::State& state) {
    raytrace::ray r{raytrace::point{0, 0, 2}, -R3::basis::Z};
    raytrace::objects::cuboid obj(R3::origin, 1.0_p, 1.0_p, 1.0_p);
    for (auto _ : state) {
        volatile auto hit = obj.intersect(r).intersect;
        benchmark::DoNotOptimize(hit);
    }
}
BENCHMARK(BM_CuboidIntersections);

// Cylinder Intersection Benchmark
static void BM_CylinderIntersections(benchmark::State& state) {
    raytrace::ray r{raytrace::point{2.0_p, 0, 0}, -R3::basis::X};
    raytrace::objects::cylinder obj(R3::origin, 1.0_p, 1.0_p);
    for (auto _ : state) {
        volatile auto hit = obj.intersect(r).intersect;
        benchmark::DoNotOptimize(hit);
    }
}
BENCHMARK(BM_CylinderIntersections);

BENCHMARK_MAIN();