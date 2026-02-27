#include "benchmark/benchmark.h"
#include <linalg/dkf.hpp>
#include <linalg/kmeans.hpp>
#include <linalg/softmax.hpp>

using namespace iso::literals;
using namespace linalg;
using namespace linalg::operators;

static void BM_KMeansIteration(benchmark::State& state) {
    size_t num_points = 1000;
    size_t num_clusters = 5;
    std::vector<geometry::R2::point> points;
    for (size_t i = 0; i < num_points; i++) {
        points.push_back(geometry::R2::point{static_cast<precision>(rand() % 1000), static_cast<precision>(rand() % 1000)});
    }
    linalg::kmeans kmeans_instance(points);
    kmeans_instance.configure(num_clusters);
    kmeans_instance.initial(linalg::kmeans::InitialMethod::RandomPoints);
    for (auto _ : state) {
        volatile auto error = kmeans_instance.iteration(linalg::kmeans::IterationMethod::EuclideanDistance);
        benchmark::DoNotOptimize(error);
    }
}
BENCHMARK(BM_KMeansIteration);

// DKF Cycle Benchmark
static void BM_DKF_Cycle(benchmark::State& state) {
}
BENCHMARK(BM_DKF_Cycle);

// Softmax Benchmark
static void BM_Softmax(benchmark::State& state) {
    linalg::matrix m(3, 1);
    m[0][0] = 1.0_p;
    m[1][0] = 2.0_p;
    m[2][0] = 3.0_p;

    for (auto _ : state) {
        volatile auto result = linalg::softmax(m);
        // benchmark::DoNotOptimize(result.cols);
    }
}
BENCHMARK(BM_Softmax);

BENCHMARK_MAIN();