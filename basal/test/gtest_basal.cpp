#include <gtest/gtest.h>

#include <basal/basal.hpp>

TEST(BasalTest, FloatCheck) {
    ASSERT_TRUE(basal::equals(0.0125, 0.0124, 0.001));
    ASSERT_FALSE(basal::equals(0.0125, 0.0124, 0.0001));
    ASSERT_FALSE(basal::equals(0.0125, 0.0124));
    ASSERT_TRUE(basal::equals(0.0125, 0.0125));
    double value = 1.0 / 3.0;
    ASSERT_TRUE(basal::equals(value * 3.0, 1.0));
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
        double value = basal::rand_range(-0.847, 1.497);
        ASSERT_LE(value, 1.497);
        ASSERT_GE(value, -0.847);
    }
}
