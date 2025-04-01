#pragma once

#include <basal/basal.hpp>
#include <basal/ieee754.hpp>
#include <iso/iso.hpp>

namespace audio {

using precision = basal::precision;
using namespace basal::literals;

using SampleRate = iso::rate<uint32_t, iso::seconds>;

namespace specification {
/// The default sample rate for audio
static constexpr precision default_sample_rate = 44'800.0_p;
/// The default sample rate for CD quality audio
static constexpr precision cd_sample_rate = 44'100.0_p;
/// The default sample rate for DVD quality audio
static constexpr precision dvd_sample_rate = 48'000.0_p;
/// The default sample rate for BluRay quality audio
static constexpr precision blu_ray_sample_rate = 96'000.0_p;
/// The default sample rate for high definition audio
static constexpr precision high_def_sample_rate = 192'000.0_p;
}  // namespace specification

namespace equal_temperament {
using namespace iso::literals;

const iso::hertz c0 = 16.35_Hz;   ///< C0
const iso::hertz cs0 = 17.32_Hz;  ///< C#0 / Db0
const iso::hertz d0 = 18.35_Hz;   ///< D0
const iso::hertz ds0 = 19.45_Hz;  ///< D#0 / Eb0
const iso::hertz e0 = 20.60_Hz;   ///< E0
const iso::hertz f0 = 21.83_Hz;   ///< F0
const iso::hertz fs0 = 23.12_Hz;  ///< F#0 / Gb0
const iso::hertz g0 = 24.50_Hz;   ///< G0
const iso::hertz gs0 = 25.96_Hz;  ///< G#0 / Ab0
const iso::hertz a0 = 27.50_Hz;   ///< A0
const iso::hertz as0 = 29.14_Hz;  ///< A#0 / Bb0
const iso::hertz b0 = 30.87_Hz;   ///< B0

const iso::hertz c1 = 32.70_Hz;   ///< C1
const iso::hertz cs1 = 34.65_Hz;  ///< C#1 / Db1
const iso::hertz d1 = 36.71_Hz;   ///< D1
const iso::hertz ds1 = 38.89_Hz;  ///< D#1 / Eb1
const iso::hertz e1 = 41.20_Hz;   ///< E1
const iso::hertz f1 = 43.65_Hz;   ///< F1
const iso::hertz fs1 = 46.25_Hz;  ///< F#1 / Gb1
const iso::hertz g1 = 49.00_Hz;   ///< G1
const iso::hertz gs1 = 51.91_Hz;  ///< G#1 / Ab1
const iso::hertz a1 = 55.00_Hz;   ///< A1
const iso::hertz as1 = 58.27_Hz;  ///< A#1 / Bb1
const iso::hertz b1 = 61.74_Hz;   ///< B1

const iso::hertz c2 = 65.41_Hz;    ///< C2
const iso::hertz cs2 = 69.30_Hz;   ///< C#2 / Db2
const iso::hertz d2 = 73.42_Hz;    ///< D2
const iso::hertz ds2 = 77.78_Hz;   ///< D#2 / Eb2
const iso::hertz e2 = 82.41_Hz;    ///< E2
const iso::hertz f2 = 87.31_Hz;    ///< F2
const iso::hertz fs2 = 92.50_Hz;   ///< F#2 / Gb2
const iso::hertz g2 = 98.00_Hz;    ///< G2
const iso::hertz gs2 = 103.83_Hz;  ///< G#2 / Ab2
const iso::hertz a2 = 110.00_Hz;   ///< A2
const iso::hertz as2 = 116.54_Hz;  ///< A#2 / Bb2
const iso::hertz b2 = 123.47_Hz;   ///< B2

const iso::hertz c3 = 130.81_Hz;   ///< C3
const iso::hertz cs3 = 138.59_Hz;  ///< C#3 / Db3
const iso::hertz d3 = 146.83_Hz;   ///< D3
const iso::hertz ds3 = 155.56_Hz;  ///< D#3 / Eb3
const iso::hertz e3 = 164.81_Hz;   ///< E3
const iso::hertz f3 = 174.61_Hz;   ///< F3
const iso::hertz fs3 = 185.00_Hz;  ///< F#3 / Gb3
const iso::hertz g3 = 196.00_Hz;   ///< G3
const iso::hertz gs3 = 207.65_Hz;  ///< G#3 / Ab3
const iso::hertz a3 = 220.00_Hz;   ///< A3
const iso::hertz as3 = 233.08_Hz;  ///< A#3 / Bb3
const iso::hertz b3 = 246.94_Hz;   ///< B3

const iso::hertz c4 = 261.63_Hz;   ///< C4
const iso::hertz cs4 = 277.18_Hz;  ///< C#4 / Db4
const iso::hertz d4 = 293.66_Hz;   ///< D4
const iso::hertz ds4 = 311.13_Hz;  ///< D#4 / Eb4
const iso::hertz e4 = 329.63_Hz;   ///< E4
const iso::hertz f4 = 349.23_Hz;   ///< F4
const iso::hertz fs4 = 369.99_Hz;  ///< F#4 / Gb4
const iso::hertz g4 = 392.00_Hz;   ///< G4
const iso::hertz gs4 = 415.30_Hz;  ///< G#4 / Ab4
const iso::hertz a4 = 440.00_Hz;   ///< A4
const iso::hertz as4 = 466.16_Hz;  ///< A#4 / Bb4
const iso::hertz b4 = 493.88_Hz;   ///< B4

const iso::hertz c5 = 523.25_Hz;   ///< C5
const iso::hertz cs5 = 554.37_Hz;  ///< C#5 / Db5
const iso::hertz d5 = 587.33_Hz;   ///< D5
const iso::hertz ds5 = 622.25_Hz;  ///< D#5 / Eb5
const iso::hertz e5 = 659.25_Hz;   ///< E5
const iso::hertz f5 = 698.46_Hz;   ///< F5
const iso::hertz fs5 = 739.99_Hz;  ///< F#5 / Gb5
const iso::hertz g5 = 783.99_Hz;   ///< G5
const iso::hertz gs5 = 830.61_Hz;  ///< G#5 / Ab5
const iso::hertz a5 = 880.00_Hz;   ///< A5
const iso::hertz as5 = 932.33_Hz;  ///< A#5 / Bb5
const iso::hertz b5 = 987.77_Hz;   ///< B5

const iso::hertz c6 = 1046.50_Hz;   ///< C6
const iso::hertz cs6 = 1108.73_Hz;  ///< C#6 / Db6
const iso::hertz d6 = 1174.66_Hz;   ///< D6
const iso::hertz ds6 = 1244.51_Hz;  ///< D#6 / Eb6
const iso::hertz e6 = 1318.51_Hz;   ///< E6
const iso::hertz f6 = 1396.91_Hz;   ///< F6
const iso::hertz fs6 = 1479.98_Hz;  ///< F#6 / Gb6
const iso::hertz g6 = 1567.98_Hz;   ///< G6
const iso::hertz gs6 = 1661.22_Hz;  ///< G#6 / Ab6
const iso::hertz a6 = 1760.00_Hz;   ///< A6
const iso::hertz as6 = 1864.66_Hz;  ///< A#6 / Bb6
const iso::hertz b6 = 1975.53_Hz;   ///< B6

const iso::hertz c7 = 2093.00_Hz;   ///< C7
const iso::hertz cs7 = 2217.46_Hz;  ///< C#7 / Db7
const iso::hertz d7 = 2349.32_Hz;   ///< D7
const iso::hertz ds7 = 2489.02_Hz;  ///< D#7 / Eb7
const iso::hertz e7 = 2637.02_Hz;   ///< E7
const iso::hertz f7 = 2793.83_Hz;   ///< F7
const iso::hertz fs7 = 2959.96_Hz;  ///< F#7 / Gb7
const iso::hertz g7 = 3135.96_Hz;   ///< G7
const iso::hertz gs7 = 3322.44_Hz;  ///< G#7 / Ab7
const iso::hertz a7 = 3520.00_Hz;   ///< A7
const iso::hertz as7 = 3729.31_Hz;  ///< A#7 / Bb7
const iso::hertz b7 = 3951.07_Hz;   ///< B7

const iso::hertz c8 = 4186.01_Hz;   ///< C8
const iso::hertz cs8 = 4434.92_Hz;  ///< C#8 / Db8
const iso::hertz d8 = 4698.64_Hz;   ///< D8
const iso::hertz ds8 = 4978.03_Hz;  ///< D#8 / Eb8
const iso::hertz e8 = 5274.04_Hz;   ///< E8
const iso::hertz f8 = 5587.65_Hz;   ///< F8
const iso::hertz fs8 = 5919.91_Hz;  ///< F#8 / Gb8
const iso::hertz g8 = 6271.93_Hz;   ///< G8
const iso::hertz gs8 = 6644.88_Hz;  ///< G#8 / Ab8
const iso::hertz a8 = 7040.00_Hz;   ///< A8
const iso::hertz as8 = 7458.62_Hz;  ///< A#8 / Bb8
const iso::hertz b8 = 7902.13_Hz;   ///< B8

const iso::hertz db0 = cs0;  ///< Db0
const iso::hertz eb0 = ds0;  ///< Eb0
const iso::hertz gb0 = fs0;  ///< Gb0
const iso::hertz ab0 = gs0;  ///< Ab0
const iso::hertz bb0 = as0;  ///< Bb0

const iso::hertz db1 = cs1;  ///< Db1
const iso::hertz eb1 = ds1;  ///< Eb1
const iso::hertz gb1 = fs1;  ///< Gb1
const iso::hertz ab1 = gs1;  ///< Ab1
const iso::hertz bb1 = as1;  ///< Bb1

const iso::hertz db2 = cs2;  ///< Db2
const iso::hertz eb2 = ds2;  ///< Eb2
const iso::hertz gb2 = fs2;  ///< Gb2
const iso::hertz ab2 = gs2;  ///< Ab2
const iso::hertz bb2 = as2;  ///< Bb2

const iso::hertz db3 = cs3;  ///< Db3
const iso::hertz eb3 = ds3;  ///< Eb3
const iso::hertz gb3 = fs3;  ///< Gb3
const iso::hertz ab3 = gs3;  ///< Ab3
const iso::hertz bb3 = as3;  ///< Bb3

const iso::hertz db4 = cs4;  ///< Db4
const iso::hertz eb4 = ds4;  ///< Eb4
const iso::hertz gb4 = fs4;  ///< Gb4
const iso::hertz ab4 = gs4;  ///< Ab4
const iso::hertz bb4 = as4;  ///< Bb4

const iso::hertz db5 = cs5;  ///< Db5
const iso::hertz eb5 = ds5;  ///< Eb5
const iso::hertz gb5 = fs5;  ///< Gb5
const iso::hertz ab5 = gs5;  ///< Ab5
const iso::hertz bb5 = as5;  ///< Bb5

const iso::hertz db6 = cs6;  ///< Db6
const iso::hertz eb6 = ds6;  ///< Eb6
const iso::hertz gb6 = fs6;  ///< Gb6
const iso::hertz ab6 = gs6;  ///< Ab6
const iso::hertz bb6 = as6;  ///< Bb6

const iso::hertz db7 = cs7;  ///< Db7
const iso::hertz eb7 = ds7;  ///< Eb7
const iso::hertz gb7 = fs7;  ///< Gb7
const iso::hertz ab7 = gs7;  ///< Ab7
const iso::hertz bb7 = as7;  ///< Bb7

const iso::hertz db8 = cs8;  ///< Db8
const iso::hertz eb8 = ds8;  ///< Eb8
const iso::hertz gb8 = fs8;  ///< Gb8
const iso::hertz ab8 = gs8;  ///< Ab8
const iso::hertz bb8 = as8;  ///< Bb8
}  // namespace equal_temperament

namespace notes = equal_temperament;

/// Computes the sample value for a given frequency and offset
/// @param frequency The frequency of the sample
/// @param offset The time offset of the sample
/// @return The sample value
/// @note This is a simple sine wave generator
precision tone(iso::hertz frequency, iso::seconds offset);

template <typename SAMPLE_TYPE, size_t CHANNELS>
class Sequence {
public:
    using sample = SAMPLE_TYPE;
    static constexpr size_t channels = CHANNELS;

