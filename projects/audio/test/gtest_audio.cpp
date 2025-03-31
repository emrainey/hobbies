#include <gtest/gtest.h>

#include <audio/audio.hpp>
#include <basal/gtest_helper.hpp>

using namespace basal::literals;

TEST(AudioTest, Empty) {
    audio::StereoPCM seq{44'100.0_p, 3 * 44'100U};
    EXPECT_EQ(seq.count(), 132'300U);
}