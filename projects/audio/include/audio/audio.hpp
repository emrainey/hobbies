#pragma once

#include <basal/basal.hpp>
#include <basal/ieee754.hpp>
#include <iso/iso.hpp>

namespace audio {

using precision = basal::precision;
using namespace basal::literals;

template <typename SAMPLE_TYPE, size_t CHANNELS>
class Sequence {
public:
    using sample = SAMPLE_TYPE;
    static constexpr size_t channels = CHANNELS;

    /// Default Constructor
    Sequence() : Sequence(44'800.0_p, 134'400U) {
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

    Sequence& for_each(std::function<void(size_t, size_t, sample&)> iter) {
        // for each sample
        for (size_t i = 0; i < count(); ++i) {
            // for each channel
            for (size_t c = 0; c < channels; ++c) {
                iter(c, i, samples_[c][i]);
            }
        }
        return (*this);
    }

    Sequence const& for_each(std::function<void(size_t, size_t, sample const&)> iter) const {
        // for each sample
        for (size_t i = 0; i < count(); ++i) {
            // for each channel
            for (size_t c = 0; c < channels; ++c) {
                iter(c, i, samples_[c][i]);
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

using MonoPCM = Sequence<uint16_t, 1>;
using StereoPCM = Sequence<uint16_t, 2>;

class Mixer {};

}  // namespace audio