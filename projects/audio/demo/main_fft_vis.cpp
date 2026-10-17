/// @file
/// Reads any .wav file and writes a spectrogram image of it.

#include <cstdio>
#include <string>

#include <basal/basal.hpp>
#include <basal/options.hpp>
#include <fft_vis/fft_vis.hpp>

using namespace basal::literals;
using basal::precision;

int main(int argc, char* argv[]) {
    basal::options::config opts[] = {
        {"-i", "--input", std::string(""), "Input .wav file"},
        {"-g", "--spectrogram", std::string("spectrogram.ppm"), "Output spectrogram image (.ppm or .tga)"},
        {"-t", "--fft-size", size_t{2048U}, "FFT size (power of two)"},
        {"-m", "--max-freq", 8000.0_p, "Top frequency in Hz"},
        {"-b", "--db-range", 80.0_p, "Dynamic range in dB"},
        {"-n", "--analyze", false, "Print FFT based variation statistics"},
        {"-h", "--help", false, "Show this help"},
    };
    basal::options::process(basal::dimof(opts), opts, argc, argv);
    std::string input = std::get<std::string>(opts[0].value);
    if (std::get<bool>(opts[6].value) or input.empty()) {
        basal::options::print(basal::dimof(opts), opts);
        return std::get<bool>(opts[6].value) ? 0 : 1;
    }
    fft_vis::Signal signal;
    if (not fft_vis::read_wav(input, signal)) {
        return 1;
    }
    printf("Read %s: %zu channel(s), %zu samples @ %.0f Hz\n", input.c_str(), signal.channels.size(), signal.count(),
           signal.sample_rate);
    fft_vis::Parameters parameters;
    parameters.fft_size = std::get<size_t>(opts[2].value);
    parameters.max_frequency = std::get<precision>(opts[3].value);
    parameters.db_range = std::get<precision>(opts[4].value);
    if (std::get<bool>(opts[5].value)) {
        auto stats = fft_vis::analyze(signal, 8192U);
        if (stats) {
            printf("Analysis: centroid %.1f..%.1f Hz, rms %.4f..%.4f, min frame-to-frame spectral similarity %.3f\n",
                   stats->min_centroid, stats->max_centroid, stats->min_rms, stats->max_rms, stats->min_similarity);
        } else {
            printf("Too short to analyze\n");
        }
    }
    return fft_vis::write_spectrogram(signal, parameters, std::get<std::string>(opts[1].value)) ? 0 : 1;
}