    /// Default Constructor
    Sequence() : Sequence(specification::cd_sample_rate, 0U) {
    }

    Sequence(precision sample_rate, size_t samples) : sample_rate_{sample_rate}, samples_{} {
        basal::exception::throw_if(channels == 0, __FILE__, __LINE__, "Channels must be greater than zero");
        basal::exception::throw_if(sample_rate <= 0, __FILE__, __LINE__, "Sample rate must be greater than zero");
        basal::exception::throw_if(samples == 0, __FILE__, __LINE__, "Samples must be greater than zero");
        samples_.fill(std::vector<sample>{});
        for (size_t i = 0; i < channels; ++i) {
            samples_[i].resize(samples);
        }
    }

    /// @return The duration of the sequence
    std::chrono::duration<precision> duration() const {
        return std::chrono::duration<precision>(static_cast<precision>(count()) / sample_rate_);
    }

    /// Mutable Sample Accessor per channel
    /// @param index The sample index
    sample& operator()(size_t chan, size_t index) {
        basal::exception::throw_if(chan >= channels, __FILE__, __LINE__, "Out of bounds channel=%z", chan);
        basal::exception::throw_if(index >= count(), __FILE__, __LINE__, "Out of bounds index=%z", index);
        return samples_[chan][index];
    }

    /// Const Sample Accessor per channel
    /// @param index The sample index
    sample const& operator()(size_t chan, size_t index) const {
        basal::exception::throw_if(chan >= channels, __FILE__, __LINE__, "Out of bounds channel=%z", chan);
        basal::exception::throw_if(index >= count(), __FILE__, __LINE__, "Out of bounds index=%z", index);
        return samples_[chan][index];
    }

