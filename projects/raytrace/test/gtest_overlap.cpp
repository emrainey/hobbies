#include "basal/gtest_helper.hpp"

#include <basal/basal.hpp>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

TEST(OverlapTest, RayIntersectionsInclusive) {

    using namespace raytrace;
    using namespace raytrace::objects;

    objects::sphere s0{R3::point(-1, 0, 0), 2};
    objects::sphere s1{R3::point(1, 0, 0), 2};
    overlap shape(s1, s0, overlap::type::inclusive);  // inclusive overlap which makes a "lens" like shape

    // check for collisions at the lens edges
    {
        raytrace::ray r{raytrace::point{3, 0, 0}, -R3::basis::X};
        geometry::intersection is = shape.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(is));
        raytrace::point A{1, 0, 0};
        ASSERT_POINT_EQ(A, as_point(is));
    }
    {
        raytrace::ray r{raytrace::point{-3, 0, 0}, R3::basis::X};
        geometry::intersection is = shape.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(is));
        raytrace::point A{-1, 0, 0};
        ASSERT_POINT_EQ(A, as_point(is));
    }
}

TEST(OverlapTest, RayIntersectionsSubtractive) {
    using namespace raytrace;
    using namespace raytrace::objects;
    objects::sphere s0{R3::point(-1, 0, 0), 2};
    objects::sphere s1{R3::point(1, 0, 0), 2};
    overlap shape(s0, s1, overlap::type::subtractive);  // subtractive overlap which makes a concave sphere
    // check for collisions at the sphere edges and in the internal cavity
    {
        raytrace::ray r{raytrace::point{4, 0, 0}, -R3::basis::X};
        geometry::intersection is = shape.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(is));
        raytrace::point A{-1, 0, 0};
        ASSERT_POINT_EQ(A, as_point(is));
    }
    {
        raytrace::ray r{raytrace::point{-4, 0, 0}, R3::basis::X};
        geometry::intersection is = shape.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(is));
        raytrace::point A{-3, 0, 0};
        ASSERT_POINT_EQ(A, as_point(is));
    }
}

TEST(OverlapTest, RayIntersectionsAdditive) {
    using namespace raytrace;
    using namespace raytrace::objects;
    objects::sphere s0{R3::point(-1, 0, 0), 2};
    objects::sphere s1{R3::point(1, 0, 0), 2};
    overlap shape(s0, s1, overlap::type::additive);  // additive overlap which makes a "double" sphere
    // check for collisions at the sphere edges
    {
        raytrace::ray r{raytrace::point{4, 0, 0}, -R3::basis::X};
        geometry::intersection is = shape.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(is));
        raytrace::point A{3, 0, 0};
        ASSERT_POINT_EQ(A, as_point(is));
    }
    {
        raytrace::ray r{raytrace::point{-4, 0, 0}, R3::basis::X};
        geometry::intersection is = shape.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(is));
        raytrace::point A{-3, 0, 0};
        ASSERT_POINT_EQ(A, as_point(is));
    }
}

TEST(OverlapTest, DISABLED_RayIntersectionsExclusive) {
    using namespace raytrace;
    using namespace raytrace::objects;
    objects::sphere s0{R3::point(-1, 0, 0), 2};
    objects::sphere s1{R3::point(1, 0, 0), 2};
    overlap shape(s0, s1, overlap::type::exclusive);  // exclusive overlap which makes a reverse or inverse "lens" like shape
    // check for collisions at the lens edges which are INTERNAL to the structure
    {
        raytrace::ray r{raytrace::point{0, 0, 0}, -R3::basis::X};
        geometry::intersection is = shape.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(is));
        raytrace::point A{-1, 0, 0};
        ASSERT_POINT_EQ(A, as_point(is));
    }
    {
        raytrace::ray r{raytrace::point{0, 0, 0}, R3::basis::X};
        geometry::intersection is = shape.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(is));
        raytrace::point A{1, 0, 0};
    }
    // now check for collisions at the sphere edges
    {
        raytrace::ray r{raytrace::point{4, 0, 0}, -R3::basis::X};
        geometry::intersection is = shape.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(is));
        raytrace::point A{3, 0, 0};
        ASSERT_POINT_EQ(A, as_point(is));
    }
    {
        raytrace::ray r{raytrace::point{-4, 0, 0}, R3::basis::X};
        geometry::intersection is = shape.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(is));
        raytrace::point A{-3, 0, 0};
        ASSERT_POINT_EQ(A, as_point(is));
    }
}