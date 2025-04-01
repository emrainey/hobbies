#include <gtest/gtest.h>

#include <audio/audio.hpp>
#include <basal/gtest_helper.hpp>

using namespace basal::literals;

TEST(AudioTest, Empty) {
    audio::StereoPCM seq{44'100.0_p, 3 * 44'100U};
    EXPECT_EQ(seq.count(), 132'300U);
}

TEST(AudioTest, WhenSavingMonoPCM) {
    basal::precision sample_rate = 44'100.0_p;

    basal::precision increment = 1.0_p / sample_rate;
    basal::precision period = 3.0_p;
    size_t number_of_samples = static_cast<size_t>(period * sample_rate);
    audio::MonoPCM seq{sample_rate, number_of_samples};
    seq.for_each([](size_t channel, size_t index, basal::precision offset) -> audio::MonoPCM::sample {
        basal::precision v = (0.5_p * sin(2.0_p * M_PI * 440.0_p * offset));  // "A" 440.00Hz
        return std::round(v * INT16_MAX);
    });
    EXPECT_TRUE(seq.save("mono_a.wav"));
}

TEST(AudioTest, WhenSavingStereoPCM) {
    basal::precision sample_rate = audio::specification::cd_sample_rate;
    basal::precision increment = 1.0_p / sample_rate;
    basal::precision period = 3.0_p;
    size_t number_of_samples = static_cast<size_t>(period * sample_rate);
    audio::StereoPCM seq{sample_rate, number_of_samples};
    seq.for_each([](size_t channel, size_t index, basal::precision offset) -> audio::StereoPCM::sample {
        basal::precision v;
        iso::seconds t{offset};
        if (channel == 0) {
            v = audio::tone(audio::notes::a4, t);
        } else if (channel == 1) {
            v = audio::tone(audio::notes::c4, t);
        } else {
            v = 0.0_p;
        }
        return std::round(v * INT16_MAX);
    });
    EXPECT_TRUE(seq.save("stereo_a_and_c.wav"));
}
