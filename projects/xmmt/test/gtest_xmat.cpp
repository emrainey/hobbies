#include "gtest_helper.hpp"

TEST(XMATTest, ConstructionAndIndexing) {
    using namespace intel;
    xmat_<3, 3> m3 = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    EXPECT_PRECISION_EQ(1, m3.at(0, 0));
    EXPECT_PRECISION_EQ(2, m3.at(0, 1));
    EXPECT_PRECISION_EQ(3, m3.at(0, 2));
    EXPECT_PRECISION_EQ(4, m3.at(1, 0));
    EXPECT_PRECISION_EQ(5, m3.at(1, 1));
    EXPECT_PRECISION_EQ(6, m3.at(1, 2));
    EXPECT_PRECISION_EQ(7, m3.at(2, 0));
    EXPECT_PRECISION_EQ(8, m3.at(2, 1));
    EXPECT_PRECISION_EQ(9, m3.at(2, 2));

    EXPECT_PRECISION_EQ(1, m3[0][0]);
    EXPECT_PRECISION_EQ(2, m3[0][1]);
    EXPECT_PRECISION_EQ(3, m3[0][2]);
    EXPECT_PRECISION_EQ(4, m3[1][0]);
    EXPECT_PRECISION_EQ(5, m3[1][1]);
    EXPECT_PRECISION_EQ(6, m3[1][2]);
    EXPECT_PRECISION_EQ(7, m3[2][0]);
    EXPECT_PRECISION_EQ(8, m3[2][1]);
    EXPECT_PRECISION_EQ(9, m3[2][2]);

    m3.at(0, 0) = 42.0_p;  // assignment
    EXPECT_PRECISION_EQ(42.0_p, m3.at(0, 0));
}

TEST(XMATTest, Operators2x2) {
    using namespace intel;
    xmat_<2, 2> m0 = {1, 2, 3, 4};
    xmat_<2, 2> m1 = {1, 2, 3, 4};

    m1 *= 7.0_p;  // scalar
    for (size_t j = 0; j < m0.rows; j++) {
        for (size_t i = 0; i < m0.cols; i++) {
            EXPECT_PRECISION_EQ(m0.at(j, i) * 7.0_p, m1.at(j, i));
        }
    }
}

TEST(XMATTest, Operators3x3) {
    using namespace intel;
    xmat_<3, 3> m0 = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    xmat_<3, 3> m1 = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    m1 *= 2.0_p;  // scalar
    for (size_t j = 0; j < m0.rows; j++) {
        for (size_t i = 0; i < m0.cols; i++) {
            EXPECT_PRECISION_EQ(m0.at(j, i) * 2.0_p, m1.at(j, i));
        }
    }

    xmat_<3, 3> m2 = m1;
    m2 += m0;
    for (size_t j = 0; j < m0.rows; j++) {
        for (size_t i = 0; i < m0.cols; i++) {
            EXPECT_PRECISION_EQ(m0.at(j, i) + m1.at(j, i), m2.at(j, i));
        }
    }
}

TEST(XMATTest, Operators4x4) {
    using namespace intel;
    xmat_<4, 4> m0 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    xmat_<4, 4> m1 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

    m1 *= 3.0_p;  // scalar
    for (size_t j = 0; j < m0.rows; j++) {
        for (size_t i = 0; i < m0.cols; i++) {
            EXPECT_PRECISION_EQ(m0.at(j, i) * 3.0_p, m1.at(j, i));
        }
    }
}
