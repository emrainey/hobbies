#include "gtest_helper.hpp"

TYPED_TEST(XMMContainer2, VectorDefaultConstruction) {
    using namespace intel;
    typename TypeParam::vector v{};
    EXPECT_PRECISION_EQ(zero, v.c.x);
    EXPECT_PRECISION_EQ(zero, v.c.y);
}

TYPED_TEST(XMMContainer3, VectorDefaultConstruction) {
    using namespace intel;
    typename TypeParam::vector v{};
    EXPECT_PRECISION_EQ(zero, v.c.x);
    EXPECT_PRECISION_EQ(zero, v.c.y);
    EXPECT_PRECISION_EQ(zero, v.c.z);
}

TYPED_TEST(XMMContainer4, VectorDefaultConstruction) {
    using namespace intel;
    typename TypeParam::vector v{};
    EXPECT_PRECISION_EQ(zero, v.c.x);
    EXPECT_PRECISION_EQ(zero, v.c.y);
    EXPECT_PRECISION_EQ(zero, v.c.z);
    EXPECT_PRECISION_EQ(zero, v.c.w);
}

TYPED_TEST(XMMContainer2, VectorParameterConstructor) {
    using namespace intel;
    typename TypeParam::vector v{one, two};
    EXPECT_PRECISION_EQ(one, v.c.x);
    EXPECT_PRECISION_EQ(two, v.c.y);
}

TYPED_TEST(XMMContainer3, VectorParameterConstructor) {
    using namespace intel;
    typename TypeParam::vector v{one, two, half};
    EXPECT_PRECISION_EQ(one, v.c.x);
    EXPECT_PRECISION_EQ(two, v.c.y);
    EXPECT_PRECISION_EQ(half, v.c.z);
}

TYPED_TEST(XMMContainer4, VectorParameterConstructor) {
    using namespace intel;
    typename TypeParam::vector v{one, two, half, quarter};
    EXPECT_PRECISION_EQ(one, v.c.x);
    EXPECT_PRECISION_EQ(two, v.c.y);
    EXPECT_PRECISION_EQ(half, v.c.z);
    EXPECT_PRECISION_EQ(quarter, v.c.w);
}

TYPED_TEST(XMMContainer4, VectorCopyConstruct) {
    using namespace intel;
    typename TypeParam::vector a{eighth, quarter, half, two};
    typename TypeParam::vector b{a};
    EXPECT_PRECISION_EQ(eighth, b[0]);
    EXPECT_PRECISION_EQ(quarter, b[1]);
    EXPECT_PRECISION_EQ(half, b[2]);
    EXPECT_PRECISION_EQ(two, b[3]);
}

TYPED_TEST(XMMContainer4, VectorCopyAssign) {
    using namespace intel;
    typename TypeParam::vector a{eighth, quarter, half, two};
    typename TypeParam::vector b = a;
    EXPECT_PRECISION_EQ(eighth, b[0]);
    EXPECT_PRECISION_EQ(quarter, b[1]);
    EXPECT_PRECISION_EQ(half, b[2]);
    EXPECT_PRECISION_EQ(two, b[3]);
}

TYPED_TEST(XMMContainer4, VectorMoveAssign) {
    using namespace intel;
    typename TypeParam::vector a{eighth, quarter, half, two};
    typename TypeParam::vector b = std::move(a);
    EXPECT_PRECISION_EQ(eighth, b[0]);
    EXPECT_PRECISION_EQ(quarter, b[1]);
    EXPECT_PRECISION_EQ(half, b[2]);
    EXPECT_PRECISION_EQ(two, b[3]);
}

TYPED_TEST(XMMContainer2, VectorInitializerConstructor) {
    using namespace intel;
    typename TypeParam::vector v{{one, two, half, quarter, eighth}};
    EXPECT_PRECISION_EQ(one, v.c.x);
    EXPECT_PRECISION_EQ(two, v.c.y);
}

TYPED_TEST(XMMContainer3, VectorInitializerConstructor) {
    using namespace intel;
    typename TypeParam::vector v{{one, two, half, quarter, eighth}};
    EXPECT_PRECISION_EQ(one, v.c.x);
    EXPECT_PRECISION_EQ(two, v.c.y);
    EXPECT_PRECISION_EQ(half, v.c.z);
}

