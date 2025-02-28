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

TEST(EntityTest, Constructors) {
    raytrace::point C{1, 2, 3};
    entity e{C};
    ASSERT_POINT_EQ(C, e.position());
    entity f{std::move(C)};
    ASSERT_POINT_EQ(C, f.position());
}

TEST(EntityTest, Assigners) {
    raytrace::point C{1, 2, 3};
    entity e{C};
    entity f;
    f = e;
    ASSERT_POINT_EQ(C, f.position());
    entity g;
    g = std::move(f);
    ASSERT_POINT_EQ(C, g.position());
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
    raytrace::point I = E.forward_transform(D);
    ASSERT_POINT_EQ(I, D); // no movement so should be equal
    E.scale(2.0_p, 1.0_p, 0.5_p);
    raytrace::point J = E.forward_transform(D);
    raytrace::point K = raytrace::point{4.0_p, 2.0_p, 1.0_p};
    ASSERT_POINT_EQ(J, K); // all points have scaled up
    ASSERT_POINT_EQ(C, E.position()); // the center is still the same
    E.move_by(M); // move the center
    ASSERT_POINT_EQ((C + M), E.position()); // the center has moved
    C += M;
    ASSERT_POINT_EQ(C, E.position()); // scale shouldn't affect position of the entity
    raytrace::point G = E.forward_transform(F);
    raytrace::point H = raytrace::point{4 * 2 + 1, 4 * 1 + 1, 4 * 0.5 + 1};
    ASSERT_POINT_EQ(H, G); // scaling should not effect translations, only points themselves
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

TEST(EntityTest, RotationScaleTranslate) {
    entity E{R3::origin};
    E.move_by(raytrace::vector{1, 2, 3});
    E.scale(5, 7, 9);
    E.rotation(iso::degrees{0}, iso::degrees{0}, iso::degrees{180});

    ASSERT_POINT_EQ(raytrace::point(1,2,3), E.position()); // center does not scale or rotate
    // 1,1,1 -> -4, -5, 12
    ASSERT_POINT_EQ(raytrace::point(-5 * 1 + 1, -7 * 1 + 2, 9 * 1 + 3) , E.forward_transform(raytrace::point(1, 1, 1)));
    // -4, -5, 12 -> 1, 1, 1
    ASSERT_POINT_EQ(raytrace::point(1, 1, 1), E.reverse_transform(raytrace::point(-5 * 1 + 1, -7 * 1 + 2, 9 * 1 + 3)));
}