    /// @return The number of samples in the sequence
    size_t count() const {
        return samples_[0].size();
    }

    /// Loads the sequence from a file (like a .wav file)
    /// @param filename The file to load
    bool load(std::string filename);

    /// Saves the sequence to a file (like a .wav file)
    /// @param filename The file to save to
    bool save(std::string filename) const;

    /// @brief Generates samples from the channel, index and time offset
    /// @param channel The channel to generate
    /// @param index The index of the sample
    /// @param offset The time offset of the sample
    /// @return The generated sample
    /// @note This is a functor that is called for each potential sample in the sequence
    using generator = std::function<sample(size_t channel, size_t index, precision offset)>;

    /// @brief Iterates over each sample in the sequence
    /// @param iter the functor to call for each sample
    /// @return
    Sequence& for_each(generator gen) {
        precision const delta = 1.0_p / sample_rate_;
        // for each sample
        for (size_t i = 0; i < count(); ++i) {
            precision t = static_cast<precision>(i) * delta;
            // for each channel
            for (size_t c = 0; c < channels; ++c) {
                samples_[c][i] = gen(c, i, t);
            }
        }
        return (*this);
    }

    /// Used to iterate over each existing sample in the sequence
    /// the Iterator is called for each channel of each sample, not all samples of a channel, then the other.
    /// @param channel The channel where the sample is from
    /// @param index The index of the sample
    /// @param offset The time offset of the sample
    /// @param sample The sample to use
    /// @note This is a functor that is called for each sample in the sequence
    using iterator = std::function<void(size_t, size_t, precision, sample const&)>;

    /// @brief Iterates over each sample in the sequence
    /// @param iter the functor to call for each sample
    Sequence const& for_each(iterator iter) const {
        precision const delta = 1.0_p / sample_rate_;
        // for each sample
        for (size_t i = 0; i < count(); ++i) {
            precision t = static_cast<precision>(i) * delta;
            // for each channel
            for (size_t c = 0; c < channels; ++c) {
                iter(c, i, t, samples_[c][i]);
            }
        }
        return (*this);
    }

    precision sample_rate() const {
        return sample_rate_;
    }

protected:
    /// The sample rate of the sequence
    precision sample_rate_;
    /// The array of channels of vector of samples
    std::array<std::vector<sample>, CHANNELS> samples_;
};

using MonoPCM = Sequence<int16_t, 1>;
using StereoPCM = Sequence<int16_t, 2>;

class Mixer {};

}  // namespace audio