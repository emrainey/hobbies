
#include "basal/gtest_helper.hpp"

#include <basal/basal.hpp>
#include <geometry/geometry.hpp>
#include <linalg/linalg.hpp>

#include "linalg/gtest_helper.hpp"
#include "geometry/gtest_helper.hpp"

using namespace geometry;
using namespace geometry::operators;

TEST(VectorTest, DefaultConstructor) {
    R3::vector v;
    ASSERT_EQ(3, v.dimensions);
    ASSERT_TRUE(v.is_zero());
}

TEST(VectorTest, ArrayConstructor) {
    precision a[] = {1.0_p, 2.0_p, 3.0_p};
    R3::vector v{3, a};
    ASSERT_EQ(3, v.dimensions);

    // indexing
    ASSERT_PRECISION_EQ(1.0_p, v[0]);
    ASSERT_PRECISION_EQ(2.0_p, v[1]);
    ASSERT_PRECISION_EQ(3.0_p, v[2]);

    ASSERT_THROW(v[3], basal::exception);

    ASSERT_THROW(R3::vector(3, nullptr), basal::exception);
    ASSERT_THROW(R3::vector(0, a), basal::exception);
    ASSERT_THROW(R3::vector(2, a), basal::exception);
}

TEST(VectorTest, ListInitializer) {
    R4::vector v4{{6.0_p, 7.0_p, 8.0_p, 9.0_p}};
    ASSERT_EQ(4, v4.dimensions);
    ASSERT_PRECISION_EQ(6.0_p, v4[0]);
    ASSERT_PRECISION_EQ(7.0_p, v4[1]);
    ASSERT_PRECISION_EQ(8.0_p, v4[2]);
    ASSERT_PRECISION_EQ(9.0_p, v4[3]);
    ASSERT_THROW(v4[4], basal::exception);
}

TEST(VectorTest, CopyConstructor) {
    R4::vector v4{{6.0_p, 7.0_p, 8.0_p, 9.0_p}};
    R4::vector _v4(v4);
    ASSERT_EQ(4, _v4.dimensions);
    ASSERT_PRECISION_EQ(6.0_p, _v4[0]);
    ASSERT_PRECISION_EQ(7.0_p, _v4[1]);
    ASSERT_PRECISION_EQ(8.0_p, _v4[2]);
    ASSERT_PRECISION_EQ(9.0_p, _v4[3]);
    ASSERT_THROW(_v4[4], basal::exception);
}

TEST(VectorTest, MoveConstructor) {
    R4::vector v4{{6.0_p, 7.0_p, 8.0_p, 9.0_p}};
    R4::vector _v4(std::move(v4));
    ASSERT_EQ(4, _v4.dimensions);
    ASSERT_PRECISION_EQ(6.0_p, _v4[0]);
    ASSERT_PRECISION_EQ(7.0_p, _v4[1]);
    ASSERT_PRECISION_EQ(8.0_p, _v4[2]);
    ASSERT_PRECISION_EQ(9.0_p, _v4[3]);
    ASSERT_THROW(_v4[4], basal::exception);
}
/*
TEST(VectorTest, ConversionConstructor) {
    matrix m{ {{6.0_p}, {7.0_p}, {8.0_p}, {9.0_p}} };
    R4::vector _v4(m);
    ASSERT_EQ(4, _v4.dimensions);
    ASSERT_PRECISION_EQ(6.0_p, _v4[0]);
    ASSERT_PRECISION_EQ(7.0_p, _v4[1]);
    ASSERT_PRECISION_EQ(8.0_p, _v4[2]);
    ASSERT_PRECISION_EQ(9.0_p, _v4[3]);
    ASSERT_THROW(_v4[4], basal::exception);

    matrix m2{ {{6.0_p, 7.0_p}, {8.0_p, 9.0_p}} };
    ASSERT_THROW(vector _v2(m2), basal::exception);
}
*/

TEST(VectorTest, CopyAssign) {
    R4::vector v4{{6.0_p, 7.0_p, 8.0_p, 9.0_p}};
    R4::vector _v4;
    _v4 = v4;
    ASSERT_EQ(4, _v4.dimensions);
    ASSERT_PRECISION_EQ(6.0_p, _v4[0]);
    ASSERT_PRECISION_EQ(7.0_p, _v4[1]);
    ASSERT_PRECISION_EQ(8.0_p, _v4[2]);
    ASSERT_PRECISION_EQ(9.0_p, _v4[3]);
    ASSERT_THROW(_v4[4], basal::exception);
}

