#include <gtest/gtest.h>

#include <cmath>
#include <cstdio>
#include <fstream>

#include <audio/audio.hpp>
#include <basal/gtest_helper.hpp>
#include <fft_vis/fft_vis.hpp>

using namespace basal::literals;
using namespace iso::literals;

namespace {
fft_vis::Signal tone(basal::precision sample_rate, basal::precision frequency, size_t count) {
    fft_vis::Signal signal;
    signal.sample_rate = sample_rate;
    signal.channels.assign(2U, std::vector<basal::precision>(count));
    for (size_t i = 0; i < count; ++i) {
        signal.channels[0][i] = 0.5_p * std::sin(iso::tau * frequency * static_cast<basal::precision>(i) / sample_rate);
        signal.channels[1][i] = signal.channels[0][i] * 0.5_p;
    }
    return signal;
}
}  // namespace

TEST(FftVisTest, ReadsWhatAudioWrote) {
    audio::StereoPCM seq = audio::generate(8'000.0_p, 1.0_sec, 440.0_Hz, 660.0_Hz);
    ASSERT_TRUE(seq.save("fft_vis_stereo.wav"));
    fft_vis::Signal signal;
    ASSERT_TRUE(fft_vis::read_wav("fft_vis_stereo.wav", signal));
    EXPECT_EQ(2U, signal.channels.size());
    EXPECT_EQ(seq.count(), signal.count());
    EXPECT_NEAR(8'000.0_p, signal.sample_rate, 0.5_p);
    for (size_t i = 0; i < seq.count(); i += 97U) {
        EXPECT_NEAR(static_cast<basal::precision>(seq(1, i)) / 32768.0_p, signal.channels[1][i], 1e-4_p);
    }
}

TEST(FftVisTest, RejectsMissingAndInvalidFiles) {
    fft_vis::Signal signal;
    EXPECT_FALSE(fft_vis::read_wav("this_file_does_not_exist.wav", signal));
    std::ofstream("fft_vis_bad.wav") << "not a wav file at all";
    EXPECT_FALSE(fft_vis::read_wav("fft_vis_bad.wav", signal));
}

TEST(FftVisTest, WritesSpectrogramImage) {
    fft_vis::Signal signal = tone(8'000.0_p, 1'000.0_p, 8'000U);
    fft_vis::Parameters parameters;
    parameters.fft_size = 512U;
    parameters.max_frequency = 4'000.0_p;
    ASSERT_TRUE(fft_vis::write_spectrogram(signal, parameters, "fft_vis_test.ppm"));
    std::ifstream file("fft_vis_test.ppm", std::ios::binary);
    std::string magic;
    size_t width = 0U, height = 0U;
    file >> magic >> width >> height;
    EXPECT_EQ("P6", magic);
    size_t const bins = 256U;  // limited to fft_size / 2 (4 kHz at 15.625 Hz per bin)
    EXPECT_EQ(1U + (8'000U - 512U) / 128U, width);
    EXPECT_EQ(bins * 2U + 1U, height);
}

TEST(FftVisTest, RejectsBadSpectrogramParameters) {
    fft_vis::Signal signal = tone(8'000.0_p, 1'000.0_p, 8'000U);
    fft_vis::Parameters parameters;
    parameters.fft_size = 1000U;  // not a power of two
    EXPECT_FALSE(fft_vis::write_spectrogram(signal, parameters, "fft_vis_unused.ppm"));
    parameters.fft_size = 16'384U;  // longer than the signal
    EXPECT_FALSE(fft_vis::write_spectrogram(signal, parameters, "fft_vis_unused.ppm"));
}

TEST(FftVisTest, SteadyToneIsStaticAndDifferentTonesAreNot) {
    fft_vis::Signal steady = tone(8'000.0_p, 1'000.0_p, 8'192U * 4U);
    auto stats = fft_vis::analyze(steady, 1024U);
    ASSERT_TRUE(stats.has_value());
    EXPECT_NEAR(1'000.0_p, stats->min_centroid, 50.0_p);
    EXPECT_GT(stats->min_similarity, 0.99_p);

    fft_vis::Signal changing = tone(8'000.0_p, 500.0_p, 4'096U);
    fft_vis::Signal high = tone(8'000.0_p, 3'000.0_p, 4'096U);
    changing.channels[0].insert(changing.channels[0].end(), high.channels[0].begin(), high.channels[0].end());
    stats = fft_vis::analyze(changing, 1024U);
    ASSERT_TRUE(stats.has_value());
    EXPECT_LT(stats->min_similarity, 0.5_p);
    EXPECT_GT(stats->max_centroid - stats->min_centroid, 1'000.0_p);

    EXPECT_FALSE(fft_vis::analyze(steady, 100U).has_value());
    EXPECT_FALSE(fft_vis::analyze(steady, 1U << 20U).has_value());
}
