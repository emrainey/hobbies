#include <gtest/gtest.h>

#include <raytrace/raytrace.hpp>

#include "geometry/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

using namespace raytrace;
using namespace linalg;

TEST(EntityTest, Basics) {
    entity e;
    raytrace::point c{0, 0, 0};
    matrix I = matrix::identity(3, 3);
    ASSERT_POINT_EQ(c, e.position());
    ASSERT_MATRIX_EQ(I, e.rotation());

    // since we haven't moved it, object space is world space
    raytrace::point P{1, 1, 1};
    raytrace::point O = e.forward_transform(P);
    ASSERT_POINT_EQ(P, O);
    raytrace::point Q = e.reverse_transform(O);
    ASSERT_POINT_EQ(P, Q);
}

TEST(EntityTest, MoveBy) {
    raytrace::point C{14, -18, -1};
    entity e{C};

    ASSERT_POINT_EQ(C, e.position());
    raytrace::point D{-3, -3, 19};
    raytrace::vector V = D - C;
    e.move_by(V);
    ASSERT_POINT_EQ(D, e.position());
}

TEST(EntityTest, Transforms) {
    entity e;
    // Rotate the thing and move it then
    iso::degrees rX{0};
    iso::degrees rY{0};
    iso::degrees rZ{90};
    e.rotation(rX, rY, rZ);
    raytrace::point center(1, 1, 1);
    e.position(center);

    raytrace::point c{0, 0, 0};
    raytrace::point p0 = e.forward_transform(c);
    ASSERT_POINT_EQ(center, p0);

    raytrace::point p1{2, 2, 2};
    raytrace::point p2 = e.forward_transform(p1);
    raytrace::point p3{-1, 3, 3};
    ASSERT_POINT_EQ(p3, p2);
    raytrace::point p4 = e.reverse_transform(p3);
    ASSERT_POINT_EQ(p1, p4);
}

TEST(EntityTest, Scaling) {
    raytrace::point C{0, 0, 0};
    raytrace::point D{2, 2, 2};
    raytrace::point F{4, 4, 4};
    raytrace::vector M{1, 1, 1};
    entity E{C};
    E.scale(2.0, 2.0, 2.0);
    E.move_by(M);
    C += M;
    ASSERT_POINT_EQ(C, E.position()); // scale shouldn't affect position
    raytrace::point G = E.forward_transform(D);
    raytrace::point H = F + M;
    ASSERT_POINT_EQ(H, G); // scaling should effect translations
}

TEST(EntityTest, UpwardsTranslation) {
    raytrace::point C{0, 0, 0};
    raytrace::point D{0, 0, 10};
    raytrace::point E{0, 0, 12};
    entity E0{C};
    raytrace::vector M = D - C;
    E0.move_by(M);
    ASSERT_VECTOR_EQ(M, E0.translation());
    raytrace::point F = E0.reverse_transform(E);
    raytrace::point G{0, 0, 2};
    ASSERT_POINT_EQ(G, F);
}