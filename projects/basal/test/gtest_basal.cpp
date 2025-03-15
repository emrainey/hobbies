#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <basal/gtest_helper.hpp>

using namespace basal::literals;

TEST(BasalTest, PrecisionLiteral) {
    using namespace basal::literals;
    ASSERT_PRECISION_EQ(0.0_p, 0.0);  // test precision literal
}

TEST(BasalTest, PrecisionEpsilon) {
    ASSERT_LT(
        std::numeric_limits<basal::precision>::epsilon(),
        basal::epsilon);  // our epsilon is larger than std::numeric_limits<basal::precision>::epsilon() on purpose
    // this lets us have larger numbers which can have some precision loss
}

TEST(BasalTest, PrecisionConstants) {
    ASSERT_LE(basal::epsilon, 1.0 / 1024.0);  // At least this small
    ASSERT_PRECISION_EQ(basal::pos_inf, std::numeric_limits<basal::precision>::infinity());
    ASSERT_PRECISION_EQ(basal::neg_inf, -std::numeric_limits<basal::precision>::infinity());
    // ASSERT_PRECISION_EQ(basal::nan, std::numeric_limits<basal::precision>::quiet_NaN()); // this won't be true
}

TEST(BasalTest, PrecisionNearlyEquals) {
    ASSERT_TRUE(basal::nearly_equals(0.0125_p, 0.0124_p, 0.001_p));
    ASSERT_FALSE(basal::nearly_equals(0.0125_p, 0.0124_p, 0.0001_p));
    ASSERT_FALSE(basal::nearly_equals(0.0125_p, 0.0124_p, std::numeric_limits<basal::precision>::epsilon()));
    ASSERT_FALSE(basal::nearly_equals(0.0125_p, 0.0124_p));  // uses default epsilon
    ASSERT_TRUE(basal::nearly_equals(0.0125_p, 0.0125_p));   // uses default epsilon
    basal::precision value = 1.0_p / 3.0_p;
    ASSERT_TRUE(basal::nearly_equals(value * 3.0_p, 1.0_p));
    // 256 is already too large to keep track of the precision of the epsilon
    ASSERT_TRUE(basal::nearly_equals(256.0_p + std::numeric_limits<basal::precision>::epsilon(), 256.0_p));
    ASSERT_TRUE(
        basal::nearly_equals(256.0_p + basal::epsilon,
                             256.0_p));  // our epsilon is larger than std::numeric_limits<basal::precision>::epsilon()
}

TEST(BasalTest, PrecisionNearlyZero) {
    ASSERT_TRUE(basal::nearly_zero(0.0_p));
    ASSERT_TRUE(basal::nearly_zero(-0.0_p));
    ASSERT_TRUE(basal::nearly_zero(std::numeric_limits<basal::precision>::epsilon()));
    ASSERT_FALSE(basal::nearly_zero(basal::epsilon));
    ASSERT_FALSE(basal::nearly_zero(1.0_p));
    ASSERT_FALSE(basal::nearly_zero(-1.0_p));
}

TEST(BasalTest, PrecisionEquivalent) {
    // 256 is already too large to keep track of the precision of the epsilon
    ASSERT_TRUE(basal::equivalent(256.0_p + std::numeric_limits<basal::precision>::epsilon(), 256.0_p));
    // 1 is not too large to keep track of the precision of the epsilon
    ASSERT_FALSE(basal::equivalent(1.0_p + std::numeric_limits<basal::precision>::epsilon(), 1.0_p));
    // 0.5 is not too large to keep track of the precision of the epsilon
    ASSERT_FALSE(basal::equivalent(0.5_p + std::numeric_limits<basal::precision>::epsilon(), 0.5_p));
    // negative zero is equivalent to positive zero
    ASSERT_TRUE(basal::equivalent(0.0_p, -0.0_p));
}

TEST(BasalTest, PrecisionIsExactlyZero) {
    ASSERT_TRUE(basal::is_exactly_zero(0.0_p));
    ASSERT_TRUE(basal::is_exactly_zero(-0.0_p));
    ASSERT_FALSE(basal::is_exactly_zero(std::numeric_limits<basal::precision>::epsilon()));
}

