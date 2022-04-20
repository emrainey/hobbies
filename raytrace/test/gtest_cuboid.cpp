#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <iostream>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

TEST(CuboidTest, Normals) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point P{0, 0, 0};
    cuboid c0{P, 1.0, 1.0, 1.0};

    raytrace::point pX{1, 0, 0};
    raytrace::point nX{-1, 0, 0};
    raytrace::point pY{0, 1, 0};
    raytrace::point nY{0, -1, 0};
    raytrace::point pZ{0, 0, 1};
    raytrace::point nZ{0, 0, -1};

    vector nx = -R3::basis::X;
    vector ny = -R3::basis::Y;
    vector nz = -R3::basis::Z;

    ASSERT_VECTOR_EQ(R3::basis::X, c0.normal(pX));
    ASSERT_VECTOR_EQ(nx, c0.normal(nX));
    ASSERT_VECTOR_EQ(R3::basis::Y, c0.normal(pY));
    ASSERT_VECTOR_EQ(ny, c0.normal(nY));
    ASSERT_VECTOR_EQ(R3::basis::Z, c0.normal(pZ));
    ASSERT_VECTOR_EQ(nz, c0.normal(nZ));
}

TEST(CuboidTest, IntersectionMissed) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point P{0, 0, 0};
    cuboid c0{P, 1.0, 1.0, 1.0};
    raytrace::point Q{2.0, 2.0, 2.0};
    vector R{{1, 1, 0}};

    ray complete_miss(Q, R);
    geometry::intersection icm = c0.intersect(complete_miss);
    ASSERT_EQ(geometry::IntersectionType::None, get_type(icm));
}

TEST(CuboidTest, IntersectionHitX) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point P{0, 0, 0};
    cuboid c0{P, 1.0, 1.0, 1.0};
    raytrace::point Q{2.0, 0.75, 0.25};
    vector R{{-1, 0, 0}};
    ray hit0(Q, R);
    raytrace::point h0{1.0, 0.75, 0.25};

    geometry::intersection ipx = c0.intersect(hit0);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ipx));
    raytrace::point i0 = as_point(ipx);
    ASSERT_POINT_EQ(h0, i0);
}

TEST(CuboidTest, IntersectionMassXY) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point P{7, 22, -19};
    element_type w = 0.5;  // the half-width of the cuboid
    cuboid c0{P, w, w, w};
    element_type u = 1.0 / 32.0;  // the testing step

    // in the XY plane
    for (element_type y = (-w + u + P.y); y < (w + P.y); y += u) {
        for (element_type x = (-w + u + P.x); x < (w + P.x); x += u) {
            raytrace::point R{x, y, P.z + (2 * w)};
            vector N{{0, 0, -1}};
            ray W{R, N};
            geometry::intersection I = c0.intersect(W);
            EXPECT_EQ(geometry::IntersectionType::Point, get_type(I)) << "At y=" << y << ", x=" << x << std::endl;
            raytrace::point Q{x, y, P.z + w};
            EXPECT_TRUE(Q == as_point(I));
        }
    }
}

TEST(CuboidTest, IntersectionsOnSurface) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point center(0, 0, 0);
    element_type w = 2.0;
    cuboid c0{center, w, w, w};
    raytrace::point A{w, 0, 0};
    raytrace::point B{-w, 0, 0};
    raytrace::point C{0, w, 0};
    raytrace::point D{0, -w, 0};
    raytrace::point E{0, 0, w};
    raytrace::point F{0, 0, -w};

    ray rA0(A, A - center);
    ray rB0(B, B - center);
    ray rC0(C, C - center);
    ray rD0(D, D - center);
    ray rE0(E, E - center);
    ray rF0(F, F - center);

    geometry::intersection irA0c0 = c0.intersect(rA0);
    geometry::intersection irB0c0 = c0.intersect(rB0);
    geometry::intersection irC0c0 = c0.intersect(rC0);
    geometry::intersection irD0c0 = c0.intersect(rD0);
    geometry::intersection irE0c0 = c0.intersect(rE0);
    geometry::intersection irF0c0 = c0.intersect(rF0);

    EXPECT_EQ(geometry::IntersectionType::None, get_type(irA0c0));
    EXPECT_EQ(geometry::IntersectionType::None, get_type(irB0c0));
    EXPECT_EQ(geometry::IntersectionType::None, get_type(irC0c0));
    EXPECT_EQ(geometry::IntersectionType::None, get_type(irD0c0));
    EXPECT_EQ(geometry::IntersectionType::None, get_type(irE0c0));
    EXPECT_EQ(geometry::IntersectionType::None, get_type(irF0c0));

    ray rA1(A, center - A);
    ray rB1(B, center - B);
    ray rC1(C, center - C);
    ray rD1(D, center - D);
    ray rE1(E, center - E);
    ray rF1(F, center - F);

    geometry::intersection irA1c0 = c0.intersect(rA1);
    geometry::intersection irB1c0 = c0.intersect(rB1);
    geometry::intersection irC1c0 = c0.intersect(rC1);
    geometry::intersection irD1c0 = c0.intersect(rD1);
    geometry::intersection irE1c0 = c0.intersect(rE1);
    geometry::intersection irF1c0 = c0.intersect(rF1);

    EXPECT_EQ(geometry::IntersectionType::Point, get_type(irA1c0));
    ASSERT_POINT_EQ(A, as_point(irA1c0));
    EXPECT_EQ(geometry::IntersectionType::Point, get_type(irB1c0));
    ASSERT_POINT_EQ(B, as_point(irB1c0));
    EXPECT_EQ(geometry::IntersectionType::Point, get_type(irC1c0));
    ASSERT_POINT_EQ(C, as_point(irC1c0));
    EXPECT_EQ(geometry::IntersectionType::Point, get_type(irD1c0));
    ASSERT_POINT_EQ(D, as_point(irD1c0));
    EXPECT_EQ(geometry::IntersectionType::Point, get_type(irE1c0));
    ASSERT_POINT_EQ(E, as_point(irE1c0));
    EXPECT_EQ(geometry::IntersectionType::Point, get_type(irF1c0));
    ASSERT_POINT_EQ(F, as_point(irF1c0));
}
