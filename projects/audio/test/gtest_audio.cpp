#include <gtest/gtest.h>

#include <audio/audio.hpp>
#include <basal/gtest_helper.hpp>

using namespace audio;
using namespace audio::equal_temperament;
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

TEST(AudioTest, WhenSavingEnvelopedMonoPCM) {
    basal::precision sample_rate = audio::specification::cd_sample_rate;
    basal::precision period = 3.0_p;
    size_t number_of_samples = static_cast<size_t>(period * sample_rate);
    audio::MonoPCM seq{sample_rate, number_of_samples};
    seq.for_each([period](size_t channel, size_t index, basal::precision offset) -> audio::MonoPCM::sample {
        precision ratio = offset / period;
        iso::seconds t{offset};
        basal::precision v = envelope(ratio) * audio::waveform(ToFrequency(audio::Tone::Bb, 5), t);
        return std::round(v * INT16_MAX);
    });
    EXPECT_TRUE(seq.save("mono_Bb_enveloped.wav"));
}

TEST(AudioTest, WhenSavingMixedMonoPCM) {
    basal::precision sample_rate = audio::specification::cd_sample_rate;
    basal::precision period = 3.0_p;
    size_t number_of_samples = static_cast<size_t>(period * sample_rate);
    audio::MonoPCM seq0{sample_rate, number_of_samples};
    audio::MonoPCM seq1{sample_rate, number_of_samples};
    seq0.for_each([](size_t channel, size_t index, basal::precision offset) -> audio::MonoPCM::sample {
        iso::seconds t{offset};
        basal::precision v = audio::waveform(ToFrequency(audio::Tone::A, 4), t);
        return std::round(v * INT16_MAX);
    });
    seq1.for_each([](size_t channel, size_t index, basal::precision offset) -> audio::MonoPCM::sample {
        iso::seconds t{offset};
        basal::precision v = audio::waveform(ToFrequency(audio::Tone::C, 4), t);
        return std::round(v * INT16_MAX);
    });
    audio::MonoPCM seq2 = Mixer<audio::MonoPCM>::even(seq0, seq1);
    EXPECT_TRUE(seq2.save("mono_a_and_c.wav"));
}

TEST(AudioTest, WhenSavingJoinedMonoPCM) {
    basal::precision sample_rate = audio::specification::cd_sample_rate;
    basal::precision period = 3.0_p;
    size_t number_of_samples = static_cast<size_t>(period * sample_rate);
    audio::MonoPCM seq0{sample_rate, number_of_samples};
    audio::MonoPCM seq1{sample_rate, number_of_samples};
    seq0.for_each([](size_t channel, size_t index, basal::precision offset) -> audio::MonoPCM::sample {
        iso::seconds t{offset};
        basal::precision v = audio::waveform(ToFrequency(audio::Tone::A, 4), t);
        return std::round(v * INT16_MAX);
    });
    seq1.for_each([](size_t channel, size_t index, basal::precision offset) -> audio::MonoPCM::sample {
        iso::seconds t{offset};
        basal::precision v = audio::waveform(ToFrequency(audio::Tone::C, 4), t);
        return std::round(v * INT16_MAX);
    });
    seq0 += seq1;
    EXPECT_TRUE(seq0.save("mono_a_then_c.wav"));
}

TEST(AudioTest, WhenSavingStereoPCM) {
    basal::precision sample_rate = audio::specification::cd_sample_rate;
    basal::precision period = 3.0_p;
    size_t number_of_samples = static_cast<size_t>(period * sample_rate);
    audio::StereoPCM seq{sample_rate, number_of_samples};
    seq.for_each([](size_t channel, size_t index, basal::precision offset) -> audio::StereoPCM::sample {
        basal::precision v;
        iso::seconds t{offset};
        if (channel == 0) {
            v = audio::waveform(ToFrequency(audio::Tone::A, 4), t);
        } else if (channel == 1) {
            v = audio::waveform(ToFrequency(audio::Tone::C, 4), t);
        } else {
            v = 0.0_p;
        }
        return std::round(v * INT16_MAX);
    });
    EXPECT_TRUE(seq.save("stereo_a_and_c.wav"));
}