TEST(BasalTest, PrecisionNan) {
    // Only NaN is itself
    ASSERT_TRUE(basal::is_nan(basal::nan));
    ASSERT_FALSE(basal::is_nan(0.0_p));
}

TEST(BasalTest, PrecisionClamp) {
    ASSERT_PRECISION_EQ(basal::clamp(0.0_p, 1.0_p, 2.0_p), 1.0_p);
    ASSERT_PRECISION_EQ(basal::clamp(0.0_p, 3.0_p, 2.0_p), 2.0_p);
    ASSERT_PRECISION_EQ(basal::clamp(1.0_p, -3.0_p, 2.0_p), 1.0_p);
    ASSERT_PRECISION_EQ(basal::clamp(-1.0_p, -4.0_p, 2.0_p), -1.0_p);
}

TEST(BasalTest, PrecisionIsGreaterThanOrEqualToZero) {
    ASSERT_TRUE(basal::is_greater_than_or_equal_to_zero(0.0_p));
    ASSERT_TRUE(basal::is_greater_than_or_equal_to_zero(-0.0_p));
    ASSERT_TRUE(basal::is_greater_than_or_equal_to_zero(std::numeric_limits<basal::precision>::epsilon()));
    ASSERT_TRUE(basal::is_greater_than_or_equal_to_zero(1.0_p));
    ASSERT_TRUE(basal::is_greater_than_or_equal_to_zero(std::numeric_limits<basal::precision>::infinity()));
    ASSERT_FALSE(basal::is_greater_than_or_equal_to_zero(-std::numeric_limits<basal::precision>::epsilon()));
    ASSERT_FALSE(basal::is_greater_than_or_equal_to_zero(-1.0_p));
}

TEST(BasalTest, PrecisionIsLessThanOrEqualToZero) {
    ASSERT_TRUE(basal::is_less_than_or_equal_to_zero(0.0_p));
    ASSERT_TRUE(basal::is_less_than_or_equal_to_zero(-0.0_p));
    ASSERT_TRUE(basal::is_less_than_or_equal_to_zero(-std::numeric_limits<basal::precision>::epsilon()));
    ASSERT_FALSE(basal::is_less_than_or_equal_to_zero(std::numeric_limits<basal::precision>::epsilon()));
    ASSERT_FALSE(basal::is_less_than_or_equal_to_zero(1.0_p));
    ASSERT_TRUE(basal::is_less_than_or_equal_to_zero(-std::numeric_limits<basal::precision>::infinity()));
}

TEST(BasalTest, ThrowAndCatch) {
    bool must_set = false;
    basal::exit_if_not_thrown("Must throw exception", [&](void) -> void {
        must_set = true;
        basal::exception::throw_unless(must_set == false, __FILE__, __LINE__);
    });
    ASSERT_TRUE(must_set);
    must_set = false;
    basal::exit_if_not_thrown("Must throw exception", [&](void) -> void {
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

class DebuggableFixture
    : public ::testing::Test
    , public basal::debuggable {
public:
    DebuggableFixture() : debuggable{} {
        // constructor
    }

    ~DebuggableFixture() = default;

    void SetUp() override {
        // setup code
        ASSERT_EQ(0u, mask().all);  // ensure all zones are clear
    }
    void TearDown() override {
        // teardown code
        mask().all = 0u;  // clear all zones
    }
};

TEST_F(DebuggableFixture, Empty) {
}

TEST_F(DebuggableFixture, Mask) {
    // set the mask to all zones
    mask().all = 0xFFFFFFFF;
    ASSERT_EQ(0xFFFFFFFFu, mask().all);
    ASSERT_TRUE(mask().zones.priority);
    ASSERT_TRUE(mask().zones.fatal);
    ASSERT_TRUE(mask().zones.error);
    ASSERT_TRUE(mask().zones.warn);
    ASSERT_TRUE(mask().zones.api);
    ASSERT_TRUE(mask().zones.info);
    mask().zones.info = 0;  // clear the info zone
    ASSERT_FALSE(mask().zones.info);
}

TEST_F(DebuggableFixture, Emit) {
    // emit a message
    emit("Hello World");
}