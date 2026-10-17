#include <fft_vis/fft_vis.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstdio>

#include <audio/audio.hpp>
#include <fourcc/image.hpp>
#include <iso/iso.hpp>

namespace fft_vis {

namespace {

bool valid_fft_size(size_t fft_size) {
    return fft_size >= 16U and (fft_size & (fft_size - 1U)) == 0U;
}

/// Hann windowed FFT of a frame, returning the first fft_size/2 magnitudes
std::vector<precision> magnitudes(std::vector<precision> const& samples, size_t start, size_t fft_size) {
    std::vector<std::complex<precision>> data(fft_size);
    for (size_t i = 0; i < fft_size; ++i) {
        precision w
            = 0.5_p - 0.5_p * std::cos(iso::tau * static_cast<precision>(i) / static_cast<precision>(fft_size - 1U));
        data[i] = samples[start + i] * w;
    }
    audio::fft(data);
    std::vector<precision> result(fft_size / 2U);
    for (size_t k = 0; k < result.size(); ++k) {
        result[k] = std::abs(data[k]);
    }
    return result;
}

/// Converts the fourcc heat color map (linear values) directly to an 8 bit pixel
fourcc::rgb8 heat(precision v) {
    fourcc::color c = fourcc::heat(v);
    fourcc::rgb8 pixel;
    pixel.components.r = static_cast<uint8_t>(std::round(c.red() * 255.0_p));
    pixel.components.g = static_cast<uint8_t>(std::round(c.green() * 255.0_p));
    pixel.components.b = static_cast<uint8_t>(std::round(c.blue() * 255.0_p));
    return pixel;
}

}  // namespace

template <typename SEQUENCE>
static bool convert(SEQUENCE const& seq, Signal& signal) {
    signal.sample_rate = seq.sample_rate();
    signal.channels.assign(SEQUENCE::channels, std::vector<precision>(seq.count()));
    for (size_t c = 0; c < SEQUENCE::channels; ++c) {
        for (size_t i = 0; i < seq.count(); ++i) {
            signal.channels[c][i] = static_cast<precision>(seq(c, i)) / 32768.0_p;
        }
    }
    return true;
}

bool read_wav(std::string const& filename, Signal& signal) {
    audio::StereoPCM stereo{audio::specification::cd_sample_rate, 1U};  // load replaces the contents
    if (stereo.load(filename)) {
        return convert(stereo, signal);
    }
    audio::MonoPCM mono{audio::specification::cd_sample_rate, 1U};
    if (mono.load(filename)) {
        return convert(mono, signal);
    }
    fprintf(stderr, "Could not load %s as a mono or stereo .wav file\n", filename.c_str());
    return false;
}

bool write_spectrogram(Signal const& signal, Parameters const& parameters, std::string const& filename) {
    size_t const fft_size = parameters.fft_size;
    size_t const count = signal.count();
    if (not valid_fft_size(fft_size)) {
        fprintf(stderr, "FFT size must be a power of two >= 16\n");
        return false;
    }
    if (signal.channels.empty() or signal.sample_rate <= 0.0_p or count < fft_size) {
        fprintf(stderr, "Signal is empty or shorter than the FFT size %zu\n", fft_size);
        return false;
    }
    if (parameters.max_frequency <= 0.0_p or parameters.db_range <= 0.0_p) {
        fprintf(stderr, "Max frequency and dB range must be positive\n");
        return false;
    }
    size_t const channels = signal.channels.size();
    size_t const hop = fft_size / 4U;
    size_t const frames = 1U + (count - fft_size) / hop;
    precision const bin_hz = signal.sample_rate / static_cast<precision>(fft_size);
    size_t const bins = std::min(fft_size / 2U, static_cast<size_t>(parameters.max_frequency / bin_hz) + 1U);
    // [channel][frame][bin] magnitudes
    std::vector<std::vector<std::vector<precision>>> mags(channels);
    precision peak = 1e-12_p;
    for (size_t c = 0; c < channels; ++c) {
        mags[c].resize(frames);
        for (size_t f = 0; f < frames; ++f) {
            mags[c][f] = magnitudes(signal.channels[c], f * hop, fft_size);
            mags[c][f].resize(bins);
            peak = std::max(peak, *std::max_element(mags[c][f].begin(), mags[c][f].end()));
        }
    }
    size_t const width = frames;
    size_t const height = bins * channels + (channels - 1U);  // one divider row between channels
    fourcc::image<fourcc::PixelFormat::RGB8> image{height, width};
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            image.at(y, x).components = {255U, 255U, 255U};  // divider rows stay white
        }
    }
    for (size_t c = 0; c < channels; ++c) {
        for (size_t k = 0; k < bins; ++k) {
            size_t row = c * (bins + 1U) + (bins - 1U - k);  // low frequencies at the bottom
            for (size_t f = 0; f < frames; ++f) {
                precision db = 20.0_p * std::log10(std::max(mags[c][f][k] / peak, 1e-12_p));
                image.at(row, f) = heat(1.0_p + db / parameters.db_range);
            }
        }
    }
    if (not image.save(filename)) {
        fprintf(stderr, "Could not write %s (use a .ppm or .tga extension)\n", filename.c_str());
        return false;
    }
    printf("Spectrogram %zux%zu: %zu channel(s), 0..%.0f Hz, %.0f dB range, %.3f s/column -> %s\n", width, height,
           channels, static_cast<precision>(bins - 1U) * bin_hz, parameters.db_range,
           static_cast<precision>(hop) / signal.sample_rate, filename.c_str());
    return true;
}

std::optional<Statistics> analyze(Signal const& signal, size_t fft_size) {
    if (not valid_fft_size(fft_size) or signal.channels.empty() or signal.sample_rate <= 0.0_p
        or signal.count() < fft_size) {
        return std::nullopt;
    }
    auto const& samples = signal.channels[0];
    Statistics stats;
    stats.min_centroid = stats.min_rms = 1e30_p;
    std::vector<precision> previous;
    for (size_t start = 0; start + fft_size <= samples.size(); start += fft_size) {
        std::vector<precision> magnitude = magnitudes(samples, start, fft_size);
        precision energy = 0.0_p, sum = 0.0_p, weighted = 0.0_p;
        for (size_t i = 0; i < fft_size; ++i) {
            energy += samples[start + i] * samples[start + i];
        }
        for (size_t k = 0; k < magnitude.size(); ++k) {
            sum += magnitude[k];
            weighted
                += magnitude[k] * static_cast<precision>(k) * signal.sample_rate / static_cast<precision>(fft_size);
        }
        precision centroid = weighted / std::max(sum, 1e-12_p);
        precision rms = std::sqrt(energy / static_cast<precision>(fft_size));
        stats.min_centroid = std::min(stats.min_centroid, centroid);
        stats.max_centroid = std::max(stats.max_centroid, centroid);
        stats.min_rms = std::min(stats.min_rms, rms);
        stats.max_rms = std::max(stats.max_rms, rms);
        if (not previous.empty()) {
            precision ab = 0.0_p, aa = 0.0_p, bb = 0.0_p;
            for (size_t k = 0; k < magnitude.size(); ++k) {
                ab += previous[k] * magnitude[k];
                aa += previous[k] * previous[k];
                bb += magnitude[k] * magnitude[k];
            }
            stats.min_similarity = std::min(stats.min_similarity, ab / std::sqrt(std::max(aa * bb, 1e-24_p)));
        }
        previous = std::move(magnitude);
    }
    return stats;
}

}  // namespace fft_vis
