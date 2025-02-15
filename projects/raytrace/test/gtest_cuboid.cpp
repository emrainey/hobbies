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

    raytrace::point P{3, 3, 3};
    cuboid c0{P, 1.0, 1.0, 1.0};

    raytrace::point pX = P + raytrace::vector{1, 0, 0};
    raytrace::point nX = P + raytrace::vector{-1, 0, 0};
    raytrace::point pY = P + raytrace::vector{0, 1, 0};
    raytrace::point nY = P + raytrace::vector{0, -1, 0};
    raytrace::point pZ = P + raytrace::vector{0, 0, 1};
    raytrace::point nZ = P + raytrace::vector{0, 0, -1};

    vector nx = -R3::basis::X;
    vector ny = -R3::basis::Y;
    vector nz = -R3::basis::Z;

    vector pnX = c0.normal(pX);
    vector nnX = c0.normal(nX);
    vector pnY = c0.normal(pY);
    vector nnY = c0.normal(nY);
    vector pnZ = c0.normal(pZ);
    vector nnZ = c0.normal(nZ);

    ASSERT_VECTOR_EQ(R3::basis::X, pnX);
    ASSERT_VECTOR_EQ(nx, nnX);
    ASSERT_VECTOR_EQ(R3::basis::Y, pnY);
    ASSERT_VECTOR_EQ(ny, nnY);
    ASSERT_VECTOR_EQ(R3::basis::Z, pnZ);
    ASSERT_VECTOR_EQ(nz, nnZ);
}

TEST(CuboidTest, IntersectionMissed) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point P{0, 0, 0};
    cuboid c0{P, 1.0, 1.0, 1.0};
    raytrace::point Q{2.0, 2.0, 2.0};
    vector R{{1, 1, 0}};

    ray complete_miss(Q, R);
    geometry::intersection icm = c0.intersect(complete_miss).intersect;
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

    geometry::intersection ipx = c0.intersect(hit0).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ipx));
    raytrace::point i0 = as_point(ipx);
    ASSERT_POINT_EQ(h0, i0);
}


TEST(CuboidTest, IntersectionHitOffOrigin) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point P{9, -14, 77};
    cuboid c0{P, 1.0, 1.0, 1.0};
    raytrace::point Q{2.0, 0.75, 0.25};
    raytrace::vector v1 = P - R3::origin;
    vector R{{-1, 0, 0}};
    ray hit0((Q + v1), R);
    raytrace::point h0{1.0, 0.75, 0.25};
    raytrace::point h1 = h0 + v1;

    geometry::intersection ipx = c0.intersect(hit0).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ipx));
    raytrace::point i0 = as_point(ipx);
    ASSERT_POINT_EQ(h1, i0);
}

TEST(CuboidTest, IntersectionMassXY) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point P{7, 22, -19};
    precision w = 0.5;  // the half-width of the cuboid
    cuboid c0{P, w, w, w};
    precision u = 1.0 / 8.0;  // the testing step

    // in the XY plane
    for (precision y = (-w + u + P.y); y < (w + P.y); y += u) {
        for (precision x = (-w + u + P.x); x < (w + P.x); x += u) {
            raytrace::point R{x, y, P.z + (2 * w)};
            vector N{{0, 0, -1}};
            ray W{R, N};
            geometry::intersection I = c0.intersect(W).intersect;
            EXPECT_EQ(geometry::IntersectionType::Point, get_type(I)) << "At y=" << y << ", x=" << x << std::endl;
            raytrace::point Q{x, y, P.z + w};
            EXPECT_POINT_EQ(Q, as_point(I));
        }
    }
}

TEST(CuboidTest, IntersectionsOnSurface) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point center(0, 0, 0);
    precision w = 2.0;
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

    geometry::intersection irA0c0 = c0.intersect(rA0).intersect;
    geometry::intersection irB0c0 = c0.intersect(rB0).intersect;
    geometry::intersection irC0c0 = c0.intersect(rC0).intersect;
    geometry::intersection irD0c0 = c0.intersect(rD0).intersect;
    geometry::intersection irE0c0 = c0.intersect(rE0).intersect;
    geometry::intersection irF0c0 = c0.intersect(rF0).intersect;

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

    geometry::intersection irA1c0 = c0.intersect(rA1).intersect;
    geometry::intersection irB1c0 = c0.intersect(rB1).intersect;
    geometry::intersection irC1c0 = c0.intersect(rC1).intersect;
    geometry::intersection irD1c0 = c0.intersect(rD1).intersect;
    geometry::intersection irE1c0 = c0.intersect(rE1).intersect;
    geometry::intersection irF1c0 = c0.intersect(rF1).intersect;

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
