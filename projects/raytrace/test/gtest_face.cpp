#include <gtest/gtest.h>

#include <raytrace/objects/face.hpp>

#include "geometry/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

using namespace raytrace;
using namespace linalg;

TEST(FaceTest, Basics) {
    raytrace::point A{1, 0, 0};
    raytrace::point B{0, 0, 0};
    raytrace::point C{0, 1, 0};
    raytrace::point D{0.5_p, 0.5_p, 0};
    raytrace::objects::face f{A, B, C};
    EXPECT_EQ(f.get_type(), raytrace::objects::Type::Face);
    f.print(std::cout, "Face");
    EXPECT_EQ(f.points().size(), 3u);
    EXPECT_POINT_EQ(raytrace::point(1.0_p / 3.0_p, 1.0_p / 3.0_p, 0.0_p / 3.0_p), f.position());
    raytrace::vector n = f.normal(D);
    EXPECT_VECTOR_EQ(R3::basis::Z, f.normal(D));
    image::point p = f.map(R3::origin);
    EXPECT_PRECISION_EQ(p[0], 1.0_p / 3.0_p);
    EXPECT_PRECISION_EQ(p[1], 1.0_p / 3.0_p);

    raytrace::objects::face f2{f};
    EXPECT_POINT_EQ(raytrace::point(1.0_p / 3.0_p, 1.0_p / 3.0_p, 0.0_p / 3.0_p), f2.position());
}

TEST(FaceTest, Texture) {
    raytrace::point A{3, 2, 0};
    raytrace::point B{1, 1, 0};
    raytrace::point C{-2, 3, 0};
    raytrace::point D{1.0_p, 2.0_p, 0};
    raytrace::objects::face f{A, B, C, image::point{1, 0}, image::point{0, 0}, image::point{0, 1}};
    f.print(std::cout, "Face");
    EXPECT_EQ(f.points().size(), 3u);
    raytrace::vector n = f.normal(D);
    EXPECT_VECTOR_EQ(R3::basis::Z, f.normal(D));
    image::point p = f.map(D);
    // FIXME uv mapping needs to be verified, but this is not helpful
    // EXPECT_PRECISION_EQ(p[0], 0.44721359549995793_p);
    // EXPECT_PRECISION_EQ(p[1], 0.55470019622522915_p);
}

TEST(FaceTest, Normals) {
    raytrace::point A{5, 1, 0};
    raytrace::point B{1, 1, 0};
    raytrace::point C{3, 4, 0};
    raytrace::point D{3.0_p, 2.0_p, 0};
    image::point uva{1, 0};
    image::point uvb{0, 0};
    image::point uvc{0, 1};
    precision a = sqrt(2.0_p / 3.0_p);
    precision b = sqrt(1.0_p / 3.0_p);
    raytrace::vector na{a, -b, b};
    raytrace::vector nb{-a, -b, b};
    raytrace::vector nc{b, a, b};
    raytrace::objects::face shape{A, B, C, uva, uvb, uvc, na.normalize(), nb.normalize(), nc.normalize()};
    auto n = shape.normal(D);
    auto expected = raytrace::vector{0.310938_p, -0.182143_p, 0.932814_p};
    std::cout << "Expected: " << expected << std::endl;
    std::cout << "Normal: " << n << std::endl;
    EXPECT_VECTOR_EQ(expected, shape.normal(D));
}

TEST(FaceTest, Intersect) {
    raytrace::point A{1, 0, 0};
    raytrace::point B{0, 0, 0};
    raytrace::point C{0, 1, 0};
    raytrace::objects::face shape{A, B, C};

    // the polygon's position is at the centeroid of the points
    EXPECT_POINT_EQ(raytrace::point(1.0_p / 3.0_p, 1.0_p / 3.0_p, 0.0_p / 3.0_p), shape.position());

    raytrace::ray r{raytrace::point{0.4_p, 0.4_p, 1.0_p}, -R3::basis::Z};
    auto hit = shape.intersect(r);
    EXPECT_EQ(geometry::IntersectionType::Point, get_type(hit.intersect));
    EXPECT_POINT_EQ(raytrace::point(0.4_p, 0.4_p, 0.0_p), as_point(hit.intersect));
    EXPECT_VECTOR_EQ(R3::basis::Z, hit.normal);
}

TEST(FaceTest, IntersectAfterMove) {
    raytrace::point A{1, 0, 0};
    raytrace::point B{0, 0, 0};
    raytrace::point C{0, 1, 0};
    raytrace::point E{1.0_p / 3.0_p, 1.0_p / 3.0_p, 0.0_p / 3.0_p};
    raytrace::objects::face shape{A, B, C};
    EXPECT_POINT_EQ(E, shape.position());
    shape.move_by(raytrace::vector{1, 1, 1});
    EXPECT_POINT_EQ(raytrace::point(4.0_p / 3.0_p, 4.0_p / 3.0_p, 3.0_p / 3.0_p), shape.position());
    {
        raytrace::ray r{raytrace::point{0.4_p, 0.4_p, 1.0_p}, -R3::basis::Z};
        auto hit = shape.intersect(r);
        EXPECT_EQ(geometry::IntersectionType::None, get_type(hit.intersect));
    }
    shape.print(std::cout, "Face");
    {
        raytrace::ray r{raytrace::point{1.4_p, 1.4_p, 2.0_p}, -R3::basis::Z};
        auto hit = shape.intersect(r);
        EXPECT_EQ(geometry::IntersectionType::Point, get_type(hit.intersect));
        EXPECT_POINT_EQ(raytrace::point(1.4_p, 1.4_p, 1.0_p), as_point(hit.intersect));
        EXPECT_VECTOR_EQ(R3::basis::Z, hit.normal);
    }
}