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
    f.print("Face");
    EXPECT_EQ(f.points().size(), 3u);
    EXPECT_POINT_EQ(raytrace::point(1.0_p/3.0_p, 1.0_p/3.0_p, 0.0_p/3.0_p), f.position());
    raytrace::vector n = f.normal(D);
    EXPECT_VECTOR_EQ(R3::basis::Z, f.normal(D));
    image::point p = f.map(D);
    EXPECT_PRECISION_EQ(p[0], 0.5_p);
    EXPECT_PRECISION_EQ(p[1], 0.5_p);
}

TEST(FaceTest, Texture) {
    raytrace::point A{3, 2, 0};
    raytrace::point B{1, 1, 0};
    raytrace::point C{-2,3, 0};
    raytrace::point D{1.0_p, 2.0_p, 0};
    raytrace::objects::face f{A, B, C, image::point{1, 0}, image::point{0, 0}, image::point{0, 1}};
    f.print("Face");
    EXPECT_EQ(f.points().size(), 3u);
    raytrace::vector n = f.normal(D);
    EXPECT_VECTOR_EQ(R3::basis::Z, f.normal(D));
    image::point p = f.map(D);
    EXPECT_PRECISION_EQ(p[0], 0.44721359549995793_p);
    EXPECT_PRECISION_EQ(p[1], 0.55470019622522915_p);
}

TEST(FaceTest, Normals) {
    raytrace::point A{5, 1, 0};
    raytrace::point B{1, 1, 0};
    raytrace::point C{3, 4, 0};
    raytrace::point D{3.0_p, 2.0_p, 0};
    image::point uva{1, 0};
    image::point uvb{0, 0};
    image::point uvc{0, 1};
    precision a = sqrt(2.0_p/3.0_p);
    precision b = sqrt(1.0_p/3.0_p);
    raytrace::vector na{a, -b, b};
    raytrace::vector nb{-a, -b, b};
    raytrace::vector nc{b, a, b};
    raytrace::objects::face f{A, B, C, uva, uvb, uvc, na.normalize(), nb.normalize(), nc.normalize()};
    auto n = f.normal(D);
    auto expected = raytrace::vector{0.310938_p,-0.182143_p,0.932814_p};
    std::cout << "Expected: " << expected << std::endl;
    std::cout << "Normal: " << n << std::endl;
    EXPECT_VECTOR_EQ(expected, f.normal(D));
}