TYPED_TEST(XMMContainer4, VectorInitializerConstructor) {
    using namespace intel;
    typename TypeParam::vector v{{one, two, half, quarter, eighth}};
    EXPECT_PRECISION_EQ(one, v.c.x);
    EXPECT_PRECISION_EQ(two, v.c.y);
    EXPECT_PRECISION_EQ(half, v.c.z);
    EXPECT_PRECISION_EQ(quarter, v.c.w);
}

TYPED_TEST(XMMContainer2, VectorIndexing) {
    using namespace intel;
    typename TypeParam::vector v{one, -eighth};
    EXPECT_PRECISION_EQ(one, v.c.x);
    EXPECT_PRECISION_EQ(-eighth, v.c.y);

    EXPECT_PRECISION_EQ(v.c.x, v[0]);
    EXPECT_PRECISION_EQ(v.c.y, v[1]);
}

TYPED_TEST(XMMContainer3, VectorIndexing) {
    using namespace intel;
    typename TypeParam::vector v{one, half, -eighth};
    EXPECT_PRECISION_EQ(one, v.c.x);
    EXPECT_PRECISION_EQ(half, v.c.y);
    EXPECT_PRECISION_EQ(-eighth, v.c.z);

    EXPECT_PRECISION_EQ(v.c.x, v[0]);
    EXPECT_PRECISION_EQ(v.c.y, v[1]);
    EXPECT_PRECISION_EQ(v.c.z, v[2]);
}

TYPED_TEST(XMMContainer4, VectorIndexing) {
    using namespace intel;
    typename TypeParam::vector v{one, half, -eighth, two};
    EXPECT_PRECISION_EQ(one, v.c.x);
    EXPECT_PRECISION_EQ(half, v.c.y);
    EXPECT_PRECISION_EQ(-eighth, v.c.z);
    EXPECT_PRECISION_EQ(two, v.c.w);

    EXPECT_PRECISION_EQ(v.c.x, v[0]);
    EXPECT_PRECISION_EQ(v.c.y, v[1]);
    EXPECT_PRECISION_EQ(v.c.z, v[2]);
    EXPECT_PRECISION_EQ(v.c.w, v[3]);
}

TYPED_TEST(XMMContainer3, VectorParallelMult) {
    using namespace intel;
    typename TypeParam::vector a{1.0_p, 5.8_p, 9.2_p};
    typename TypeParam::vector b{14.0_p, 3.6_p, 7.8_p};
    typename TypeParam::vector c = a * b;  // hamard || add
    std::cout << c << std::endl;
    EXPECT_NEAR(14.0_p, c[0], basal::epsilon);
    EXPECT_NEAR(20.88_p, c[1], 1E-5);
    EXPECT_NEAR(71.76_p, c[2], 1E-5);
}

