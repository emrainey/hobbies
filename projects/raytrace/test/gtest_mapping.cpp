
#include <gtest/gtest.h>

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
        ASSERT_FLOAT_EQ(1.0, D) << i << "/" << count << " Point is " << points[i];
    }
    // FIXME add more assumptions here about the spread of the golden ratio points across the sphere.
}