TEST(AudioTest, WhenSavingMixingStereoPCM) {
    basal::precision sample_rate = audio::specification::cd_sample_rate;
    basal::precision period = 3.0_p;
    size_t number_of_samples = static_cast<size_t>(period * sample_rate);
    audio::StereoPCM seq0{sample_rate, number_of_samples};
    audio::StereoPCM seq1{sample_rate, number_of_samples};
    seq0.for_each([](size_t channel, size_t index, basal::precision offset) -> audio::StereoPCM::sample {
        basal::precision v;
        iso::seconds t{offset};
        if (channel == 0) {
            v = audio::waveform(ToFrequency(audio::Tone::A, 4), t);
        } else if (channel == 1) {
            v = audio::waveform(ToFrequency(audio::Tone::C, 4), t);
        } else {
            v = 0.0_p;
        }
        return std::round(v * INT16_MAX);
    });
    seq1.for_each([](size_t channel, size_t index, basal::precision offset) -> audio::StereoPCM::sample {
        basal::precision v;
        iso::seconds t{offset};
        if (channel == 0) {
            v = audio::waveform(ToFrequency(audio::Tone::E, 4), t);
        } else if (channel == 1) {
            v = audio::waveform(ToFrequency(audio::Tone::C, 5), t);
        } else {
            v = 0.0_p;
        }
        return std::round(v * INT16_MAX);
    });
    audio::StereoPCM seq2 = Mixer<audio::StereoPCM>::even(seq0, seq1);
    EXPECT_TRUE(seq2.save("stereo_ae_and_cc.wav"));
}

TEST(AudioTest, WhenSavingJoinedStereoPCM) {
    basal::precision sample_rate = audio::specification::cd_sample_rate;
    basal::precision period = 3.0_p;
    size_t number_of_samples = static_cast<size_t>(period * sample_rate);
    audio::StereoPCM seq0{sample_rate, number_of_samples};
    audio::StereoPCM seq1{sample_rate, number_of_samples};
    seq0.for_each([](size_t channel, size_t index, basal::precision offset) -> audio::StereoPCM::sample {
        basal::precision v;
        iso::seconds t{offset};
        if (channel == 0) {
            v = audio::waveform(ToFrequency(audio::Tone::A, 4), t);
        } else if (channel == 1) {
            v = audio::waveform(ToFrequency(audio::Tone::C, 4), t);
        } else {
            v = 0.0_p;
        }
        return std::round(v * INT16_MAX);
    });
    seq1.for_each([](size_t channel, size_t index, basal::precision offset) -> audio::StereoPCM::sample {
        basal::precision v;
        iso::seconds t{offset};
        if (channel == 0) {
            v = audio::waveform(ToFrequency(audio::Tone::E, 4), t);
        } else if (channel == 1) {
            v = audio::waveform(ToFrequency(audio::Tone::C, 5), t);
        } else {
            v = 0.0_p;
        }
        return std::round(v * INT16_MAX);
    });
    seq0 += seq1;
    EXPECT_TRUE(seq0.save("stereo_ac_then_ec.wav"));
}

TEST(AudioTest, WhenSavingPopScale) {
    iso::hertz scale[8]
        = {ToFrequency(audio::Tone::C, 4), ToFrequency(audio::Tone::D, 4), ToFrequency(audio::Tone::E, 4),
           ToFrequency(audio::Tone::F, 4), ToFrequency(audio::Tone::G, 4), ToFrequency(audio::Tone::A, 4),
           ToFrequency(audio::Tone::B, 4), ToFrequency(audio::Tone::C, 5)};
    basal::precision sample_rate = audio::specification::cd_sample_rate;
    basal::precision period = 0.5_p;
    basal::precision full_period = period * 8.0_p;
    size_t subcount = static_cast<size_t>(period * sample_rate);
    size_t number_of_samples = static_cast<size_t>(full_period * sample_rate);
    audio::MonoPCM seq{sample_rate, number_of_samples};
    seq.for_each([&](size_t channel, size_t index, basal::precision offset) -> audio::MonoPCM::sample {
        iso::seconds t{offset};
        basal::precision r = fmod(offset, period) / period;
        size_t i = index / subcount;
        basal::precision v = audio::pop(r) * audio::waveform(scale[i], t);
        return std::round(v * INT16_MAX);
    });
    EXPECT_TRUE(seq.save("mono_c_major_scale.wav"));
}

