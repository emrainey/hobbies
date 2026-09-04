#include "basal/gtest_helper.hpp"

#include <basal/basal.hpp>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

TEST(PyramidTest, Type) {
    using namespace raytrace;
    using namespace raytrace::objects;

    pyramid p0{R3::origin, 1.0_p};
    ASSERT_EQ(p0.get_type(), Type::Pyramid);
    ASSERT_EQ(p0.max_collisions(), 2U);
    ASSERT_FALSE(p0.has_definite_volume());
}

TEST(PyramidTest, FaceIntersections) {
    using namespace raytrace;
    using namespace raytrace::objects;

    // Pyramid with base centered at the origin, apex at (0, 0, h).
    // Faces: F1 (+x,+y) F2 (-x,+y) F3 (+x,-y) F4 (-x,-y)
    pyramid p0{R3::origin, 1.0_p};

    {
        // F1: x + y + z = 1, x > 0, y > 0
        raytrace::point o{1.25_p, 0.25_p, 0.5_p};
        ray r{o, -R3::basis::X};
        raytrace::point i{0.25_p, 0.25_p, 0.5_p};
        raytrace::objects::object::hit h = p0.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        ASSERT_POINT_EQ(i, as_point(h.intersect));
        ASSERT_PRECISION_EQ(1.0_p, h.distance);
        ASSERT_EQ(&p0, h.object);
        ASSERT_TRUE(p0.is_surface_point(as_point(h.intersect)));
    }
    {
        // F2: -x + y + z = 1, x < 0, y > 0
        raytrace::point o{-1.25_p, 0.25_p, 0.5_p};
        ray r{o, R3::basis::X};
        raytrace::point i{-0.25_p, 0.25_p, 0.5_p};
        raytrace::objects::object::hit h = p0.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        ASSERT_POINT_EQ(i, as_point(h.intersect));
        ASSERT_PRECISION_EQ(1.0_p, h.distance);
        ASSERT_EQ(&p0, h.object);
        ASSERT_TRUE(p0.is_surface_point(as_point(h.intersect)));
    }
    {
        // F3: x - y + z = 1, x > 0, y < 0
        raytrace::point o{0.25_p, -1.25_p, 0.5_p};
        ray r{o, R3::basis::Y};
        raytrace::point i{0.25_p, -0.25_p, 0.5_p};
        raytrace::objects::object::hit h = p0.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        ASSERT_POINT_EQ(i, as_point(h.intersect));
        ASSERT_PRECISION_EQ(1.0_p, h.distance);
        ASSERT_EQ(&p0, h.object);
        ASSERT_TRUE(p0.is_surface_point(as_point(h.intersect)));
    }
    {
        // F4: -x - y + z = 1, x < 0, y < 0
        raytrace::point o{-0.25_p, -1.25_p, 0.5_p};
        ray r{o, R3::basis::Y};
        raytrace::point i{-0.25_p, -0.25_p, 0.5_p};
        raytrace::objects::object::hit h = p0.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        ASSERT_POINT_EQ(i, as_point(h.intersect));
        ASSERT_PRECISION_EQ(1.0_p, h.distance);
        ASSERT_EQ(&p0, h.object);
        ASSERT_TRUE(p0.is_surface_point(as_point(h.intersect)));
    }
}

TEST(PyramidTest, ApexApproach) {
    using namespace raytrace;
    using namespace raytrace::objects;

    pyramid p0{R3::origin, 1.0_p};
    // Descend vertically onto F1 near the apex, from outside.
    raytrace::point o{0.5_p, 0.25_p, 2.0_p};
    ray r{o, -R3::basis::Z};
    raytrace::point i{0.5_p, 0.25_p, 0.25_p};
    raytrace::objects::object::hit h = p0.intersect(r);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
    ASSERT_POINT_EQ(i, as_point(h.intersect));
    ASSERT_PRECISION_EQ(1.75_p, h.distance);
    ASSERT_TRUE(p0.is_surface_point(as_point(h.intersect)));
}

