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
        hit h = shape.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{1, 0, 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{1, 0, 0};
        ASSERT_VECTOR_EQ(N, h.normal);
    }
    {
        raytrace::ray r{raytrace::point{-3, 0, 0}, R3::basis::X};
        hit h = shape.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{-1, 0, 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{-1, 0, 0};
        ASSERT_VECTOR_EQ(N, h.normal);
    }
}

TEST(OverlapTest, RayIntersectionsSubtractive) {
    using namespace raytrace;
    using namespace raytrace::objects;
    // s0 is the center, s1 is the subtractive object
    objects::sphere s0{R3::point(-1, 0, 0), 2};
    objects::sphere s1{R3::point(1, 0, 0), 2};
    // subtractive overlap which makes a concave sphere with the bit facing +X
    overlap shape(s0, s1, overlap::type::subtractive);
    shape.position(R3::point{0, 0, 0});
    // check for collisions at the sphere edges and in the internal cavity
    {
        raytrace::ray r{raytrace::point{4, 0, 0}, -R3::basis::X};
        hit h = shape.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{-1, 0, 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{1, 0, 0};
        ASSERT_VECTOR_EQ(N, h.normal); // should reflect backwards, not into the sphere
    }
    {
        raytrace::ray r{raytrace::point{-4, 0, 0}, R3::basis::X};
        hit h = shape.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{-3, 0, 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{-1, 0, 0};
        ASSERT_VECTOR_EQ(N, h.normal); // should reflect backwards
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
        hit h = shape.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{3, 0, 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{1, 0, 0};
        ASSERT_VECTOR_EQ(N, h.normal);
    }
    {
        raytrace::ray r{raytrace::point{-4, 0, 0}, R3::basis::X};
        hit h = shape.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{-3, 0, 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{-1, 0, 0};
        ASSERT_VECTOR_EQ(N, h.normal);
    }
}

TEST(OverlapTest, RayIntersectionsExclusive) {
    using namespace raytrace;
    using namespace raytrace::objects;
    objects::sphere s0{R3::point(-1, 0, 0), 2};
    objects::sphere s1{R3::point(1, 0, 0), 2};
    overlap shape(s0, s1, overlap::type::exclusive);  // exclusive overlap which makes a reverse or inverse "lens" like shape
    // check for collisions at the lens edges which are INTERNAL to the structure
    {
        raytrace::ray r{raytrace::point{0, 0, 0}, -R3::basis::X};
        hit h = shape.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{-1, 0, 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{1, 0, 0};
        ASSERT_VECTOR_EQ(N, h.normal);
    }
    {
        raytrace::ray r{raytrace::point{0, 0, 0}, R3::basis::X};
        hit h = shape.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{1, 0, 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{-1, 0, 0};
        ASSERT_VECTOR_EQ(N, h.normal);
    }
    // now check for collisions at the sphere edges
    {
        raytrace::ray r{raytrace::point{4, 0, 0}, -R3::basis::X};
        hit h = shape.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{3, 0, 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{1, 0, 0};
        ASSERT_VECTOR_EQ(N, h.normal);
    }
    {
        raytrace::ray r{raytrace::point{-4, 0, 0}, R3::basis::X};
        hit h = shape.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{-3, 0, 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{-1, 0, 0};
        ASSERT_VECTOR_EQ(N, h.normal);
    }
}


TEST(OverlapTest, TripleSphereOverlapInclusive) {
    using namespace raytrace;
    using namespace raytrace::objects;
    // overlaps need to centered on zero to get the ray directions correct
    objects::sphere s0{R3::point(-1, 0, 0), 2};
    objects::sphere s1{R3::point(1, 0, 0), 2};
    objects::sphere s2{R3::point(0, 0, 0), sqrt(2.0_p)};
    overlap shape1(s0, s1, overlap::type::inclusive);
    overlap shape2(shape1, s2, overlap::type::inclusive);

    // check for collisions at the combined shape edges
    {
        raytrace::ray r{raytrace::point{3, 0, 0}, -R3::basis::X};
        hit h = shape2.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{1, 0, 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{1, 0, 0};
        ASSERT_VECTOR_EQ(N, h.normal);
    }
    {
        raytrace::ray r{raytrace::point{-3, 0, 0}, R3::basis::X};
        hit h = shape2.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{-1, 0, 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{-1, 0, 0};
        ASSERT_VECTOR_EQ(N, h.normal);
    }
    {
        raytrace::ray r{raytrace::point{0, 3, 0}, -R3::basis::Y};
        hit h = shape2.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{0, sqrt(2.0_p), 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{0, 1, 0};
        ASSERT_VECTOR_EQ(N, h.normal);
    }
}

TEST(OverlapTest, TripleSphereOverlapSubtractive) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point center{-1, 0, 0};
    // subtractive overlaps are centered on the first object
    objects::sphere s0{center, 2};
    objects::sphere s1{R3::point(1, 0, 0), 2};
    objects::sphere s2{R3::point(0, 0, 0), sqrt(2.0_p)};
    overlap shape1(s0, s1, overlap::type::subtractive);
    shape1.position(R3::origin); // the position of the overlap to be centered so that the next subtraction will work
    overlap shape2(shape1, s2, overlap::type::subtractive);

    // check for collisions at the combined shape edges
    {
        raytrace::ray r{raytrace::point{4, 0, 0}, -R3::basis::X};
        hit h = shape2.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{-sqrt(2.0_p), 0, 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{1, 0, 0};
        ASSERT_VECTOR_EQ(N, h.normal);
    }
    {
        raytrace::ray r{raytrace::point{-4, 0, 0}, R3::basis::X};
        hit h = shape2.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{-3, 0, 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{-1, 0, 0};
        ASSERT_VECTOR_EQ(N, h.normal);
    }
    {
        raytrace::ray r{raytrace::point{-1, 4, 0}, -R3::basis::Y};
        hit h = shape2.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{-1, 2, 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{0, 1, 0};
        ASSERT_VECTOR_EQ(N, h.normal);
    }
}

TEST(OverlapTest, TripleSphereOverlapAdditive) {
    using namespace raytrace;
    using namespace raytrace::objects;

    objects::sphere s0{R3::point(-1, 0, 0), 2};
    objects::sphere s1{R3::point(1, 0, 0), 2};
    objects::sphere s2{R3::point(0, 0, 0), sqrt(5.0_p)};
    overlap shape1(s0, s1, overlap::type::additive);
    overlap shape2(shape1, s2, overlap::type::additive);

    // check for collisions at the combined shape edges
    {
        raytrace::ray r{raytrace::point{4, 0, 0}, -R3::basis::X};
        hit h = shape2.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{3, 0, 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{1, 0, 0};
        ASSERT_VECTOR_EQ(N, h.normal);
    }
    {
        raytrace::ray r{raytrace::point{-4, 0, 0}, R3::basis::X};
        hit h = shape2.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{-3, 0, 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{-1, 0, 0};
        ASSERT_VECTOR_EQ(N, h.normal);
    }
    {
        raytrace::ray r{raytrace::point{0, 4, 0}, -R3::basis::Y};
        hit h = shape2.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{0, sqrt(5.0_p), 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{0, 1, 0};
        ASSERT_VECTOR_EQ(N, h.normal);
    }
}

TEST(OverlapTest, TripleSphereOverlapExclusive) {
    using namespace raytrace;
    using namespace raytrace::objects;

    objects::sphere s0{R3::point(-1, 0, 0), 2};
    objects::sphere s1{R3::point(1, 0, 0), 2};
    objects::sphere s2{R3::point(0, 0, 0), sqrt(2.0_p)};
    overlap shape1(s0, s1, overlap::type::inclusive);
    overlap shape2(shape1, s2, overlap::type::exclusive);
    // check for collisions at the lens edges which are INTERNAL to the structure
    {
        raytrace::ray r{raytrace::point{0, 0, 0}, -R3::basis::X};
        hit h = shape2.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{-1, 0, 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{1, 0, 0};
        ASSERT_VECTOR_EQ(N, h.normal);
    }
    {
        raytrace::ray r{raytrace::point{0, 0, 0}, R3::basis::X};
        hit h = shape2.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{1, 0, 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{-1, 0, 0};
        ASSERT_VECTOR_EQ(N, h.normal);
    }
    // check for collisions at the combined shape edges
    {
        raytrace::ray r{raytrace::point{4, 0, 0}, -R3::basis::X};
        hit h = shape2.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{sqrt(2.0_p), 0, 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{1, 0, 0};
        ASSERT_VECTOR_EQ(N, h.normal);
    }
    {
        raytrace::ray r{raytrace::point{-4, 0, 0}, R3::basis::X};
        hit h = shape2.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{-sqrt(2.0_p), 0, 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{-1, 0, 0};
        ASSERT_VECTOR_EQ(N, h.normal);
    }
    // lets test the special points like the edge
    {
        raytrace::ray r{raytrace::point{0, 4, 0}, -R3::basis::Y};
        hit h = shape2.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{0, sqrt(3.0_p), 0};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        // raytrace::vector N{0, 1, 0};
        // ASSERT_VECTOR_EQ(N, h.normal);
    }
}

TEST(OverlapTest, CuboidSphereSubtractiveAndSphereInclusive) {
    using namespace raytrace;
    using namespace raytrace::objects;


    objects::cuboid box{R3::origin, 8, 8, 2};
    objects::sphere cutout{R3::point(0, 0, 4), 4};
    overlap block(box, cutout, overlap::type::subtractive);
    objects::sphere rounder{R3::point(0, 0, -15), 20};
    overlap shape(block, rounder, overlap::type::inclusive);
    shape.position(raytrace::point{0, 0, 2});
    {
        raytrace::ray r{raytrace::point{0, 0, 11}, -R3::basis::Z};
        hit h = shape.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        raytrace::point A{0, 0, 2};
        ASSERT_POINT_EQ(A, as_point(h.intersect));
        raytrace::vector N{0, 0, 1};
        ASSERT_VECTOR_EQ(N, h.normal);
    }
}