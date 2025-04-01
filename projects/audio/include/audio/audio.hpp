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