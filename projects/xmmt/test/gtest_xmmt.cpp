#include <gtest/gtest.h>

#include <chrono>
#include <vector>

#include "xmmt/xmat.hpp"
#include "xmmt/xmmt.hpp"

TEST(XMMTTest, Indexing) {
    using namespace intel;
    point4 v{{1.0, 0.4, -0.8, 3.14}};
    EXPECT_DOUBLE_EQ(1.0, v.c.x);
    EXPECT_DOUBLE_EQ(0.4, v.c.y);
    EXPECT_DOUBLE_EQ(-0.8, v.c.z);
    EXPECT_DOUBLE_EQ(3.14, v.c.w);

    EXPECT_DOUBLE_EQ(v.c.x, v[0]);
    EXPECT_DOUBLE_EQ(v.c.y, v[1]);
    EXPECT_DOUBLE_EQ(v.c.z, v[2]);
    EXPECT_DOUBLE_EQ(v.c.w, v[3]);
}

TEST(XMMTTest, Dot) {
    using namespace intel;
    vector4 v{{1.0, 0.4, 0.8, 0.0}};
    vector4 u{{0.5, 1.0, 0.4, 1.0}};
    EXPECT_DOUBLE_EQ(1.22, dot(u, v));
}

TEST(XMMTTest, DotPerf) {
    using namespace intel;
    vector4 v{{1.0, 0.4, 0.8, 0.0}};
    vector4 u{{0.5, 1.0, 0.4, 1.0}};
    constexpr size_t number_of_ops = 100'000'000;
    auto start = std::chrono::steady_clock::now();
    for (size_t count = 0; count < number_of_ops; /* count ++ */) {
        volatile double d = dot(u, v);
        count += (d > 0 ? 1 : 1);
    }
    auto diff = std::chrono::steady_clock::now() - start;
    double rate = (double(number_of_ops) / (diff.count() / 1E9)) / 1E6;
    std::cout << "dots rate:" << rate << " M-dots/sec, Period: " << diff.count() / 1E9 << " sec" << std::endl;
}

TEST(XMMTTest, Cross) {
    using namespace intel;
    vector3 a{{1.0, 3.0, 7.2}};
    vector3 b{{-4.0, 0.9, 12}};
    vector3 uv = cross(a, b);
    std::cout << "uv " << uv << std::endl;
    EXPECT_DOUBLE_EQ(29.52, uv.c.x);
    EXPECT_DOUBLE_EQ(-40.8, uv.c.y);
    EXPECT_DOUBLE_EQ(12.9, uv.c.z);

    vector3 x{{1, 0, 0}};
    vector3 y{{0, 1, 0}};
    vector3 z{{0, 0, 1}};
    ASSERT_TRUE(z == cross(x, y));
    ASSERT_TRUE(y == cross(z, x));
    ASSERT_TRUE(x == cross(y, z));
}

TEST(XMMTTest, CrossPerf) {
    using namespace intel;
    vector3 a{{1.0, 3.0, 7.2}};
    vector3 b{{-4.0, 0.9, 12}};
    constexpr size_t number_of_ops = 100'000'000;
    auto start = std::chrono::steady_clock::now();
    for (size_t count = 0; count < number_of_ops; /* count++ */) {
        volatile vector3 c = cross(a, b);
        count += (c[0] > 0 ? 1 : 1);
    }
    auto diff = std::chrono::steady_clock::now() - start;
    double rate = (double(number_of_ops) / (diff.count() / 1E9)) / 1E6;
    std::cout << "cross rate:" << rate << " M-cross/sec, Period: " << diff.count() / 1E9 << " sec" << std::endl;
}

TEST(XMMTTest, VectorFromPoints) {
    using namespace intel;
    point4 a{{1.0, 5.8, 9.2, -10.0}};
    point4 b{{14.0, 3.6, 7.8, 3.33}};
    vector4 c = a - b;
    std::cout << c << std::endl;
    EXPECT_DOUBLE_EQ(-13.0, c[0]);
    EXPECT_DOUBLE_EQ(2.2, c[1]);
    EXPECT_DOUBLE_EQ(1.4, c[2]);
    EXPECT_DOUBLE_EQ(-13.33, c[3]);
}

