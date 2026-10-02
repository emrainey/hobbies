#pragma once

#include <chrono>
#include <tuple>

#include <basal/basal.hpp>
#include <basal/ieee754.hpp>
#include <iso/iso.hpp>

namespace audio {

using precision = basal::precision;
using namespace basal::literals;

using SampleRate = iso::rate<uint32_t, iso::seconds>;
using Hertz = iso::hertz;

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

/// @brief Dual-tone multi-frequency (DTMF) signaling for telephone keypads
namespace dtmf {

enum class Key : uint8_t {
    _1  = ((0 << 0)|(0 << 4)),
    _2  = ((1 << 0)|(0 << 4)),
    _3  = ((2 << 0)|(0 << 4)),
    A  = ((3 << 0)|(0 << 4)),
    _4  = ((0 << 0)|(1 << 4)),
    _5  = ((1 << 0)|(1 << 4)),
    _6  = ((2 << 0)|(1 << 4)),
    B  = ((3 << 0)|(1 << 4)),
    _7  = ((0 << 0)|(2 << 4)),
    _8  = ((1 << 0)|(2 << 4)),
    _9  = ((2 << 0)|(2 << 4)),
    C  = ((3 << 0)|(2 << 4)),
    Asterisk  = ((0 << 0)|(3 << 4)),
    _0  = ((1 << 0)|(3 << 4)),
    Pound  = ((2 << 0)|(3 << 4)),
    D  = ((3 << 0)|(3 << 4))
};

constexpr static Key ToKey(char c) {
    switch (c) {
        case '1': return Key::_1;
        case '2': return Key::_2;
        case '3': return Key::_3;
        case '4': return Key::_4;
        case '5': return Key::_5;
        case '6': return Key::_6;
        case '7': return Key::_7;
        case '8': return Key::_8;
        case '9': return Key::_9;
        case '0': return Key::_0;
        case 'A': return Key::A;
        case 'B': return Key::B;
        case 'C': return Key::C;
        case 'D': return Key::D;
        case '*': return Key::Asterisk;
        case '#': return Key::Pound;
        default: return Key::_0; // Default to 0 if unknown
    }
}

/// @brief Get the frequency pair (row and column) for a given DTMF key
std::tuple<Hertz, Hertz> ToFrequencyPair(Key key);

//static constexpr std::tuple<Hertz, Hertz> busy_tone = std::make_tuple(480_Hz, 620_Hz);

} // namespace dtmf

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
/// @note Examples 4/4 is common time, Signature{4, Duration::Quarter}
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

/// @brief A tempo in beats per minute
using Tempo = std::uint16_t;

/// @brief Computes the number of seconds per beat given a tempo
/// @param tempo The tempo in beats per minute
/// @return The duration of a single beat in seconds
iso::seconds SecondsPerBeat(Tempo tempo);

/// Computes how long a beat is in seconds given the time signature and tempo and the beat
iso::seconds SecondsPerDuration(Tempo tempo, Signature signature, Duration beat);

/// @brief The twelveth root of 2, used for equal temperament
static constexpr basal::precision twelveth_root_of_2 = 1.059463094359295_p;  ///< 2^(1/12)

/// @brief Converts a tone and octave to a piano key
/// @param tone The tone name
/// @param octave The octave number
/// @return The piano key number
/// @see https://en.wikipedia.org/wiki/Piano_key_frequencies
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

    /// @brief Clamps a value to the valid range of the sample type
    /// @param value The value to clamp
    /// @return The clamped value
    static sample clamp(basal::precision value) {
        if (value > static_cast<basal::precision>(std::numeric_limits<sample>::max())) {
            return std::numeric_limits<sample>::max();
        } else if (value < static_cast<basal::precision>(std::numeric_limits<sample>::min())) {
            return std::numeric_limits<sample>::min();
        }
        return static_cast<sample>(value);
    }

    /// @brief Scales and clamps a value to the valid range of the sample type
    /// @param value The value to scale
    /// @return The scaled value
    static sample convert(basal::precision value) {
        return clamp(value * static_cast<basal::precision>(std::numeric_limits<sample>::max()));
    }

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

    /// Joins samples to the sequence in a vector of samples
    /// @param channel The channel to add the samples to
    /// @param samples The samples to add
    /// @param index the index to add the samples at
    /// @note This will overwrite the samples in the sequence or append data at the end
    void join(Sequence const& other) {
        basal::exception::throw_if(basal::nearly_equals(other.sample_rate(), sample_rate()) == false, __FILE__, __LINE__,
                                   "Sample rates must be the same");
        for (size_t i = 0; i < channels; ++i) {
            // copy the samples from the other sequence (this is automatically resize)
            samples_[i].insert(samples_[i].end(), other.samples_[i].begin(), other.samples_[i].end());
        }
    }