TEST(VectorTest, MoveAssign) {
    R4::vector v4{{6.0_p, 7.0_p, 8.0_p, 9.0_p}};
    R4::vector _v4;
    _v4 = std::move(v4);
    ASSERT_EQ(4, _v4.dimensions);
    ASSERT_PRECISION_EQ(6.0_p, _v4[0]);
    ASSERT_PRECISION_EQ(7.0_p, _v4[1]);
    ASSERT_PRECISION_EQ(8.0_p, _v4[2]);
    ASSERT_PRECISION_EQ(9.0_p, _v4[3]);
    ASSERT_THROW(_v4[4], basal::exception);
}

/*
TEST(VectorTest, ConversionAssign) {

    matrix m{ {{6.0_p}, {7.0_p}, {8.0_p}, {9.0_p}} };
    R4::vector _v4;
    _v4 = m;
    ASSERT_EQ(4, _v4.dimensions);
    ASSERT_PRECISION_EQ(6.0_p, _v4[0]);
    ASSERT_PRECISION_EQ(7.0_p, _v4[1]);
    ASSERT_PRECISION_EQ(8.0_p, _v4[2]);
    ASSERT_PRECISION_EQ(9.0_p, _v4[3]);
    ASSERT_THROW(_v4[4], basal::exception);

    matrix m2{ {{6.0_p, 7.0_p}, {8.0_p, 9.0_p}} };
    ASSERT_EQ(2, m2.cols);
    vector _v2(2);
    ASSERT_THROW(_v2 = m2, basal::exception);
}
*/

TEST(VectorTest, TemplateConstructors) {
    vector_<2> v2{{1.0_p, 2.0_p}};
    vector_<3> v3{{1.0_p, 2.0_p, 3.0_p}};
    vector_<4> v4{{1.0_p, 2.0_p, 3.0_p, 4.0_p}};

    ASSERT_EQ(2, v2.dimensions);
    ASSERT_EQ(3, v3.dimensions);
    ASSERT_EQ(4, v4.dimensions);

    ASSERT_THROW(R2::vector v1{1.0_p}, basal::exception);  // the compiler may complain
    // ASSERT_THROW(R2::vector v1{{1.0_p, 2.0_p, 3.0_p}}, basal::exception); // the compiler may catch this
}

TEST(VectorTest, StaticsAndNegation) {
    ASSERT_EQ(3, R3::basis::X.dimensions);
    ASSERT_EQ(3, R3::basis::Y.dimensions);
    ASSERT_EQ(3, R3::basis::Z.dimensions);

    ASSERT_PRECISION_EQ(1.0_p, R3::basis::X[0]);
    ASSERT_PRECISION_EQ(0.0_p, R3::basis::X[1]);
    ASSERT_PRECISION_EQ(0.0_p, R3::basis::X[2]);

    ASSERT_PRECISION_EQ(0.0_p, R3::basis::Y[0]);
    ASSERT_PRECISION_EQ(1.0_p, R3::basis::Y[1]);
    ASSERT_PRECISION_EQ(0.0_p, R3::basis::Y[2]);

    ASSERT_PRECISION_EQ(0.0_p, R3::basis::Z[0]);
    ASSERT_PRECISION_EQ(0.0_p, R3::basis::Z[1]);
    ASSERT_PRECISION_EQ(1.0_p, R3::basis::Z[2]);

    auto neg_X = -R3::basis::X;
    ASSERT_EQ(3, neg_X.dimensions);
    ASSERT_EQ(3, (-R3::basis::X).dimensions);
    ASSERT_EQ(3, (-R3::basis::Y).dimensions);
    ASSERT_EQ(3, (-R3::basis::Z).dimensions);

    ASSERT_PRECISION_EQ(-1.0_p, (-R3::basis::X)[0]);
    ASSERT_PRECISION_EQ(0.0_p, (-R3::basis::X)[1]);
    ASSERT_PRECISION_EQ(0.0_p, (-R3::basis::X)[2]);

    ASSERT_PRECISION_EQ(0.0_p, (-R3::basis::Y)[0]);
    ASSERT_PRECISION_EQ(-1.0_p, (-R3::basis::Y)[1]);
    ASSERT_PRECISION_EQ(0.0_p, (-R3::basis::Y)[2]);

    ASSERT_PRECISION_EQ(0.0_p, (-R3::basis::Z)[0]);
    ASSERT_PRECISION_EQ(0.0_p, (-R3::basis::Z)[1]);
    ASSERT_PRECISION_EQ(-1.0_p, (-R3::basis::Z)[2]);
}

