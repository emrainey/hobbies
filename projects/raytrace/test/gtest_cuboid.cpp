#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <iostream>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

TEST(CuboidTest, Type) {
    using namespace raytrace;
    using namespace raytrace::objects;
    cuboid c0{R3::origin, 1.0_p, 1.0_p, 1.0_p};
    ASSERT_EQ(c0.get_type(), Type::Cuboid);
}

TEST(CuboidTest, Normals) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point P{3, 3, 3};
    cuboid c0{P, 1.0_p, 1.0_p, 1.0_p};

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
    cuboid c0{P, 1.0_p, 1.0_p, 1.0_p};
    raytrace::point Q{2.0_p, 2.0_p, 2.0_p};
    vector R{{1, 1, 0}};

    ray complete_miss(Q, R.normalized());
    geometry::intersection icm = c0.intersect(complete_miss).intersect;
    ASSERT_EQ(geometry::IntersectionType::None, get_type(icm));
}

TEST(CuboidTest, IntersectionHitX) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point P{0, 0, 0};
    cuboid c0{P, 1.0_p, 1.0_p, 1.0_p};
    raytrace::point Q{2.0_p, 0.75_p, 0.25_p};
    vector R{{-1, 0, 0}};
    ray hit0(Q, R);
    raytrace::point h0{1.0_p, 0.75_p, 0.25_p};

    geometry::intersection ipx = c0.intersect(hit0).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ipx));
    raytrace::point i0 = as_point(ipx);
    ASSERT_POINT_EQ(h0, i0);
}

TEST(CuboidTest, IntersectionHitOffOrigin) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point P{9, -14, 77};
    cuboid c0{P, 2.0_p, 3.0_p, 4.0_p};
    raytrace::point Q{4.0_p, 0.75_p, 0.25_p};
    raytrace::vector v1 = P - R3::origin;
    vector R{{-1, 0, 0}};
    ray hit0((Q + v1), R);
    raytrace::point h0{2.0_p, 0.75_p, 0.25_p};
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
    precision w = 0.5_p;  // the half-width of the cuboid
    cuboid c0{P, w, w, w};
    precision u = 1.0_p / 8.0_p;  // the testing step

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
    precision w = 2.0_p;
    cuboid c0{center, w, w, w};
    raytrace::point A{w, 0, 0};
    raytrace::point B{-w, 0, 0};
    raytrace::point C{0, w, 0};
    raytrace::point D{0, -w, 0};
    raytrace::point E{0, 0, w};
    raytrace::point F{0, 0, -w};

    ray rA0(A, (A - center).normalized());
    ray rB0(B, (B - center).normalized());
    ray rC0(C, (C - center).normalized());
    ray rD0(D, (D - center).normalized());
    ray rE0(E, (E - center).normalized());
    ray rF0(F, (F - center).normalized());

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

    ray rA1(A, (center - A).normalized());
    ray rB1(B, (center - B).normalized());
    ray rC1(C, (center - C).normalized());
    ray rD1(D, (center - D).normalized());
    ray rE1(E, (center - E).normalized());
    ray rF1(F, (center - F).normalized());

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

TEST(CuboidTest, IsSurfacePoint) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point center(0, 0, 0);
    precision w = 2.0_p;
    cuboid c0{center, w, w, w};
    raytrace::point A{w, 0, 0};
    raytrace::point B{-w, 0, 0};
    raytrace::point C{0, w, 0};
    raytrace::point D{0, -w, 0};
    raytrace::point E{0, 0, w};
    raytrace::point F{0, 0, -w};
    raytrace::point G{w + 0.1_p, 0, 0};
    raytrace::point H{0, -w - 0.1_p, 0};
    raytrace::point I{0, 0, w + 0.1_p};
    raytrace::point J{0.0_p, 0.0_p, 0.0_p};
    EXPECT_TRUE(c0.is_surface_point(A));
    EXPECT_TRUE(c0.is_surface_point(B));
    EXPECT_TRUE(c0.is_surface_point(C));
    EXPECT_TRUE(c0.is_surface_point(D));
    EXPECT_TRUE(c0.is_surface_point(E));
    EXPECT_TRUE(c0.is_surface_point(F));
    EXPECT_FALSE(c0.is_surface_point(G));
    EXPECT_FALSE(c0.is_surface_point(H));
    EXPECT_FALSE(c0.is_surface_point(I));
    EXPECT_FALSE(c0.is_surface_point(J));
}