    /// Joins samples to the sequence in a vector of samples
    Sequence& operator+=(Sequence const& other) {
        join(other);
        return (*this);
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

/// @brief Defines a function for altering the amplitude of a signal
using envelope = std::function<basal::precision(basal::precision)>;

/// A pattern for a functor which takes a vector of precision and returns a vector of precision (of the same size) but
/// manipulated.
using resampler = std::function<std::vector<basal::precision>(std::vector<basal::precision>&&)>;

/// Shapes the input signal to a cosine envelope.
/// @param input The input data (a vector of precision)
/// @return The output data (a vector of precision)
std::vector<basal::precision> cosine(std::vector<basal::precision> const& input);

/// Computes the sample value for a given frequency and offset through a sine wave.
/// @param frequency The frequency of the sample
/// @param offset The time offset of the sample
/// @return The sample value
/// @note This is a simple sine wave generator
basal::precision waveform(iso::hertz frequency, iso::seconds offset);

/// @brief Creates a constant envelope with the given value
/// @param value The constant value for the envelope
/// @return The constant value
basal::precision constant(basal::precision value);

/// @brief Creates a cosine envelope for the given ratio
/// @param ratio A value between 0.0 to 1.0. Users can use any fraction value they want.
/// @return A value between 0.0 to 1.0
basal::precision cosine(basal::precision ratio);

/// @brief Creates a nice attack envelope with a decay
/// @param ratio A value between 0.0 to 1.0. Users can use any fraction value they want.
/// @return A value between 0.0 to 1.0
basal::precision pop(basal::precision ratio);

namespace in {
/// @brief Creates a linear fade envelope in from the given ratio
/// @param ratio A value between 0.0 to 1.0. Users can use any fraction value they want.
/// @return A value between 0.0 to 1.0
basal::precision fade(basal::precision ratio);
basal::precision ramp(basal::precision ratio);
}  // namespace in

namespace out {
/// @brief Creates a linear fade envelope out from the given ratio
/// @param ratio A value between 0.0 to 1.0. Users can use any fraction value they want.
/// @return A value between 0.0 to 1.0
basal::precision fade(basal::precision ratio);
basal::precision hill(basal::precision ratio);
basal::precision cliff(basal::precision ratio);
basal::precision ramp(basal::precision ratio);
}  // namespace out

template <typename SEQUENCE_TYPE>
class Mixer {
public:
    using sample = typename SEQUENCE_TYPE::sample;
    using SequenceType = SEQUENCE_TYPE;

    /// Mixes two sequences together using an average of the two sequences
    /// @param a The first sequence
    /// @param b The second sequence
    /// @return The mixed sequence in even parts
    static SequenceType even(SequenceType const& a, SequenceType const& b);
};

/// @brief Swizzles the input signal and sorts the even and odd samples into the front and back
/// @param input
void swizzle(std::complex<precision> input[], size_t N);

/// @brief Computes the FFT of the input signal
/// @param input
/// @return The FFT of the input signal
void fft(std::vector<std::complex<precision>>& data);

/// @brief Computes the inverse FFT of the input signal
/// @param input
/// @return The inverse FFT of the input signal
void ifft(std::vector<std::complex<precision>>& data);

/// Generates a mono PCM sequence with the given parameters
/// @param sample_rate The sample rate of the sequence
/// @param period The duration of the sequence in seconds
/// @param frequency The frequency of the waveform (in Hertz)
/// @return The generated mono PCM sequence
MonoPCM generate(basal::precision sample_rate, iso::seconds period, iso::hertz frequency, envelope env = constant);

/// Generates a Stereo PCM sequence with the given parameters
/// @param sample_rate The sample rate of the sequence
/// @param period The duration of the sequence in seconds
/// @param frequency_left The frequency of the waveform for the left channel (in Hertz)
/// @param frequency_right The frequency of the waveform for the right channel (in Hertz)
/// @return The generated stereo PCM sequence
StereoPCM generate(basal::precision sample_rate, iso::seconds period, iso::hertz frequency, iso::hertz frequency_right, envelope env = constant);

/// @brief Generates a mono PCM sequence with multiple tones
/// @param sample_rate The sample rate of the sequence
/// @param period The duration of the sequence in seconds
/// @param tones The vector of tones to generate
/// @return The generated mono PCM sequence containing the specified tones
MonoPCM generate(basal::precision sample_rate, iso::seconds period, std::vector<Tone> const& tones, envelope env = constant);

namespace dtmf {

/// Generates a DTMF tone for the given key
/// @param sample_rate The sample rate of the sequence
/// @param period The duration of the sequence in seconds
/// @param key The DTMF key
/// @return The generated mono PCM sequence containing the DTMF tone
MonoPCM generate(basal::precision sample_rate, iso::seconds period, Key key, envelope env = constant);

/// @brief Generates a DTMF sequence for the given phone number
/// @param sample_rate The sample rate of the sequence
/// @param period The duration of each DTMF tone in seconds
/// @param phone_number The phone number to dial
/// @return The generated mono PCM sequence containing the DTMF tones
MonoPCM dial(basal::precision sample_rate, iso::seconds period, std::string const& phone_number);

}  // namespace dtmf

}  // namespace audio
