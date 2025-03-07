#include "gtest_helper.hpp"

TYPED_TEST(XMMContainer4, VectorFromPoints) {
    using namespace intel;
    typename TypeParam::point a{1.0_p, 5.8_p, 9.2_p, -10.0_p};
    typename TypeParam::point b{14.0_p, 3.6_p, 7.8_p, 3.33_p};
    typename TypeParam::vector c = a - b;
    std::cout << c << std::endl;
    EXPECT_NEAR(-13.0_p, c[0], basal::epsilon);
    EXPECT_NEAR(2.2_p, c[1], basal::epsilon);
    EXPECT_NEAR(1.4_p, c[2], basal::epsilon);
    EXPECT_NEAR(-13.33_p, c[3], basal::epsilon);
}

TYPED_TEST(XMMContainer4, PointPlusVector) {
    using namespace intel;
    typename TypeParam::point a{1.0_p, 5.8_p, 9.2_p, -10.0_p};
    typename TypeParam::vector b{14.0_p, 3.6_p, 7.8_p, 3.33_p};
    typename TypeParam::point c = a + b;
    std::cout << c << std::endl;
    EXPECT_NEAR(15.0_p, c[0], basal::epsilon);
    EXPECT_NEAR(9.4_p, c[1], basal::epsilon);
    EXPECT_NEAR(17.0_p, c[2], basal::epsilon);
    EXPECT_NEAR(-6.67_p, c[3], basal::epsilon);
}

TYPED_TEST(XMMContainer4, PointAccumVector) {
    using namespace intel;
    typename TypeParam::point a{1.0_p, 5.8_p, 9.2_p, -10.0_p};
    typename TypeParam::vector b{14.0_p, 3.6_p, 7.8_p, 3.33_p};
    a += b;
    std::cout << a << std::endl;
    EXPECT_NEAR(15.0_p, a[0], basal::epsilon);
    EXPECT_NEAR(9.4_p, a[1], basal::epsilon);
    EXPECT_NEAR(17.0_p, a[2], basal::epsilon);
    EXPECT_NEAR(-6.67_p, a[3], basal::epsilon);
}

TYPED_TEST(XMMContainer4, PointDeaccumVector) {
    using namespace intel;
    typename TypeParam::point a{1.0_p, 5.8_p, 9.2_p, -10.0_p};
    typename TypeParam::vector b{14.0_p, 3.6_p, 7.8_p, 3.33_p};
    a -= b;
    std::cout << a << std::endl;
    EXPECT_NEAR(-13.0_p, a[0], basal::epsilon);
    EXPECT_NEAR(2.2_p, a[1], basal::epsilon);
    EXPECT_NEAR(1.4_p, a[2], basal::epsilon);
    EXPECT_NEAR(-13.33_p, a[3], basal::epsilon);
}