TYPED_TEST(XMMContainer4, VectorParallelMult) {
    using namespace intel;
    typename TypeParam::vector a{1.0_p, 5.8_p, 9.2_p, -10.0_p};
    typename TypeParam::vector b{14.0_p, 3.6_p, 7.8_p, 3.33_p};
    typename TypeParam::vector c = a * b;  // hamard || add
    std::cout << c << std::endl;
    EXPECT_NEAR(14.0_p, c[0], basal::epsilon);
    EXPECT_NEAR(20.88_p, c[1], 1E-5);
    EXPECT_NEAR(71.76_p, c[2], 1E-5);
    EXPECT_NEAR(-33.3_p, c[3], basal::epsilon);
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// UNIQUE TESTS
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPED_TEST(XMMContainer4, InvertVector) {
    using namespace intel;
    typename TypeParam::vector a{1.0_p, 5.8_p, 9.2_p, -10.0_p};
    typename TypeParam::vector c = -a;
    std::cout << a << std::endl;
    EXPECT_NEAR(-1.0_p, c[0], basal::epsilon);
    EXPECT_NEAR(-5.8_p, c[1], basal::epsilon);
    EXPECT_NEAR(-9.2_p, c[2], basal::epsilon);
    EXPECT_NEAR(10.0_p, c[3], basal::epsilon);
}

TYPED_TEST(XMMContainer4, VectorParallelAdd) {
    using namespace intel;
    typename TypeParam::vector a{1.0_p, 5.8_p, 9.2_p, -10.0_p};
    typename TypeParam::vector b{14.0_p, 3.6_p, 7.8_p, 3.33_p};
    typename TypeParam::vector c = a + b;
    std::cout << c << std::endl;
    EXPECT_NEAR(15.0_p, c[0], basal::epsilon);
    EXPECT_NEAR(9.4_p, c[1], basal::epsilon);
    EXPECT_NEAR(17.0_p, c[2], basal::epsilon);
    EXPECT_NEAR(-6.67_p, c[3], basal::epsilon);
}

TYPED_TEST(XMMContainer2, Dot) {
    using namespace intel;
    typename TypeParam::vector v{one, half + quarter};
    typename TypeParam::vector u{half, two};
    EXPECT_PRECISION_EQ(two, dot(u, v));
}

TYPED_TEST(XMMContainer3, Dot) {
    using namespace intel;
    typename TypeParam::vector v{two, half, half + quarter};
    typename TypeParam::vector u{half, one, two};
    EXPECT_PRECISION_EQ(two + one, dot(u, v));
}

TYPED_TEST(XMMContainer4, Dot) {
    using namespace intel;
    typename TypeParam::vector v{one, half, half + quarter, zero};
    typename TypeParam::vector u{half, one, two, one};
    EXPECT_PRECISION_EQ(two + half, dot(u, v));
}

TYPED_TEST(XMMContainer4, DotPerf) {
    using namespace intel;
    typename TypeParam::vector v{1.0_p, 0.4_p, 0.8_p, 0.0_p};
    typename TypeParam::vector u{0.5_p, 1.0_p, 0.4_p, 1.0_p};
    constexpr size_t number_of_ops = 100'000'000;
    auto start = std::chrono::steady_clock::now();
    for (size_t count = 0; count < number_of_ops; /* count ++ */) {
        volatile double d = dot(u, v);
        count += (d > 0 ? 1 : 1);
    }
    std::chrono::duration<double> diff = std::chrono::steady_clock::now() - start;
    std::chrono::duration<double> unit = diff / number_of_ops;
    std::cout << "dot period:" << diff.count() << " sec over " << number_of_ops << " ops. Unit: " << unit.count() << "."
              << std::endl;
    double diff_ns = diff.count() / 1E9;
    double rate_Mops = (1.0_p / unit.count()) / 1E6;
    std::cout << "dots rate:" << rate_Mops << " M-dots/sec, Period: " << diff_ns << " sec" << std::endl;
}

TYPED_TEST(XMMContainer3, Cross) {
    using namespace intel;
    typename TypeParam::vector a{1.0_p, 3.0_p, 7.2_p};
    typename TypeParam::vector b{-4.0_p, 0.9_p, 12};
    typename TypeParam::vector uv = cross(a, b);
    std::cout << "uv " << uv << std::endl;
    EXPECT_NEAR(29.52_p, uv.c.x, basal::epsilon);
    EXPECT_NEAR(-40.8_p, uv.c.y, basal::epsilon);
    EXPECT_NEAR(12.9_p, uv.c.z, basal::epsilon);

    typename TypeParam::vector x{1, 0, 0};
    typename TypeParam::vector y{0, 1, 0};
    typename TypeParam::vector z{0, 0, 1};
    ASSERT_TRUE(z == cross(x, y));
    ASSERT_TRUE(y == cross(z, x));
    ASSERT_TRUE(x == cross(y, z));
}

TYPED_TEST(XMMContainer3, CrossPerf) {
    using namespace intel;
    typename TypeParam::vector a{1.0_p, 3.0_p, 7.2_p};
    typename TypeParam::vector b{-4.0_p, 0.9_p, 12};
    constexpr size_t number_of_ops = 100'000'000;
    auto start = std::chrono::steady_clock::now();
    for (size_t count = 0; count < number_of_ops; /* count++ */) {
        volatile typename TypeParam::vector c = cross(a, b);
        count += (c[0] > 0 ? 1 : 1);
    }
    std::chrono::duration<double> diff = std::chrono::steady_clock::now() - start;
    std::chrono::duration<double> unit = diff / number_of_ops;
    std::cout << "dot period:" << diff.count() << " sec over " << number_of_ops << " ops. Unit: " << unit.count() << "."
              << std::endl;
    double diff_ns = diff.count() / 1E9;
    double rate_Mops = (1.0_p / unit.count()) / 1E6;
    std::cout << "dots rate:" << rate_Mops << " M-dots/sec, Period: " << diff_ns << " sec" << std::endl;
}
