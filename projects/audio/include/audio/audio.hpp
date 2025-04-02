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

/// @brief Enumerated values of the letter tone as they would be on a piano keyboard
enum class Tone : int {
    C = 4,  // 4, 16, 28, 40, 52, 64, 76, 88
    Cs = 5,
    Db = Cs,
    D = 6,
    Ds = 7,
    Eb = Ds,
    E = 8,
    F = 9,
    Fs = 10,
    Gb = Fs,
    G = 11,
    Gs = 12,
    Ab = Gs,
    A = 13,
    As = 14,
    Bb = As,
    B = 15,
};

struct Pitch {
    Tone tone;   ///< The tone (e.g. C, D, E, etc.)
    int octave;  ///< The octave (0-8)
};

/// @brief Enumerated values for use in TimeSignatures
enum class Duration : int {
    Whole = 1,
    Half = 2,
    Quarter = 4,
    Crotchet = Quarter,
    Eighth = 8,
    Quaver = Eighth,
    Sixteenth = 16,
    Semiquaver = Sixteenth,
    ThirtySecond = 32,
};

/// @brief A time or metre signature for use in music
/// The numerator is the number of beats in a measure (e.g. 2,3,4,5,6),
/// The denominator is the type of beat (e.g. quarter, eighth, etc.)
/// @note Examples 4/4 is common time, TimeSignature{4, Duration::Quarter}
struct Signature {
    uint8_t beats_per_measure;  ///< The number of beats in a measure (e.g. 2,3,4,5,6)
    Duration value;             ///< The type of beat (e.g. quarter, eighth, etc.)
};

/// @brief A constructor for a time signature from a literal divider
/// @param beats The number of beats in a measure (e.g. 2,3,4,5,6)
/// @param value The type of beat (e.g. quarter, eighth, etc.)
/// @return A time signature
constexpr Signature operator/(unsigned long beats, Duration value) {
    return Signature{static_cast<uint8_t>(beats), value};
}

using Tempo = std::uint16_t;

iso::seconds SecondsPerBeat(Tempo tempo);

/// Computes how long a beat is in seconds given the time signature and tempo and the beat
iso::seconds SecondsPerDuration(Tempo tempo, Signature signature, Duration beat);

/// @brief The twelveth root of 2, used for equal temperament
static constexpr precision twelveth_root_of_2 = 1.059463094359295_p;  ///< 2^(1/12)

constexpr int ToPianoKey(Tone tone, int octave) {
    return basal::to_underlying(tone) + (12 * (octave - 1));
}

static_assert(ToPianoKey(Tone::C, 1) == 4, "C1 is 4");
static_assert(ToPianoKey(Tone::C, 4) == 40, "C4 is 40");
static_assert(ToPianoKey(Tone::Fs, 4) == 46, "F# is 46");
static_assert(ToPianoKey(Tone::A, 4) == 49, "A4 is 49");
static_assert(ToPianoKey(Tone::E, 5) == 56, "E5 is 56");
static_assert(ToPianoKey(Tone::C, 8) == 88, "C8 is 88");

namespace equal_temperament {

/// @brief Converts a pitch to a frequency
iso::hertz ToFrequency(Tone tone, int octave);

/// @brief Converts a pitch to a frequency
inline iso::hertz ToFrequency(Pitch pitch) {
    return ToFrequency(pitch.tone, pitch.octave);
}

}  // namespace equal_temperament

namespace notes = equal_temperament;

template <typename SAMPLE_TYPE, size_t CHANNELS>
class Sequence {
public:
    using sample = SAMPLE_TYPE;
    static constexpr size_t channels = CHANNELS;

    /// Default Constructor
    Sequence() : Sequence(specification::cd_sample_rate, 0U) {
    }

    Sequence(basal::precision sample_rate, size_t samples) : sample_rate_{sample_rate}, samples_{} {
        basal::exception::throw_if(channels == 0, __FILE__, __LINE__, "Channels must be greater than zero");
        basal::exception::throw_if(sample_rate <= 0, __FILE__, __LINE__, "Sample rate must be greater than zero");
        basal::exception::throw_if(samples == 0, __FILE__, __LINE__, "Samples must be greater than zero");
        samples_.fill(std::vector<sample>{});
        for (size_t i = 0; i < channels; ++i) {
            samples_[i].resize(samples);
        }
    }

    /// @return The duration of the sequence
    std::chrono::duration<basal::precision> duration() const {
        return std::chrono::duration<basal::precision>(static_cast<basal::precision>(count()) / sample_rate_);
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
    using generator = std::function<sample(size_t channel, size_t index, basal::precision offset)>;

    /// @brief Iterates over each sample in the sequence
    /// @param iter the functor to call for each sample
    /// @return
    Sequence& for_each(generator gen) {
        basal::precision const delta = 1.0_p / sample_rate_;
        // for each sample
        for (size_t i = 0; i < count(); ++i) {
            basal::precision t = static_cast<basal::precision>(i) * delta;
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
    using iterator = std::function<void(size_t, size_t, basal::precision, sample const&)>;

    /// @brief Iterates over each sample in the sequence
    /// @param iter the functor to call for each sample
    Sequence const& for_each(iterator iter) const {
        basal::precision const delta = 1.0_p / sample_rate_;
        // for each sample
        for (size_t i = 0; i < count(); ++i) {
            basal::precision t = static_cast<basal::precision>(i) * delta;
            // for each channel
            for (size_t c = 0; c < channels; ++c) {
                iter(c, i, t, samples_[c][i]);
            }
        }
        return (*this);
    }

    basal::precision sample_rate() const {
        return sample_rate_;
    }

protected:
    /// The sample rate of the sequence
    basal::precision sample_rate_;
    /// The array of channels of vector of samples
    std::array<std::vector<sample>, CHANNELS> samples_;
};

using MonoPCM = Sequence<int16_t, 1>;
using StereoPCM = Sequence<int16_t, 2>;
using MonoPCMFloat = Sequence<float, 1>;
using StereoPCMFloat = Sequence<float, 2>;

/// A pattern for a functor which takes a vector of precision and returns a vector of precision (of the same size) but
/// manipulated.
using resampler = std::function<std::vector<basal::precision>(std::vector<basal::precision>&&)>;

/// Shapes the input signal to a cosine envelope.
/// @param input The input data (a vector of precision)
/// @return
std::vector<basal::precision> envelope(std::vector<basal::precision> const& input);

/// Computes the sample value for a given frequency and offset through a sine wave.
/// @param frequency The frequency of the sample
/// @param offset The time offset of the sample
/// @return The sample value
/// @note This is a simple sine wave generator
basal::precision waveform(iso::hertz frequency, iso::seconds offset);

template <typename SEQUENCE_TYPE>
class Mixer {
public:
    using sample = typename SEQUENCE_TYPE::sample;
    using SequenceType = SEQUENCE_TYPE;

    /// Mixes two sequences together
    /// @param a The first sequence
    /// @param b The second sequence
    /// @return The mixed sequence in even parts
    static SequenceType even(SequenceType const& a, SequenceType const& b);
};

}  // namespace audio