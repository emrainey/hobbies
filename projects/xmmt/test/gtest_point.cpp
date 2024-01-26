#include "gtest_helper.hpp"

TYPED_TEST(XMMContainer2, PointDefaultConstruction) {
    using namespace intel;
    typename TypeParam::point v{};
    EXPECT_PRECISION_EQ(zero, v.c.x);
    EXPECT_PRECISION_EQ(zero, v.c.y);
}

TYPED_TEST(XMMContainer3, PointDefaultConstruction) {
    using namespace intel;
    typename TypeParam::point v{};
    EXPECT_PRECISION_EQ(zero, v.c.x);
    EXPECT_PRECISION_EQ(zero, v.c.y);
    EXPECT_PRECISION_EQ(zero, v.c.z);
}

TYPED_TEST(XMMContainer4, PointDefaultConstruction) {
    using namespace intel;
    typename TypeParam::point v{};
    EXPECT_PRECISION_EQ(zero, v.c.x);
    EXPECT_PRECISION_EQ(zero, v.c.y);
    EXPECT_PRECISION_EQ(zero, v.c.z);
    EXPECT_PRECISION_EQ(zero, v.c.w);
}

TYPED_TEST(XMMContainer2, PointParameterConstructor) {
    using namespace intel;
    typename TypeParam::point v{one, two};
    EXPECT_PRECISION_EQ(one, v.c.x);
    EXPECT_PRECISION_EQ(two, v.c.y);
}

TYPED_TEST(XMMContainer3, PointParameterConstructor) {
    using namespace intel;
    typename TypeParam::point v{one, two, half};
    EXPECT_PRECISION_EQ(one, v.c.x);
    EXPECT_PRECISION_EQ(two, v.c.y);
    EXPECT_PRECISION_EQ(half, v.c.z);
}

TYPED_TEST(XMMContainer4, PointParameterConstructor) {
    using namespace intel;
    typename TypeParam::point v{one, two, half, quarter};
    EXPECT_PRECISION_EQ(one, v.c.x);
    EXPECT_PRECISION_EQ(two, v.c.y);
    EXPECT_PRECISION_EQ(half, v.c.z);
    EXPECT_PRECISION_EQ(quarter, v.c.w);
}

TYPED_TEST(XMMContainer4, PointCopyConstruct) {
    using namespace intel;
    typename TypeParam::point a{eighth, quarter, half, two};
    typename TypeParam::point b{a};
    EXPECT_PRECISION_EQ(eighth, b[0]);
    EXPECT_PRECISION_EQ(quarter, b[1]);
    EXPECT_PRECISION_EQ(half, b[2]);
    EXPECT_PRECISION_EQ(two, b[3]);
}


TYPED_TEST(XMMContainer4, PointCopyAssign) {
    using namespace intel;
    typename TypeParam::point a{eighth, quarter, half, two};
    typename TypeParam::point b = a;
    EXPECT_PRECISION_EQ(eighth, b[0]);
    EXPECT_PRECISION_EQ(quarter, b[1]);
    EXPECT_PRECISION_EQ(half, b[2]);
    EXPECT_PRECISION_EQ(two, b[3]);
}

TYPED_TEST(XMMContainer4, PointMoveAssign) {
    using namespace intel;
    typename TypeParam::point a{eighth, quarter, half, two};
    typename TypeParam::point b = std::move(a);
    EXPECT_PRECISION_EQ(eighth, b[0]);
    EXPECT_PRECISION_EQ(quarter, b[1]);
    EXPECT_PRECISION_EQ(half, b[2]);
    EXPECT_PRECISION_EQ(two, b[3]);
}

TYPED_TEST(XMMContainer2, PointInitializerConstructor) {
    using namespace intel;
    typename TypeParam::point v{{one, two, half, quarter, eighth}};
    EXPECT_PRECISION_EQ(one, v.c.x);
    EXPECT_PRECISION_EQ(two, v.c.y);
}

TYPED_TEST(XMMContainer3, PointInitializerConstructor) {
    using namespace intel;
    typename TypeParam::point v{{one, two, half, quarter, eighth}};
    EXPECT_PRECISION_EQ(one, v.c.x);
    EXPECT_PRECISION_EQ(two, v.c.y);
    EXPECT_PRECISION_EQ(half, v.c.z);
}

TYPED_TEST(XMMContainer4, PointInitializerConstructor) {
    using namespace intel;
    typename TypeParam::point v{{one, two, half, quarter, eighth}};
    EXPECT_PRECISION_EQ(one, v.c.x);
    EXPECT_PRECISION_EQ(two, v.c.y);
    EXPECT_PRECISION_EQ(half, v.c.z);
    EXPECT_PRECISION_EQ(quarter, v.c.w);
}

TYPED_TEST(XMMContainer2, PointIndexing) {
    using namespace intel;
    typename TypeParam::point v{one, -eighth};
    EXPECT_PRECISION_EQ(one, v.c.x);
    EXPECT_PRECISION_EQ(-eighth, v.c.y);

    EXPECT_PRECISION_EQ(v.c.x, v[0]);
    EXPECT_PRECISION_EQ(v.c.y, v[1]);
}

TYPED_TEST(XMMContainer3, PointIndexing) {
    using namespace intel;
    typename TypeParam::point v{one, half, -eighth};
    EXPECT_PRECISION_EQ(one, v.c.x);
    EXPECT_PRECISION_EQ(half, v.c.y);
    EXPECT_PRECISION_EQ(-eighth, v.c.z);

    EXPECT_PRECISION_EQ(v.c.x, v[0]);
    EXPECT_PRECISION_EQ(v.c.y, v[1]);
    EXPECT_PRECISION_EQ(v.c.z, v[2]);
}

TYPED_TEST(XMMContainer4, PointIndexing) {
    using namespace intel;
    typename TypeParam::point v{one, half, -eighth, two};
    EXPECT_PRECISION_EQ(one, v.c.x);
    EXPECT_PRECISION_EQ(half, v.c.y);
    EXPECT_PRECISION_EQ(-eighth, v.c.z);
    EXPECT_PRECISION_EQ(two, v.c.w);

    EXPECT_PRECISION_EQ(v.c.x, v[0]);
    EXPECT_PRECISION_EQ(v.c.y, v[1]);
    EXPECT_PRECISION_EQ(v.c.z, v[2]);
    EXPECT_PRECISION_EQ(v.c.w, v[3]);
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// UNIQUE TESTS
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPED_TEST(XMMContainer4, Floor) {
    using namespace intel;
    typename TypeParam::point a{1.0_p, 5.8_p, 9.2_p, -10.0_p};
    typename TypeParam::point c = floor(a);
    std::cout << c << std::endl;
    EXPECT_PRECISION_EQ(1.0_p, c[0]);
    EXPECT_PRECISION_EQ(5.0_p, c[1]);
    EXPECT_PRECISION_EQ(9.0_p, c[2]);
    EXPECT_PRECISION_EQ(-10.0_p, c[3]);
}

TYPED_TEST(XMMContainer4, Fract) {
    using namespace intel;
    typename TypeParam::point a{1.25_p, 5.75_p, 9.25_p, -10.0625_p};
    typename TypeParam::point c = fract(a);
    std::cout << c << std::endl;
    EXPECT_PRECISION_EQ(0.25_p, c[0]);
    EXPECT_PRECISION_EQ(0.75_p, c[1]);
    EXPECT_PRECISION_EQ(0.25_p, c[2]);
    // this doesn't make sense to me that we would want this value and not -0.0625_p
    EXPECT_PRECISION_EQ(0.9375_p, c[3]);
}
