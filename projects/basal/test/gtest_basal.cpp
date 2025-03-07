#include <gtest/gtest.h>

#include <basal/basal.hpp>

using namespace basal::literals;

TEST(BasalTest, FloatCheck) {
    ASSERT_TRUE(basal::nearly_equals(0.0125_p, 0.0124_p, 0.001_p));
    ASSERT_FALSE(basal::nearly_equals(0.0125_p, 0.0124_p, 0.0001_p));
    ASSERT_FALSE(basal::nearly_equals(0.0125_p, 0.0124_p));
    ASSERT_TRUE(basal::nearly_equals(0.0125_p, 0.0125_p));
    basal::precision value = 1.0_p / 3.0_p;
    ASSERT_TRUE(basal::nearly_equals(value * 3.0_p, 1.0_p));
}

TEST(BasalTest, ThrowAndCatch) {
    bool must_set = false;
    basal::assert_if_not_thrown("Must throw exception", [&](void) -> void {
        must_set = true;
        basal::exception::throw_unless(must_set == false, __FILE__, __LINE__);
    });
    ASSERT_TRUE(must_set);
    must_set = false;
    basal::assert_if_not_thrown("Must throw exception", [&](void) -> void {
        must_set = true;
        basal::exception::throw_if(must_set == true, __FILE__, __LINE__);
    });
    ASSERT_TRUE(must_set);
}

TEST(BasalTest, RandRange) {
    for (size_t i = 0; i < 1000; i++) {
        basal::precision value = basal::rand_range(-0.847_p, 1.497_p);
        ASSERT_LE(value, 1.497_p);
        ASSERT_GE(value, -0.847_p);
    }
}

TEST(BasalTest, HalfConstants) {
    using basal::half;
    using namespace basal::half_constants;
    ASSERT_TRUE(positive_zero == negative_zero);
    ASSERT_FALSE(positive_zero.is_exactly(negative_zero));
    ASSERT_TRUE(positive_zero.is_zero());
    ASSERT_TRUE(negative_zero.is_zero());

    ASSERT_TRUE(positive_nan == negative_nan);
    ASSERT_TRUE(positive_nan.is_nan());
    ASSERT_TRUE(negative_nan.is_nan());
    ASSERT_FALSE(positive_nan.is_exactly(negative_nan));

    half rando{0b1, 0b1'1111, 0b00'0000'0011};  // rando nan
    ASSERT_TRUE(rando.is_nan());

    ASSERT_TRUE(positive_infinity != negative_infinity);
    ASSERT_TRUE(positive_infinity.is_inf());
    ASSERT_TRUE(negative_infinity.is_inf());

    half ep{0.00097608566f};
    ASSERT_FLOAT_EQ(0.00097608566f, float(epsilon)) << ep;
}

TEST(BasalTest, FloatToHalfToFloat) {
    using basal::half;
    using namespace basal::half_literals;
    ASSERT_FLOAT_EQ(4.0f, float(half{4.0f}));
    ASSERT_FLOAT_EQ(2.0f, float(half{2.0f}));
    ASSERT_FLOAT_EQ(1.0f, float(half{1.0f}));
    ASSERT_FLOAT_EQ(0.0f, float(half{0.0f}));
    ASSERT_FLOAT_EQ(-0.0f, float(half{-0.0f}));
    ASSERT_FLOAT_EQ(0.5f, float(half{0.5f}));
    ASSERT_FLOAT_EQ(0.25f, float(half{0.25f}));
    ASSERT_FLOAT_EQ(0.125f, float(half{0.125f}));

    ASSERT_FLOAT_EQ(std::numeric_limits<float>::infinity(), float(half{123'028'302.0f}))
        << "large numbers should become inf";
    ASSERT_FLOAT_EQ(std::numeric_limits<float>::infinity(), float(half{std::numeric_limits<float>::infinity()}));
    ASSERT_FLOAT_EQ(-std::numeric_limits<float>::infinity(), float(half{-std::numeric_limits<float>::infinity()}));
    ASSERT_TRUE(std::isnan(float(half{std::numeric_limits<float>::quiet_NaN()})));

    ASSERT_FLOAT_EQ(0.99951172f, float(0.99951172_hf));
}