TEST(PyramidTest, ThroughIntersections) {
    using namespace raytrace;
    using namespace raytrace::objects;

    pyramid p0{R3::origin, 1.0_p};
    // Passes from +x to -x through the middle of the pyramid (along the F3/F4 edge at y = 0).
    raytrace::point o{1.5_p, 0.0_p, 0.5_p};
    ray r{o, -R3::basis::X};

    raytrace::objects::hits ts = p0.collisions_along(r);
    ASSERT_EQ(2U, ts.size());
    precision dists[2] = {0.0_p, 0.0_p};
    raytrace::point pts[2] = {raytrace::point{}, raytrace::point{}};
    for (size_t i = 0; i < ts.size(); i++) {
        dists[i] = ts[i].distance;
        pts[i] = as_point(ts[i].intersect);
    }
    std::sort(dists, dists + 2, [](precision a, precision b) { return a < b; });
    ASSERT_PRECISION_EQ(1.0_p, dists[0]);
    ASSERT_PRECISION_EQ(2.0_p, dists[1]);
    // both hits must be valid surface points
    for (size_t i = 0; i < ts.size(); i++) {
        ASSERT_TRUE(p0.is_surface_point(pts[i]));
    }
    // the closest is the first
    raytrace::objects::object::hit h = p0.intersect(r);
    raytrace::point expect{0.5_p, 0.0_p, 0.5_p};
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
    ASSERT_POINT_EQ(expect, as_point(h.intersect));
    ASSERT_PRECISION_EQ(1.0_p, h.distance);
}

TEST(PyramidTest, Normals) {
    using namespace raytrace;
    using namespace raytrace::objects;

    pyramid p0{R3::origin, 1.0_p};

    precision s = basal::inv_sqrt_3;
    raytrace::point p1{0.25_p, 0.25_p, 0.5_p};
    raytrace::point p2{-0.25_p, 0.25_p, 0.5_p};
    raytrace::point p3{0.25_p, -0.25_p, 0.5_p};
    raytrace::point p4{-0.25_p, -0.25_p, 0.5_p};

    vector n1 = p0.normal(p1);
    vector n2 = p0.normal(p2);
    vector n3 = p0.normal(p3);
    vector n4 = p0.normal(p4);

    ASSERT_NEAR(+s, n1.x(), basal::epsilon);
    ASSERT_NEAR(+s, n1.y(), basal::epsilon);
    ASSERT_NEAR(+s, n1.z(), basal::epsilon);

    ASSERT_NEAR(-s, n2.x(), basal::epsilon);
    ASSERT_NEAR(+s, n2.y(), basal::epsilon);
    ASSERT_NEAR(+s, n2.z(), basal::epsilon);

    ASSERT_NEAR(+s, n3.x(), basal::epsilon);
    ASSERT_NEAR(-s, n3.y(), basal::epsilon);
    ASSERT_NEAR(+s, n3.z(), basal::epsilon);

    ASSERT_NEAR(-s, n4.x(), basal::epsilon);
    ASSERT_NEAR(-s, n4.y(), basal::epsilon);
    ASSERT_NEAR(+s, n4.z(), basal::epsilon);

    // each normal must be unit length
    for (vector const& n : {n1, n2, n3, n4}) {
        ASSERT_NEAR(1.0_p, n.magnitude(), basal::epsilon);
    }
}

TEST(PyramidTest, SurfacePoints) {
    using namespace raytrace;
    using namespace raytrace::objects;

    pyramid p0{R3::origin, 1.0_p};

    // on the surface
    ASSERT_TRUE(p0.is_surface_point(raytrace::point{0.0_p, 0.0_p, 1.0_p}));      // apex
    ASSERT_TRUE(p0.is_surface_point(raytrace::point{0.5_p, 0.5_p, 0.0_p}));      // base rim
    ASSERT_TRUE(p0.is_surface_point(raytrace::point{-0.25_p, 0.25_p, 0.5_p}));   // F2
    ASSERT_TRUE(p0.is_surface_point(raytrace::point{-0.25_p, -0.25_p, 0.5_p}));  // F4

    // NOT on the surface
    ASSERT_FALSE(p0.is_surface_point(raytrace::point{0.0_p, 0.0_p, 0.0_p}));  // base interior
    ASSERT_FALSE(p0.is_surface_point(raytrace::point{0.1_p, 0.1_p, 0.5_p}));  // interior
    ASSERT_FALSE(p0.is_surface_point(raytrace::point{0.0_p, 0.0_p, 2.0_p}));  // above apex
    // The side planes extend below the base (z < 0). Points on that extension are NOT
    // part of the actual pyramid surface and must be rejected.
    ASSERT_FALSE(p0.is_surface_point(raytrace::point{1.0_p, 1.0_p, -1.0_p}));  // below base extension
}

