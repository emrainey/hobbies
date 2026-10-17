#pragma once
/// @file
/// Tools to look at audio through the FFT: a WAV reader, spectrogram images and variation statistics.

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include <basal/ieee754.hpp>

namespace fft_vis {

using precision = basal::precision;
using namespace basal::literals;

/// Floating point audio, one vector of samples (nominally -1.0 to 1.0) per channel.
struct Signal {
    precision sample_rate{0.0_p};
    std::vector<std::vector<precision>> channels;

    /// @return The number of samples in each channel (0 if there are no channels)
    size_t count() const {
        return channels.empty() ? 0U : channels[0].size();
    }
};

/// Parameters for creating a spectrogram
struct Parameters {
    size_t fft_size{2048U};             ///< Must be a power of two and at least 16. Frames overlap by 75%.
    precision max_frequency{8000.0_p};  ///< The highest frequency shown in Hz.
    precision db_range{80.0_p};         ///< The dynamic range shown in dB below the peak.
};

/// Variation statistics of the first channel over non overlapping frames.
struct Statistics {
    precision min_centroid{0.0_p};    ///< Lowest spectral centroid (Hz) of any frame
    precision max_centroid{0.0_p};    ///< Highest spectral centroid (Hz) of any frame
    precision min_rms{0.0_p};         ///< Quietest frame
    precision max_rms{0.0_p};         ///< Loudest frame
    precision min_similarity{1.0_p};  ///< Lowest frame to frame magnitude spectrum similarity (1.0 is static)
};

/// Reads a mono or stereo .wav file into floating point channels using audio::Sequence::load.
/// @param filename The .wav file
/// @param signal The output signal
/// @return true on success; the reason is printed to stderr on failure.
bool read_wav(std::string const& filename, Signal& signal);

/// Writes a spectrogram of every channel as a .ppm or .tga image using audio::fft.
/// Time runs left to right, frequency bottom to top, color is log magnitude relative to the peak of all channels.
/// Channels are stacked top to bottom separated by a white row.
/// @param signal The audio
/// @param parameters The FFT and display parameters
/// @param filename The output .ppm or .tga file
/// @return true on success; the reason is printed to stderr on failure.
bool write_spectrogram(Signal const& signal, Parameters const& parameters, std::string const& filename);

/// Computes how much the first channel changes over time.
/// @param signal The audio
/// @param fft_size The frame size, a power of two
/// @return The statistics, or nothing if the signal is shorter than one frame or the size is invalid.
std::optional<Statistics> analyze(Signal const& signal, size_t fft_size);

}  // namespace fft_vis
