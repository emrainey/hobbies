
#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

using namespace raytrace;
using namespace raytrace::objects;

TEST(TriangleTest, Basic) {
    raytrace::point A{0.0_p, 0.0_p, 0.0_p};
    raytrace::point B{-1.0_p, 1.0_p, 0.0_p};
    raytrace::point C{1.0_p, 1.0_p, 0.0_p};
    raytrace::objects::triangle shape{{A, B, C}};
    ASSERT_EQ(shape.get_type(), raytrace::objects::Type::Polygon);
    ASSERT_POINT_EQ(raytrace::point(0.0_p / 3.0_p, 2.0_p / 3.0_p, 0.0_p / 3.0_p), shape.position());
    ASSERT_TRUE(shape.is_contained(raytrace::point{0.0_p, 0.25_p, 0.0_p}))
        << "Point must be in _object space_, not world space";
    auto displacement = raytrace::point{1, 1, 0} - raytrace::point{0, 2.0_p / 3.0_p, 0};
    ASSERT_PRECISION_EQ(displacement.magnitude(), shape.get_object_extent());
    ASSERT_POINT_EQ(R3::basis::Z, shape.normal(raytrace::point{0.0_p, 0.25_p, 0.0_p}));
}

TEST(TriangleTest, PlaneParallelTriangle) {
    raytrace::point A{0.0_p, 0.0_p, 0.0_p};
    raytrace::point B{1.0_p, 1.0_p, 0.0_p};
    raytrace::point C{-1.0_p, 1.0_p, 0.0_p};
    raytrace::objects::triangle D{A, B, C};
    vector G = D.normal(R3::origin);

    using namespace linalg::operators;
    ASSERT_TRUE(G || R3::basis::Z);
}

TEST(TriangleTest, RayIntersection) {
    raytrace::point A{1, 0, 0};
    raytrace::point B{-1, -1, 0};
    raytrace::point C{0, 1, 0};
    raytrace::objects::triangle T{A, B, C};
    ray r0{raytrace::point{0.5_p, 0.5_p, 1}, vector{{0, 0, -1}}};

    geometry::intersection ir0T = T.intersect(r0).intersect;
    raytrace::point P{0.5_p, 0.5_p, 0};
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir0T));
    ASSERT_POINT_EQ(P, as_point(ir0T));

    ray r1{raytrace::point{0.7_p, 0.7_p, 1}, vector{{0, 0, -1}}};
    geometry::intersection ir1T = T.intersect(r1).intersect;
    ASSERT_EQ(geometry::IntersectionType::None, get_type(ir1T));

    ray r2{raytrace::point{0.5_p, 0.5_p, -1}, vector{{0, 0, 1}}};
    geometry::intersection ir2T = T.intersect(r2).intersect;
    ASSERT_EQ(geometry::IntersectionType::None, get_type(ir2T));

    ray r3{raytrace::point{0.7_p, 0.7_p, -1}, vector{{0, 0, 1}}};
    geometry::intersection ir3T = T.intersect(r3).intersect;
    ASSERT_EQ(geometry::IntersectionType::None, get_type(ir3T));
}

TEST(PolygonTest, Quadragon) {
    raytrace::point A{1.0_p, 1.0_p, 0.0_p};
    raytrace::point B{1.0_p, -1.0_p, 0.0_p};
    raytrace::point C{-1.0_p, -1.0_p, 0.0_p};
    raytrace::point D{-1.0_p, 1.0_p, 0.0_p};
    raytrace::objects::polygon<4> shape{{A, B, C, D}};
    ASSERT_EQ(shape.get_type(), raytrace::objects::Type::Polygon);
    ASSERT_POINT_EQ(R3::origin, shape.position());
    R3::point expect_in_point{0.25_p, 0.25_p, 0.0_p};
    ASSERT_TRUE(shape.is_contained(expect_in_point));
    auto displacement = raytrace::point{1.0_p, 1.0_p, 0.0_p} - R3::origin;
    ASSERT_PRECISION_EQ(displacement.magnitude(), shape.get_object_extent());
    ASSERT_POINT_EQ(R3::basis::Z, shape.normal(expect_in_point));
}

TEST(PolygonTest, QuadragonInTest) {
    raytrace::point A{1.0_p, 1.0_p, 0.0_p};
    raytrace::point B{1.0_p, -1.0_p, 0.0_p};
    raytrace::point C{-1.0_p, -1.0_p, 0.0_p};
    raytrace::point D{-1.0_p, 1.0_p, 0.0_p};
    raytrace::objects::polygon<4> shape{{A, B, C, D}};
    // scan the x, y plane every 0.25_p from -1.0_p to 1.0_p
    for (precision x = -1.0_p; x <= 1.0_p; x += 0.25_p) {
        for (precision y = -1.0_p; y <= 1.0_p; y += 0.25_p) {
            raytrace::point test_point{x, y, 0};
            if (shape.is_contained(test_point)) {
                ASSERT_TRUE(shape.is_surface_point(test_point));
            } else {
                ASSERT_FALSE(shape.is_surface_point(test_point));
            }
        }
    }
}

TEST(PolygonTest, MakeAPolygon4) {
    R3::point C{7.0_p, 42.0_p, -8.0_p};
    auto points = raytrace::objects::make_polygon_points<4>(2, C);
    raytrace::objects::polygon<4> shape{points};
    ASSERT_EQ(shape.get_type(), raytrace::objects::Type::Polygon);
    ASSERT_POINT_EQ(C, shape.position());
    ASSERT_TRUE(shape.is_contained(R3::origin));     // in object space, the center is the origin
    ASSERT_POINT_EQ(R3::basis::Z, shape.normal(C));  // in world space, the center is at C
}
