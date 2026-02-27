#include <benchmark/benchmark.h>
#include <geometry/geometry.hpp>

using namespace iso::literals;
using namespace linalg;
using namespace linalg::operators;
using namespace geometry;

static void BM_DotProduct(benchmark::State& state) {
    geometry::R4::vector a{{1.0, 2.0, 3.0, 1.0}};
    geometry::R4::vector b{{4.0, 5.0, 6.0, 1.0}};
    for (auto _ : state) {
        volatile auto result = dot(a, b);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_DotProduct);

static void BM_CrossProduct(benchmark::State& state) {
    geometry::R3::vector a{{1.0, 2.0, 3.0}};
    geometry::R3::vector b{{4.0, 5.0, 6.0}};
    for (auto _ : state) {
        volatile auto result = R3::cross(a, b);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_CrossProduct);

// triple product benchmark
static void BM_TripleProduct(benchmark::State& state) {
    geometry::R3::vector u{{1, 2, 3}};
    geometry::R3::vector v{{4, 5, 6}};
    geometry::R3::vector w{{7, 8, 9}};
    for (auto _ : state) {
        volatile auto result = geometry::R3::triple(u, v, w);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_TripleProduct);

static void BM_ClosestPointsFromSkewLines(benchmark::State& state) {
    geometry::R3::line L1{{1, 0, 0, 90, -1, 0}};
    geometry::R3::line L2{{0, 0, 1, 1, 1, 70}};
    for (auto _ : state) {
        volatile auto result = closest_points_from_skew_lines(L1, L2);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_ClosestPointsFromSkewLines);

static void BM_RotationMatrix(benchmark::State& state) {
    iso::radians const theta(iso::pi / 2);
    for (auto _ : state) {
        volatile auto result = rotation(geometry::R3::basis::Z, theta);
        // benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_RotationMatrix);

static void BM_RotateVector(benchmark::State& state) {
    iso::radians const theta(iso::pi / 2);
    matrix R = rotation(geometry::R3::basis::Z, theta);
    geometry::R3::vector v0{{1, 0, 0}};
    for (auto _ : state) {
        volatile auto result = R * v0;
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_RotateVector);

static void BM_LineSolve(benchmark::State& state) {
    geometry::R3::point A{1, 2, 3};
    geometry::R3::point B{4, 5, 6};
    geometry::R3::point C{9, 12, 0};
    geometry::R3::vector vA{{1, 1, 1}};

    geometry::R3::line L0{vA, A};
    precision t;
    for (auto _ : state) {
        volatile bool result = L0.solve(B, t);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_LineSolve);

// Rodrigues rotation formula benchmark
static void BM_RodriguesRotation(benchmark::State& state) {
    iso::radians const theta(iso::pi / 2);
    geometry::R3::vector k = -geometry::R3::basis::Z;
    geometry::R3::vector v0{{1, 0, 0}};
    precision cos_t = std::cos(theta.value);
    precision sin_t = std::sin(theta.value);
    for (auto _ : state) {
        volatile auto result = geometry::R3::rodrigues(k, v0, theta);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_RodriguesRotation);

// Spherical to Cartesian conversion benchmark
static void BM_SphericalToCartesian(benchmark::State& state) {
    geometry::R3::point spherical_point{2.0_p, iso::pi / 4, iso::pi / 4};  // r, theta, phi
    for (auto _ : state) {
        volatile auto result = geometry::spherical_to_cartesian(spherical_point);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_SphericalToCartesian);

static void BM_CartesianToSpherical(benchmark::State& state) {
    geometry::R3::point cartesian_point{1.0_p, 1.0_p, std::sqrt(2.0_p)};  // x, y, z
    for (auto _ : state) {
        volatile auto result = geometry::cartesian_to_spherical(cartesian_point);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_CartesianToSpherical);

// Intersection Tests

static void BM_PlanePlaneIntersection(benchmark::State& state) {
    geometry::plane P1{geometry::R3::basis::X, geometry::R3::origin};
    geometry::plane P2{geometry::R3::basis::Y, geometry::R3::origin};
    for (auto _ : state) {
        volatile auto result = intersects(P1, P2);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_PlanePlaneIntersection);

// Sphere Intersection
static void BM_SphereLineIntersection(benchmark::State& state) {
    geometry::R3::sphere S{5.0_p};
    geometry::R3::line L{{1, 1, 1, 10, 10, 10}};
    for (auto _ : state) {
        volatile auto result = intersects(S, L);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_SphereLineIntersection);

// Plane Perpendicular Foot
static void BM_PlanePerpendicular(benchmark::State& state) {
    geometry::plane P{geometry::R3::basis::Z, geometry::R3::origin};
    geometry::plane Q{geometry::R3::basis::Y, geometry::R3::point{1, 1, 1}};
    for (auto _ : state) {
        volatile auto result = P.perpendicular(Q);
        // benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_PlanePerpendicular);

BENCHMARK_MAIN();