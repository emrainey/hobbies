#include <benchmark/benchmark.h>
#include <linalg/linalg.hpp>

using namespace iso::literals;
using namespace linalg;
using namespace linalg::operators;

static void BM_MatrixMultiplication3x3(benchmark::State& state) {
    matrix A{{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}}};
    matrix B{{{9.0, 8.0, 7.0}, {6.0, 5.0, 4.0}, {3.0, 2.0, 1.0}}};
    for (auto _ : state) {
        volatile auto C = A * B;
        // benchmark::DoNotOptimize(C);
    }
}
BENCHMARK(BM_MatrixMultiplication3x3);

// 4x4
static void BM_MatrixMultiplication4x4(benchmark::State& state) {
    matrix A{{{1.0, 2.0, 3.0, 4.0}, {5.0, 6.0, 7.0, 8.0}, {9.0, 0.0, 1.0, 2.0}, {3.0, 4.0, 5.0, 6.0}}};
    matrix B{{{6.0, 5.0, 4.0, 3.0}, {2.0, 1.0, 0.0, 9.0}, {8.0, 7.0, 6.0, 5.0}, {4.0, 3.0, 2.0, 1.0}}};
    for (auto _ : state) {
        volatile auto C = A * B;
        // benchmark::DoNotOptimize(C);
    }
}
BENCHMARK(BM_MatrixMultiplication4x4);

// 5x5
static void BM_MatrixMultiplication5x5(benchmark::State& state) {
    matrix A{{{1.0, 2.0, 3.0, 4.0, 5.0}, {6.0, 7.0, 8.0, 9.0, 0.0}, {1.0, 2.0, 3.0, 4.0, 5.0}, {6.0, 7.0, 8.0, 9.0, 0.0}, {1.0, 2.0, 3.0, 4.0, 5.0}}};
    matrix B{{{5.0, 4.0, 3.0, 2.0, 1.0}, {0.0, 9.0, 8.0, 7.0, 6.0}, {5.0, 4.0, 3.0, 2.0, 1.0},  {0.0, 9.0, 8.0, 7.0, 6.0}, {5.0, 4.0, 3.0, 2.0, 1.0}}};
    for (auto _ : state) {
        volatile auto C = A * B;
        // benchmark::DoNotOptimize(C);
    }
}
BENCHMARK(BM_MatrixMultiplication5x5);

// Much bigger Multiplication (100x100)
static void BM_MatrixMultiplication100x100(benchmark::State& state) {
    matrix A = matrix::random(100, 100, 1.0_p, 10.0_p);
    matrix B = matrix::random(100, 100, 1.0_p, 10.0_p);
    for (auto _ : state) {
        volatile auto C = A * B;
        // benchmark::DoNotOptimize(C);
    }
}
BENCHMARK(BM_MatrixMultiplication100x100);

// 4x4 Inverse
static void BM_MatrixInverse4x4(benchmark::State& state) {
    matrix A{{{4.0, 7.0, 2.0, 3.0}, {0.0, 5.0, 0.0, 4.0}, {1.0, 0.0, 3.0, 2.0}, {0.0, 0.0, 1.0, 1.0}}};
    for (auto _ : state) {
        volatile auto Ainv = A.inverse();
        // benchmark::DoNotOptimize(Ainv);
    }
}
BENCHMARK(BM_MatrixInverse4x4);

// 4x4 Transpose
static void BM_MatrixTranspose4x4(benchmark::State& state) {
    matrix A{{{4.0, 7.0, 2.0, 3.0}, {0.0, 5.0, 0.0, 4.0}, {1.0, 0.0, 3.0, 2.0}, {0.0, 0.0, 1.0, 1.0}}};
    for (auto _ : state) {
        volatile auto At = A.T();
        // benchmark::DoNotOptimize(At);
    }
}
BENCHMARK(BM_MatrixTranspose4x4);

// Quadratic Equation Solver
static void BM_QuadraticEquationSolver(benchmark::State& state) {
    precision a = 1.0_p;
    precision b = -3.0_p;
    precision c = 2.0_p;
    for (auto _ : state) {
        volatile auto result = linalg::quadratic_roots(a, b, c);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_QuadraticEquationSolver);

// Cubic Equation Solver
static void BM_CubicEquationSolver(benchmark::State& state) {
    precision a = 1.0_p;
    precision b = -6.0_p;
    precision c = 11.0_p;
    precision d = -6.0_p;
    for (auto _ : state) {
        volatile auto result = linalg::cubic_roots(a, b, c, d);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_CubicEquationSolver);

// Quartic Equation Solver
static void BM_QuarticEquationSolver(benchmark::State& state) {
    precision a = 1.0_p;
    precision b = -10.0_p;
    precision c = 35.0_p;
    precision d = -50.0_p;
    precision e = 24.0_p;
    for (auto _ : state) {
        volatile auto result = linalg::quartic_roots(a, b, c, d, e);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_QuarticEquationSolver);


BENCHMARK_MAIN();