TEST(XMMTTest, PointPlusVector) {
    using namespace intel;
    point4 a{{1.0, 5.8, 9.2, -10.0}};
    vector4 b{{14.0, 3.6, 7.8, 3.33}};
    point4 c = a + b;
    std::cout << c << std::endl;
    EXPECT_DOUBLE_EQ(15.0, c[0]);
    EXPECT_DOUBLE_EQ(9.4, c[1]);
    EXPECT_DOUBLE_EQ(17.0, c[2]);
    EXPECT_DOUBLE_EQ(-6.67, c[3]);
}

TEST(XMMTTest, PointAccumVector) {
    using namespace intel;
    point4 a{{1.0, 5.8, 9.2, -10.0}};
    vector4 b{{14.0, 3.6, 7.8, 3.33}};
    a += b;
    std::cout << a << std::endl;
    EXPECT_DOUBLE_EQ(15.0, a[0]);
    EXPECT_DOUBLE_EQ(9.4, a[1]);
    EXPECT_DOUBLE_EQ(17.0, a[2]);
    EXPECT_DOUBLE_EQ(-6.67, a[3]);
}

TEST(XMMTTest, PointDeaccumVector) {
    using namespace intel;
    point4 a{{1.0, 5.8, 9.2, -10.0}};
    vector4 b{{14.0, 3.6, 7.8, 3.33}};
    a -= b;
    std::cout << a << std::endl;
    EXPECT_DOUBLE_EQ(-13.0, a[0]);
    EXPECT_DOUBLE_EQ(2.2, a[1]);
    EXPECT_DOUBLE_EQ(1.4, a[2]);
    EXPECT_DOUBLE_EQ(-13.33, a[3]);
}

TEST(XMMTTest, InvertVector) {
    using namespace intel;
    vector4 a{{1.0, 5.8, 9.2, -10.0}};
    vector4 c = -a;
    std::cout << a << std::endl;
    EXPECT_DOUBLE_EQ(-1.0, c[0]);
    EXPECT_DOUBLE_EQ(-5.8, c[1]);
    EXPECT_DOUBLE_EQ(-9.2, c[2]);
    EXPECT_DOUBLE_EQ(10.0, c[3]);
}

TEST(XMMTTest, ParallelAdd) {
    using namespace intel;
    vector4 a{{1.0, 5.8, 9.2, -10.0}};
    vector4 b{{14.0, 3.6, 7.8, 3.33}};
    vector4 c = a + b;
    std::cout << c << std::endl;
    EXPECT_DOUBLE_EQ(15.0, c[0]);
    EXPECT_DOUBLE_EQ(9.4, c[1]);
    EXPECT_DOUBLE_EQ(17.0, c[2]);
    EXPECT_DOUBLE_EQ(-6.67, c[3]);
}

TEST(XMMTTest, ParallelMult) {
    using namespace intel;
    vector4 a{{1.0, 5.8, 9.2, -10.0}};
    vector4 b{{14.0, 3.6, 7.8, 3.33}};
    vector4 c = a * b;  // hamard || add
    std::cout << c << std::endl;
    EXPECT_DOUBLE_EQ(14.0, c[0]);
    EXPECT_DOUBLE_EQ(20.88, c[1]);
    EXPECT_DOUBLE_EQ(71.76, c[2]);
    EXPECT_DOUBLE_EQ(-33.3, c[3]);
}

TEST(XMATTest, ConstructionAndIndexing) {
    using namespace intel;
    xmat_<3, 3> m3 = {{1, 2, 3, 4, 5, 6, 7, 8, 9}};
    EXPECT_DOUBLE_EQ(1, m3.at(0, 0));
    EXPECT_DOUBLE_EQ(2, m3.at(0, 1));
    EXPECT_DOUBLE_EQ(3, m3.at(0, 2));
    EXPECT_DOUBLE_EQ(4, m3.at(1, 0));
    EXPECT_DOUBLE_EQ(5, m3.at(1, 1));
    EXPECT_DOUBLE_EQ(6, m3.at(1, 2));
    EXPECT_DOUBLE_EQ(7, m3.at(2, 0));
    EXPECT_DOUBLE_EQ(8, m3.at(2, 1));
    EXPECT_DOUBLE_EQ(9, m3.at(2, 2));

    EXPECT_DOUBLE_EQ(1, m3[0][0]);
    EXPECT_DOUBLE_EQ(2, m3[0][1]);
    EXPECT_DOUBLE_EQ(3, m3[0][2]);
    EXPECT_DOUBLE_EQ(4, m3[1][0]);
    EXPECT_DOUBLE_EQ(5, m3[1][1]);
    EXPECT_DOUBLE_EQ(6, m3[1][2]);
    EXPECT_DOUBLE_EQ(7, m3[2][0]);
    EXPECT_DOUBLE_EQ(8, m3[2][1]);
    EXPECT_DOUBLE_EQ(9, m3[2][2]);

    m3.at(0, 0) = 42.0;  // assignment
    EXPECT_DOUBLE_EQ(42.0, m3.at(0, 0));
}

