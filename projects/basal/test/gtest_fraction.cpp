#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <basal/gtest_helper.hpp>

TEST(FractionTest, ClassScalarArthimetic) {
    using basal::fraction;
    fraction f{3, 8U};
    f += 1;
    std::cout << f << std::endl;
    ASSERT_EQ(f, fraction(11, 8U));
    f -= 2;
    std::cout << f << std::endl;
    ASSERT_EQ(f, fraction(-5, 8U));
    f *= 4;
    std::cout << f << std::endl;
    ASSERT_EQ(f, fraction(-20, 8U));
    f /= 5;
    std::cout << f << std::endl;
    ASSERT_EQ(f, fraction(-20, 40U));
}

TEST(FractionTest, ClassFractionArithmetic) {
    using basal::fraction;
    fraction f{3, 8U};
    fraction g{1, 2U};
    f += g;
    std::cout << f << std::endl;
    ASSERT_EQ(f, fraction(7, 8U));
    f -= g;
    std::cout << f << std::endl;
    ASSERT_EQ(f, fraction(3, 8U));
    f *= g;
    std::cout << f << std::endl;
    ASSERT_EQ(f, fraction(3, 16U));
    f /= g;
    std::cout << f << std::endl;
    ASSERT_EQ(f, fraction(3, 8U));
}

TEST(FractionTest, FriendScalarArithmetic) {
    using basal::fraction;
    fraction f{3, 8U};
    f = f + 1;
    std::cout << f << std::endl;
    ASSERT_EQ(f, fraction(11, 8U));
    f = f - 2;
    std::cout << f << std::endl;
    ASSERT_EQ(f, fraction(-5, 8U));
    f = f * 4;
    std::cout << f << std::endl;
    ASSERT_EQ(f, fraction(-20, 8U));
    f = f / 5;
    std::cout << f << std::endl;
    ASSERT_EQ(f, fraction(-4, 8U));
}

TEST(FractionTest, FriendFractionArithmetic) {
    using basal::fraction;
    fraction f{3, 8U};
    fraction g{1, 2U};
    f = f + g;
    std::cout << f << std::endl;
    ASSERT_EQ(f, fraction(7, 8U));
    f = f - g;
    std::cout << f << std::endl;
    ASSERT_EQ(f, fraction(3, 8U));
    f = f * g;
    std::cout << f << std::endl;
    ASSERT_EQ(f, fraction(3, 16U));
    f = f / g;
    std::cout << f << std::endl;
    ASSERT_EQ(f, fraction(3, 8U));
}

TEST(FractionTest, Comparison) {
    using basal::fraction;
    fraction f{3, 8U};
    ASSERT_TRUE(f < 1);
    ASSERT_TRUE(f < fraction(1, 2U));
    ASSERT_TRUE(f <= 1);
    ASSERT_TRUE(f <= fraction(3, 8U));
    ASSERT_FALSE(f > 1);
    ASSERT_FALSE(f > fraction(1, 2U));
    ASSERT_FALSE(f >= 1);
    ASSERT_TRUE(f >= fraction(3, 8U));
    ASSERT_TRUE(f == fraction(3, 8U));
    ASSERT_FALSE(f != fraction(3, 8U));
}

TEST(FractionTest, Conversion) {
    using basal::fraction;
    fraction f{355, 113U};  // this is an approximation of pi from antiquity
    auto g = f.as<float>();
    auto h = f.as<int>();
    ASSERT_FLOAT_EQ(g, 3.14159292f);
    ASSERT_EQ(h, 3);
}