TEST(VectorTest, NegationConstants) {
    // when referencing the basis vectors, the compiler may optimize so be sure to put the negation in ()
    ASSERT_EQ(3, (-R3::basis::X).dimensions);
    ASSERT_EQ(3, (-R3::basis::Y).dimensions);
    ASSERT_EQ(3, (-R3::basis::Z).dimensions);
    ASSERT_VECTOR_EQ(R3::vector({-1.0_p, 0.0_p, 0.0_p}), (-R3::basis::X));
    ASSERT_VECTOR_EQ(R3::vector({0.0_p, -1.0_p, 0.0_p}), (-R3::basis::Y));
    ASSERT_VECTOR_EQ(R3::vector({0.0_p, 0.0_p, -1.0_p}), (-R3::basis::Z));
}

TEST(VectorTest, AxisCross) {
    using namespace R3;
    vector xy{cross(basis::X, basis::Y)};  // Copy Construct
    ASSERT_VECTOR_EQ(basis::Z, xy);
    xy = cross(basis::X, basis::Y);  // Copy Assign
    ASSERT_VECTOR_EQ(basis::Z, xy);
    xy = cross(basis::X, vector{{0.0_p, 1.0_p, 0.0_p}});
    ASSERT_VECTOR_EQ(basis::Z, xy);
    xy = cross(vector{{1.0_p, 0.0_p, 0.0_p}}, basis::Y);
    ASSERT_VECTOR_EQ(basis::Z, xy);
    xy = cross(vector{{1.0_p, 0.0_p, 0.0_p}}, vector{{0.0_p, 1.0_p, 0.0_p}});
    ASSERT_VECTOR_EQ(basis::Z, xy);

    vector yz{cross(basis::Y, basis::Z)};  // Copy Construct
    ASSERT_VECTOR_EQ(basis::X, yz);
    yz = cross(basis::Y, basis::Z);  // Copy Assign
    ASSERT_VECTOR_EQ(basis::X, yz);

    vector zx{cross(basis::Z, basis::X)};  // Copy Construct
    ASSERT_VECTOR_EQ(basis::Y, zx);
    zx = cross(basis::Z, basis::X);  // Copy Assign
    ASSERT_VECTOR_EQ(basis::Y, zx);
}

TEST(VectorTest, AxisDots) {
    using namespace R3;
    ASSERT_PRECISION_EQ(0.0_p, dot(basis::X, basis::Y));
    ASSERT_PRECISION_EQ(0.0_p, dot(basis::X, basis::Z));
    ASSERT_PRECISION_EQ(0.0_p, dot(basis::Y, basis::X));
    ASSERT_PRECISION_EQ(0.0_p, dot(basis::Y, basis::Z));
    ASSERT_PRECISION_EQ(0.0_p, dot(basis::Z, basis::X));
    ASSERT_PRECISION_EQ(0.0_p, dot(basis::Z, basis::Y));

    ASSERT_TRUE(orthogonal(basis::X, basis::Y));
    ASSERT_TRUE(orthogonal(basis::X, basis::Z));
    ASSERT_TRUE(orthogonal(basis::Y, basis::X));
    ASSERT_TRUE(orthogonal(basis::Y, basis::Z));
    ASSERT_TRUE(orthogonal(basis::Z, basis::X));
    ASSERT_TRUE(orthogonal(basis::Z, basis::Y));

    ASSERT_PRECISION_EQ(1.0_p, dot(basis::X, basis::X));
    ASSERT_PRECISION_EQ(1.0_p, dot(basis::Y, basis::Y));
    ASSERT_PRECISION_EQ(1.0_p, dot(basis::Z, basis::Z));

    ASSERT_TRUE(parallel(basis::X, basis::X));
    ASSERT_FALSE(parallel(basis::X, basis::Y));
    ASSERT_FALSE(parallel(basis::X, basis::Z));
    ASSERT_TRUE(parallel(basis::Y, basis::Y));
    ASSERT_FALSE(parallel(basis::Y, basis::X));
    ASSERT_FALSE(parallel(basis::Y, basis::Z));
    ASSERT_TRUE(parallel(basis::Z, basis::Z));
    ASSERT_FALSE(parallel(basis::Z, basis::X));
    ASSERT_FALSE(parallel(basis::Z, basis::Y));
}

