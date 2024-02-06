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
