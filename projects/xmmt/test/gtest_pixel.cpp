#include "gtest_helper.hpp"

TYPED_TEST(XMMContainer3, PixelDefaultConstruction) {
    using namespace intel;
    typename TypeParam::pixel v{};
    EXPECT_PRECISION_EQ(zero, v.c.x);
    EXPECT_PRECISION_EQ(zero, v.c.y);
    EXPECT_PRECISION_EQ(zero, v.c.z);
}

TYPED_TEST(XMMContainer4, PixelDefaultConstruction) {
    using namespace intel;
    typename TypeParam::pixel v{};
    EXPECT_PRECISION_EQ(zero, v.c.x);
    EXPECT_PRECISION_EQ(zero, v.c.y);
    EXPECT_PRECISION_EQ(zero, v.c.z);
    EXPECT_PRECISION_EQ(zero, v.c.w);
}

TYPED_TEST(XMMContainer3, PixelParameterConstructor) {
    using namespace intel;
    typename TypeParam::pixel v{one, two, half};
    EXPECT_PRECISION_EQ(one, v.c.x);
    EXPECT_PRECISION_EQ(two, v.c.y);
    EXPECT_PRECISION_EQ(half, v.c.z);
}

TYPED_TEST(XMMContainer4, PixelParameterConstructor) {
    using namespace intel;
    typename TypeParam::pixel v{one, two, half, quarter};
    EXPECT_PRECISION_EQ(one, v.c.x);
    EXPECT_PRECISION_EQ(two, v.c.y);
    EXPECT_PRECISION_EQ(half, v.c.z);
    EXPECT_PRECISION_EQ(quarter, v.c.w);
}

TYPED_TEST(XMMContainer4, PixelCopyConstruct) {
    using namespace intel;
    typename TypeParam::pixel a{eighth, quarter, half, two};
    typename TypeParam::pixel b{a};
    EXPECT_PRECISION_EQ(eighth, b[0]);
    EXPECT_PRECISION_EQ(quarter, b[1]);
    EXPECT_PRECISION_EQ(half, b[2]);
    EXPECT_PRECISION_EQ(two, b[3]);
}

TYPED_TEST(XMMContainer4, PixelCopyAssign) {
    using namespace intel;
    typename TypeParam::pixel a{eighth, quarter, half, two};
    typename TypeParam::pixel b = a;
    EXPECT_PRECISION_EQ(eighth, b[0]);
    EXPECT_PRECISION_EQ(quarter, b[1]);
    EXPECT_PRECISION_EQ(half, b[2]);
    EXPECT_PRECISION_EQ(two, b[3]);
}

TYPED_TEST(XMMContainer4, PixelMoveAssign) {
    using namespace intel;
    typename TypeParam::pixel a{eighth, quarter, half, two};
    typename TypeParam::pixel b = std::move(a);
    EXPECT_PRECISION_EQ(eighth, b[0]);
    EXPECT_PRECISION_EQ(quarter, b[1]);
    EXPECT_PRECISION_EQ(half, b[2]);
    EXPECT_PRECISION_EQ(two, b[3]);
}

TYPED_TEST(XMMContainer3, PixelIndexing) {
    using namespace intel;
    typename TypeParam::pixel v{one, half, -eighth};
    EXPECT_PRECISION_EQ(one, v.c.x);
    EXPECT_PRECISION_EQ(half, v.c.y);
    EXPECT_PRECISION_EQ(-eighth, v.c.z);

    EXPECT_PRECISION_EQ(v.c.x, v[0]);
    EXPECT_PRECISION_EQ(v.c.y, v[1]);
    EXPECT_PRECISION_EQ(v.c.z, v[2]);
}

TYPED_TEST(XMMContainer4, PixelIndexing) {
    using namespace intel;
    typename TypeParam::pixel v{one, half, -eighth, two};
    EXPECT_PRECISION_EQ(one, v.c.x);
    EXPECT_PRECISION_EQ(half, v.c.y);
    EXPECT_PRECISION_EQ(-eighth, v.c.z);
    EXPECT_PRECISION_EQ(two, v.c.w);

    EXPECT_PRECISION_EQ(v.c.x, v[0]);
    EXPECT_PRECISION_EQ(v.c.y, v[1]);
    EXPECT_PRECISION_EQ(v.c.z, v[2]);
    EXPECT_PRECISION_EQ(v.c.w, v[3]);
}