TEST(VectorTest, DotMagAndAngle) {
    using namespace R3;
    vector a{{2, 3, 4}};
    vector b{{5, 6, 7}};
    ASSERT_FALSE(orthogonal(a, b));
    ASSERT_FALSE(parallel(a, b));

    ASSERT_PRECISION_EQ(56, dot(a, b));
    ASSERT_PRECISION_EQ(29, a.quadrance());
    ASSERT_PRECISION_EQ(29, Q(a));
    ASSERT_PRECISION_EQ(sqrt(29), a.magnitude());
    ASSERT_PRECISION_EQ(110, b.quadrance());
    ASSERT_PRECISION_EQ(110, Q(b));
    ASSERT_PRECISION_EQ(sqrt(110), b.magnitude());
    iso::radians angle_ab = angle(a, b);
    ASSERT_NEAR(std::acos(56.0_p / (sqrt(29.0_p) * sqrt(110.0_p))), angle_ab.value, basal::epsilon);

    // FIXME (Gtest) Compute Spread Independently here, don't just repeat the code
    ASSERT_PRECISION_EQ(1.0_p - ((56.0_p * 56.0_p) / (Q(a) * Q(b))), spread(a, b));
}

TEST(VectorTest, AdditionSubtraction) {
    using namespace linalg::operators;
    R2::vector A{{5, 7}};
    R2::vector B{{1, 1}};

    R2::vector C = A - B;
    R2::vector E{{4, 6}};
    ASSERT_VECTOR_EQ(E, C);
    R2::vector D = A + B;
    R2::vector F{{6, 8}};
    ASSERT_VECTOR_EQ(F, D);
}

TEST(VectorTest, Crosses) {
    using namespace R3;
    vector a{{1, 5, 2}};
    vector b{{-5, 1, 0}};
    vector c{{-2, -10, 26}};

    ASSERT_VECTOR_EQ(b, cross(basis::Z, a));
    ASSERT_VECTOR_EQ(c, cross(a, cross(basis::Z, a)));
}

TEST(VectorTest, Rodrigues) {
    R3::vector v{{1, 0, 1}};
    R3::vector k{{0, 0, 1}};
    iso::radians theta(iso::tau / 2.0_p);
    R3::vector ground_truth{{-1, 0, 1}};
    R3::vector rod = R3::rodrigues(k, v, theta);
    // print_this(rod);
    ASSERT_VECTOR_EQ(ground_truth, R3::rodrigues(k, v, theta));
    // TODO (Gtest) Add more test cases here
}

TEST(VectorTest, Projection) {
    R3::vector u{{2, -1, 3}};
    R3::vector a{{4, -1, 2}};
    R3::vector proj_w1 = u.project_along(a);
    R3::vector proj_w2 = u.project_orthogonal(a);
    iso::radians r{angle(proj_w1, proj_w2)};
    iso::degrees d;
    iso::convert(d, r);
    ASSERT_PRECISION_EQ(90.0_p, d.value);
    ASSERT_TRUE(orthogonal(proj_w1, proj_w2));
}

TEST(VectorTest, CauchySchwartz) {
    using namespace linalg::operators;
    R3::vector u{{2, -1, 1}};
    R3::vector v{{1, 1, 2}};
    ASSERT_PRECISION_EQ(iso::tau / 6.0_p, angle(u, v).value);
    // cauchy-schwartz inequality
    ASSERT_TRUE(dot(u, v) <= (u.norm() * v.norm()));
    // triange inequality
    ASSERT_TRUE((u + v).norm() <= (u.norm() + v.norm()));
}

TEST(VectorTest, TripleProduct) {
    R3::vector u{{3, -2, -5}};
    R3::vector v{{1, 4, -4}};
    R3::vector w{{0, 3, 2}};
    ASSERT_PRECISION_EQ(49.0_p, R3::triple(u, v, w));
    ASSERT_PRECISION_EQ(49.0_p, R3::triple(w, u, v));
    ASSERT_PRECISION_EQ(49.0_p, R3::triple(v, w, u));
}

TEST(VectorTest, Angles) {
    R3::vector a{{1, 0, 1}};

    iso::radians A1 = angle(R3::basis::Z, R3::basis::Y);
    ASSERT_PRECISION_EQ(iso::pi / 2, A1.value);
    iso::radians A2 = angle(R3::basis::Z, -R3::basis::Z);
    ASSERT_PRECISION_EQ(iso::pi, A2.value);
    iso::radians A3 = angle(R3::basis::Z, R3::basis::Z);
    ASSERT_PRECISION_EQ(0.0_p, A3.value);

    iso::radians A4 = angle(R3::basis::Z, a);
    ASSERT_PRECISION_EQ(iso::pi / 4, A4.value);
}

TEST(VectorTest, NearlyOrthogonal) {
    R3::vector a{{1, 2, 3}};
    R3::vector b = R3::nearly_orthogonal(a.normalized());
    R3::vector c = R3::cross(a, b);
    ASSERT_PRECISION_EQ(1.0_p, b.magnitude());
    ASSERT_FALSE(R3::parallel(a, b));
    ASSERT_TRUE(orthogonal(a, c));
}
