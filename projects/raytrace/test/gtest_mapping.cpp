
#include <gtest/gtest.h>

#include <geometry/gtest_helper.hpp>
#include <raytrace/raytrace.hpp>

using namespace geometry;
using namespace raytrace;
using namespace raytrace::colors;
using namespace raytrace::operators;

TEST(MappingTest, GoldenRatio) {
    size_t const count = 100;
    std::vector<geometry::R3::point> points(count);
    for (size_t i = 0; i < count; i++) {
        points[i] = raytrace::mapping::golden_ratio_mapper(i, count);
        // compute the distance to the origin and it should always be ~1.0
        geometry::R3::vector R = points[i] - geometry::R3::origin;
        precision D = R.magnitude();
        ASSERT_FLOAT_EQ(1.0_p, D) << i << "/" << count << " Point is " << points[i];
    }
}

TEST(MappingTest, PlanarPolar) {
    {
        raytrace::vector N{0, 0, 1};
        raytrace::vector X{1, 0, 0};
        raytrace::point C{0, 0, 0};
        geometry::R3::point P{1, 1, 1};
        geometry::R2::point polar = raytrace::mapping::planar_polar(N, X, C, P);
        ASSERT_PRECISION_EQ(sqrt(2.0_p), polar.x);
        ASSERT_PRECISION_EQ(1.0_p / 8.0_p, polar.y);
    }
    {
        raytrace::vector N{0, 0, 1};
        raytrace::vector X{1, 0, 0};
        raytrace::point C{0, 0, 0};
        geometry::R3::point P{-1, 1, 1};
        geometry::R2::point polar = raytrace::mapping::planar_polar(N, X, C, P);
        ASSERT_PRECISION_EQ(sqrt(2.0_p), polar.x);
        ASSERT_PRECISION_EQ(3.0_p / 8.0_p, polar.y);
    }
    {
        raytrace::vector N{0, 0, 1};
        raytrace::vector X{1, 0, 0};
        raytrace::point C{0, 0, 0};
        geometry::R3::point P{-1, -1, 1};
        geometry::R2::point polar = raytrace::mapping::planar_polar(N, X, C, P);
        ASSERT_PRECISION_EQ(sqrt(2.0_p), polar.x);
        ASSERT_PRECISION_EQ(5.0_p / 8.0_p, polar.y);
    }
    {
        raytrace::vector N{0, 0, 1};
        raytrace::vector X{1, 0, 0};
        raytrace::point C{0, 0, 0};
        geometry::R3::point P{1, -1, 1};
        geometry::R2::point polar = raytrace::mapping::planar_polar(N, X, C, P);
        ASSERT_PRECISION_EQ(sqrt(2.0_p), polar.x);
        ASSERT_PRECISION_EQ(7.0_p / 8.0_p, polar.y);
    }
}