TYPED_TEST(XMMContainer3, PixelParallelMult) {
    using namespace intel;
    typename TypeParam::pixel a{half, quarter, two};
    typename TypeParam::pixel b{quarter, quarter, zero};
    typename TypeParam::pixel c = a * b;  // hamard || add
    std::cout << c << std::endl;
    EXPECT_NEAR(eighth, c[0], basal::epsilon);
    EXPECT_NEAR(sixteenth, c[1], 1E-5);
    EXPECT_NEAR(zero, c[2], 1E-5);
}

TYPED_TEST(XMMContainer4, PixelParallelMult) {
    using namespace intel;
    typename TypeParam::pixel a{half, quarter, two, eighth};
    typename TypeParam::pixel b{quarter, quarter, zero, two};
    typename TypeParam::pixel c = a * b;  // hamard || add
    std::cout << c << std::endl;
    EXPECT_NEAR(eighth, c[0], basal::epsilon);
    EXPECT_NEAR(sixteenth, c[1], 1E-5);
    EXPECT_NEAR(zero, c[2], 1E-5);
    EXPECT_NEAR(quarter, c[3], basal::epsilon);
}

TYPED_TEST(XMMContainer3, PixelAccumulate) {
    using namespace intel;
    typename TypeParam::pixel a{half, quarter, two};
    typename TypeParam::pixel b{quarter, quarter, zero};
    a += b;
    std::cout << a << std::endl;
    EXPECT_PRECISION_EQ(half + quarter, a[0]);
    EXPECT_PRECISION_EQ(half, a[1]);
    EXPECT_PRECISION_EQ(two, a[2]);
}

TYPED_TEST(XMMContainer4, PixelAccumulate) {
    using namespace intel;
    typename TypeParam::pixel a{half, quarter, two, eighth};
    typename TypeParam::pixel b{quarter, quarter, zero, two};
    a += b;
    std::cout << a << std::endl;
    EXPECT_PRECISION_EQ(half + quarter, a[0]);
    EXPECT_PRECISION_EQ(half, a[1]);
    EXPECT_PRECISION_EQ(two, a[2]);
    EXPECT_PRECISION_EQ(two + eighth, a[3]);
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// UNIQUE TESTS
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPED_TEST(XMMContainer4, PixelBlend) {
    using namespace intel;
    typename TypeParam::pixel a{0.75_p, 0.25_p, 0.5_p, 0.0625_p};
    typename TypeParam::pixel b{0.25_p, 0.75_p, 0.0_p, 0.0625_p};
    typename TypeParam::pixel c = interpolate(a, b, 0.5_p);
    EXPECT_PRECISION_EQ(0.5_p, c[0]);
    EXPECT_PRECISION_EQ(0.5_p, c[1]);
    EXPECT_PRECISION_EQ(0.25_p, c[2]);
    EXPECT_PRECISION_EQ(0.0625_p, c[3]);
}

TYPED_TEST(XMMContainer3, PixelClamp) {
    using namespace intel;
    typename TypeParam::pixel a{1.75_p, 0.25_p, -0.5_p};
    a.clamp();
    EXPECT_PRECISION_EQ(1.0_p, a[0]);
    EXPECT_PRECISION_EQ(0.25_p, a[1]);
    EXPECT_PRECISION_EQ(0.0_p, a[2]);
}

TYPED_TEST(XMMContainer4, PixelClamp) {
    using namespace intel;
    typename TypeParam::pixel a{1.75_p, 0.25_p, 0.5_p, -0.0625_p};
    a.clamp();
    EXPECT_PRECISION_EQ(1.0_p, a[0]);
    EXPECT_PRECISION_EQ(0.25_p, a[1]);
    EXPECT_PRECISION_EQ(0.5_p, a[2]);
    EXPECT_PRECISION_EQ(0.0_p, a[3]);
}