TEST(XMATTest, Operators2x2) {
    using namespace intel;
    xmat_<2, 2> m0 = {{1, 2, 3, 4}};
    xmat_<2, 2> m1 = {{1, 2, 3, 4}};

    m1 *= 7.0;  // scalar
    for (size_t j = 0; j < m0.rows; j++) {
        for (size_t i = 0; i < m0.cols; i++) {
            EXPECT_DOUBLE_EQ(m0.at(j, i) * 7.0, m1.at(j, i));
        }
    }
}

TEST(XMATTest, Operators3x3) {
    using namespace intel;
    xmat_<3, 3> m0 = {{1, 2, 3, 4, 5, 6, 7, 8, 9}};
    xmat_<3, 3> m1 = {{1, 2, 3, 4, 5, 6, 7, 8, 9}};

    m1 *= 2.0;  // scalar
    for (size_t j = 0; j < m0.rows; j++) {
        for (size_t i = 0; i < m0.cols; i++) {
            EXPECT_DOUBLE_EQ(m0.at(j, i) * 2.0, m1.at(j, i));
        }
    }

    xmat_<3, 3> m2 = m1;
    m2 += m0;
    for (size_t j = 0; j < m0.rows; j++) {
        for (size_t i = 0; i < m0.cols; i++) {
            EXPECT_DOUBLE_EQ(m0.at(j, i) + m1.at(j, i), m2.at(j, i));
        }
    }
}

TEST(XMATTest, Operators4x4) {
    using namespace intel;
    xmat_<4, 4> m0 = {{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}};
    xmat_<4, 4> m1 = {{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}};

    m1 *= 3.0;  // scalar
    for (size_t j = 0; j < m0.rows; j++) {
        for (size_t i = 0; i < m0.cols; i++) {
            EXPECT_DOUBLE_EQ(m0.at(j, i) * 3.0, m1.at(j, i));
        }
    }
}

TEST(XMMTTest, ForwardingConstructors) {
    using namespace intel;
    {
        point2 blah{0.25, 0.75};
        EXPECT_EQ(0.25, blah.c.x);
        EXPECT_EQ(0.75, blah.c.y);
    }
    {
        point3 blah{0.25, 0.75, 0.5};
        EXPECT_EQ(0.25, blah.c.x);
        EXPECT_EQ(0.75, blah.c.y);
        EXPECT_EQ(0.50, blah.c.z);
    }
    {
        point4 blah{0.25, 0.75, 0.5, 0.0625};
        EXPECT_EQ(0.25, blah.c.x);
        EXPECT_EQ(0.75, blah.c.y);
        EXPECT_EQ(0.50, blah.c.z);
        EXPECT_EQ(0.0625, blah.c.w);
    }
}

TEST(XMMTTest, Floor) {
    using namespace intel;
    vector4 a{{1.0, 5.8, 9.2, -10.0}};
    vector4 c = a.floor();
    std::cout << c << std::endl;
    EXPECT_DOUBLE_EQ(1.0, c[0]);
    EXPECT_DOUBLE_EQ(5.0, c[1]);
    EXPECT_DOUBLE_EQ(9.0, c[2]);
    EXPECT_DOUBLE_EQ(-10.0, c[3]);
}

TEST(XMMTTest, Fract) {
    using namespace intel;
    vector4 a{{1.25, 5.75, 9.25, -10.0625}};
    vector4 c = a.fract();
    std::cout << c << std::endl;
    EXPECT_DOUBLE_EQ(0.25, c[0]);
    EXPECT_DOUBLE_EQ(0.75, c[1]);
    EXPECT_DOUBLE_EQ(0.25, c[2]);
    // this doesn't make sense to me that we would want this value and not -0.0625
    EXPECT_DOUBLE_EQ(0.9375, c[3]);
}