TEST(AudioTest, WhenConvertingPianoKeysToFrequency) {
    using namespace basal::literals;
    using namespace iso::literals;
    using namespace audio::equal_temperament;
    ASSERT_NEAR((16.35_Hz).value, ToFrequency(audio::Tone::C, 0).value, 0.0125_p);     // Lowest C
    ASSERT_NEAR((261.6256_Hz).value, ToFrequency(audio::Tone::C, 4).value, 0.0125_p);  // Middle C
    ASSERT_NEAR((440.0_Hz).value, ToFrequency(audio::Tone::A, 4).value, 0.0125_p);     // Middle A
    ASSERT_NEAR((1479.98_Hz).value, ToFrequency(audio::Tone::Fs, 6).value, 0.0125_p);  // F#6
    ASSERT_NEAR((7902.13_Hz).value, ToFrequency(audio::Tone::B, 8).value, 0.0125_p);   // Highest B

    // Equal Notes w/ Different Names
    for (int octave = 0; octave < 8; ++octave) {
        ASSERT_EQ(ToFrequency(audio::Tone::Cs, octave).value, ToFrequency(audio::Tone::Db, octave).value);
        ASSERT_EQ(ToFrequency(audio::Tone::Ds, octave).value, ToFrequency(audio::Tone::Eb, octave).value);
        ASSERT_EQ(ToFrequency(audio::Tone::Fs, octave).value, ToFrequency(audio::Tone::Gb, octave).value);
        ASSERT_EQ(ToFrequency(audio::Tone::Gs, octave).value, ToFrequency(audio::Tone::Ab, octave).value);
        ASSERT_EQ(ToFrequency(audio::Tone::As, octave).value, ToFrequency(audio::Tone::Bb, octave).value);
    }
}

TEST(AudioTest, Envelope) {
    size_t const count = 16U;
    precision const value = 4.0_p;
    std::vector<basal::precision> input;
    for (size_t i = 0U; i < count; i++) {
        input.push_back(value);
    }
    std::vector<basal::precision> output = audio::envelope(input);
    ASSERT_EQ(output.size(), count);
    for (size_t i = 0; i < output.size(); ++i) {
        ASSERT_LE(output[i], value);
    }
}

TEST(AudioTest, SecondsPerBeat) {
    ASSERT_PRECISION_EQ(1.0_p, SecondsPerBeat(60).value) << "60 BPM is 1 second per beat";
    ASSERT_PRECISION_EQ(0.5_p, SecondsPerBeat(120).value) << "120 BPM is 0.5 seconds per beat";
    ASSERT_PRECISION_EQ(0.25_p, SecondsPerBeat(240).value) << "240 BPM is 0.25 seconds per beat";
}

TEST(AudioTest, SecondsPerDuration) {
    ASSERT_PRECISION_EQ(1.0_p, SecondsPerDuration(60, 4 / audio::Duration::Quarter, audio::Duration::Quarter).value)
        << "60 BPM @ 4/4 time is 1 second per quarter note";
    ASSERT_PRECISION_EQ(2.0_p, SecondsPerDuration(60, 4 / audio::Duration::Quarter, audio::Duration::Half).value)
        << "60 BPM @ 4/4 time is 2 seconds per half note";
    ASSERT_PRECISION_EQ(4.0_p, SecondsPerDuration(60, 4 / audio::Duration::Quarter, audio::Duration::Whole).value)
        << "60 BPM @ 4/4 time is 4 seconds per whole note";
}