TEST(PyramidTest, BelowBaseMiss) {
    using namespace raytrace;
    using namespace raytrace::objects;

    pyramid p0{R3::origin, 1.0_p};
    // A ray that passes beside the pyramid, below the base plane (z < 0).
    // The F1 plane x + y + z = 1 happens to be met at (1, 1, -1), but that point is
    // below the base and is NOT part of the pyramid, so this must miss.
    raytrace::vector d{{-1.0_p, -1.0_p, -1.0_p}};
    raytrace::point o{2.0_p, 2.0_p, 0.0_p};
    ray r{o, d.normalized()};
    raytrace::objects::object::hit h = p0.intersect(r);
    ASSERT_EQ(geometry::IntersectionType::None, get_type(h.intersect));
}

TEST(PyramidTest, OffAxis) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point P{9.0_p, -14.0_p, 77.0_p};
    precision h = 3.0_p;
    pyramid p0{P, h};

    // world space equivalent of an F3 hit
    raytrace::point o{14.0_p, -14.0_p, 78.5_p};
    ray r{o, -R3::basis::X};
    raytrace::point i{10.5_p, -14.0_p, 78.5_p};
    raytrace::objects::object::hit h0 = p0.intersect(r);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(h0.intersect));
    ASSERT_POINT_EQ(i, as_point(h0.intersect));
    ASSERT_PRECISION_EQ(2.0_p * h - 2.5_p, h0.distance);
    ASSERT_TRUE(p0.is_surface_point(as_point(h0.intersect)));

    // the world-space normal should match the object-space F3 normal (no rotation)
    vector n = p0.normal(i);
    ASSERT_NEAR(+basal::inv_sqrt_3, n.x(), basal::epsilon);
    ASSERT_NEAR(-basal::inv_sqrt_3, n.y(), basal::epsilon);
    ASSERT_NEAR(+basal::inv_sqrt_3, n.z(), basal::epsilon);
}

TEST(PyramidTest, Rotated) {
    using namespace raytrace;
    using namespace raytrace::objects;

    pyramid p0{R3::origin, 1.0_p};
    p0.rotation(iso::degrees{0}, iso::degrees{0}, iso::degrees{90});

    // object point (0.25, 0.25, 0.5) maps to world (-0.25, 0.25, 0.5) under yaw 90
    raytrace::point world{-0.25_p, 0.25_p, 0.5_p};
    ASSERT_TRUE(p0.is_surface_point(world));
    // and the world normal is the object normal rotated by the same yaw
    vector n = p0.normal(world);
    ASSERT_NEAR(-basal::inv_sqrt_3, n.x(), basal::epsilon);
    ASSERT_NEAR(+basal::inv_sqrt_3, n.y(), basal::epsilon);
    ASSERT_NEAR(+basal::inv_sqrt_3, n.z(), basal::epsilon);
}

TEST(PyramidTest, ExtentAndBounds) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point P{9.0_p, -14.0_p, 77.0_p};
    precision h = 3.0_p;
    pyramid p0{P, h};

    // apex and all base corners are at distance h from the base center,
    // so a sphere of radius h encloses the whole pyramid.
    ASSERT_PRECISION_EQ(h, p0.get_object_extent());
    Bounds b = p0.get_world_bounds();
    raytrace::point mn{6.0_p, -17.0_p, 74.0_p};
    raytrace::point mx{12.0_p, -11.0_p, 80.0_p};
    ASSERT_POINT_EQ(mn, b.min);
    ASSERT_POINT_EQ(mx, b.max);
}