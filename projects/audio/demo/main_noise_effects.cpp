/// @file
/// Generates audio effects driven by the noise library and writes them to a stereo .wav file.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include <audio/audio.hpp>
#include <basal/basal.hpp>
#include <basal/options.hpp>
#include <fft_vis/fft_vis.hpp>
#include <noise/noise.hpp>

using namespace basal::literals;
using basal::precision;

namespace {

constexpr size_t channels = 2U;
constexpr precision channel_spread = 17.3_p;
/// turbulentsin divides its inputs by the pad dimensions, so scale time by this to make --rate mean pattern cycles/sec
constexpr precision pad_scale
    = static_cast<precision>(noise::pad::dimensions);  // y offset in noise space which decorrelates the channels

/// A buffer of floating point samples per channel before normalization
using Buffer = std::array<std::vector<precision>, channels>;

struct Settings {
    precision sample_rate;
    precision period;     ///< seconds
    precision frequency;  ///< Hz
    precision rate;       ///< how fast the noise field is traversed (noise cells per second)
    precision depth;      ///< modulation depth
    precision power;      ///< turbulentsin power
    precision size;       ///< turbulentsin initial size
    precision seed;       ///< seed angle in radians
    size_t octaves;       ///< fbm octaves
    size_t count;         ///< number of samples per channel
};

noise::pad const& pad() {
    static noise::pad const instance;
    return instance;
}

/// Evaluates the turbulentsin at a time and y position
precision turbsin(Settings const& s, precision x, precision y) {
    return noise::turbulentsin(noise::point{x, y}, 1.0_p, 1.0_p, s.power, s.size, 1.0_p, pad());
}

/// Sine whose phase is wobbled by turbulentsin noise.
void effect_drone(Settings const& s, Buffer& out) {
    for (size_t c = 0; c < channels; ++c) {
        for (size_t i = 0; i < s.count; ++i) {
            precision t = static_cast<precision>(i) / s.sample_rate;
            precision n = turbsin(s, t * s.frequency * 0.5_p,
                                  t * s.rate * pad_scale + static_cast<precision>(c) * channel_spread);
            out[c][i] = std::sin(iso::tau * s.frequency * t + s.depth * n);
        }
    }
}

/// Wind or water like texture from fractal brownian motion. The frequency controls the brightness.
/// A slow fbm gust envelope shapes the amplitude.
void effect_wind(Settings const& s, Buffer& out) {
    noise::vector seed = noise::convert_to_seed(iso::radians{s.seed});
    for (size_t c = 0; c < channels; ++c) {
        precision y = static_cast<precision>(c) * channel_spread;
        for (size_t i = 0; i < s.count; ++i) {
            precision t = static_cast<precision>(i) / s.sample_rate;
            precision body = noise::fractal_brownian(noise::point{t * s.frequency, y}, seed, s.octaves);
            precision gust = noise::fractal_brownian(noise::point{t * s.rate, y + 100.0_p}, seed, 3U);
            out[c][i] = body * (1.0_p + s.depth * gust);
        }
    }
}

/// Crackling or bubbling built from the first difference of worley noise (cell edges become clicks).
void effect_crackle(Settings const& s, Buffer& out) {
    noise::vector seed = noise::convert_to_seed(iso::radians{s.seed});
    for (size_t c = 0; c < channels; ++c) {
        precision y = static_cast<precision>(c) * channel_spread;
        precision previous = 0.0_p;
        for (size_t i = 0; i < s.count; ++i) {
            precision t = static_cast<precision>(i) / s.sample_rate;
            precision w = noise::worley(noise::point{t * s.frequency, y}, 1.0_p, seed, 1.0_p);
            out[c][i] = w - previous;
            previous = w;
        }
    }
}

/// A sine carrier with slow simplex noise driving vibrato, tremolo and a stereo pan.
void effect_lfo(Settings const& s, Buffer& out) {
    noise::vector seed = noise::convert_to_seed(iso::radians{s.seed});
    auto lfo
        = [&](precision t, precision y) { return noise::simplex(noise::point{t * s.rate, y}, 1.0_p, seed, 1.0_p); };
    precision phase = 0.0_p;
    precision const vibrato_range = 0.02_p * s.depth;  // fraction of the base frequency
    precision const tremolo_depth = std::clamp(0.3_p * s.depth, 0.0_p, 0.9_p);
    for (size_t i = 0; i < s.count; ++i) {
        precision t = static_cast<precision>(i) / s.sample_rate;
        precision vibrato = lfo(t, 1.0_p);
        precision tremolo = lfo(t, 2.0_p) * 0.5_p + 0.5_p;                        // roughly [0,1]
        precision pan = std::clamp(lfo(t, 3.0_p) * 0.5_p + 0.5_p, 0.0_p, 1.0_p);  // 0 left, 1 right
        // integrate the instantaneous frequency so the vibrato is a real frequency deviation
        phase += s.frequency * (1.0_p + vibrato_range * vibrato) / s.sample_rate;
        phase -= std::floor(phase);
        precision tone = std::sin(iso::tau * phase);
        precision amp = 1.0_p - tremolo_depth * (1.0_p - std::clamp(tremolo, 0.0_p, 1.0_p));
        out[0][i] = tone * amp * std::cos(pan * iso::pi / 2.0_p);
        out[1][i] = tone * amp * std::sin(pan * iso::pi / 2.0_p);
    }
}

/// Oscillator which plays single cycle wave tables cut from slices of turbulentsin, morphing between slices.
void effect_wavetable(Settings const& s, Buffer& out) {
    constexpr size_t table_size = 512U;
    using Table = std::array<precision, table_size>;
    std::map<std::pair<size_t, size_t>, Table> cache;
    auto get_table = [&](size_t slice, size_t c) -> Table const& {
        auto key = std::make_pair(slice, c);
        auto it = cache.find(key);
        if (it != cache.end()) {
            return it->second;
        }
        Table table;
        precision y = static_cast<precision>(slice) * pad_scale / 2.0_p + static_cast<precision>(c) * channel_spread;
        precision mean = 0.0_p;
        for (size_t k = 0; k < table_size; ++k) {
            table[k] = turbsin(s, static_cast<precision>(k) * 0.25_p, y);
            mean += table[k];
        }
        mean /= static_cast<precision>(table_size);
        // make the cycle seamless by removing the ramp between the first and last values
        precision ramp = table[table_size - 1] - table[0];
        precision peak = 1e-9_p;
        for (size_t k = 0; k < table_size; ++k) {
            table[k] -= mean + ramp * (static_cast<precision>(k) / static_cast<precision>(table_size) - 0.5_p);
            peak = std::max(peak, std::abs(table[k]));
        }
        for (auto& v : table) {
            v /= peak;
        }
        return cache.emplace(key, table).first->second;
    };
    for (size_t c = 0; c < channels; ++c) {
        precision phase = 0.0_p;
        for (size_t i = 0; i < s.count; ++i) {
            precision t = static_cast<precision>(i) / s.sample_rate;
            precision morph = t * s.rate;
            size_t slice = static_cast<size_t>(morph);
            precision mix = morph - std::floor(morph);
            precision pos = phase * static_cast<precision>(table_size);
            size_t k0 = static_cast<size_t>(pos) % table_size;
            size_t k1 = (k0 + 1U) % table_size;
            precision frac = pos - std::floor(pos);
            auto sample_of = [&](size_t sl) {
                Table const& tb = get_table(sl, c);
                return tb[k0] * (1.0_p - frac) + tb[k1] * frac;
            };
            out[c][i] = sample_of(slice) * (1.0_p - mix) + sample_of(slice + 1U) * mix;
            phase += s.frequency / s.sample_rate;
            phase -= std::floor(phase);
        }
    }
}

using Effect = std::function<void(Settings const&, Buffer&)>;

struct Entry {
    char const* name;
    char const* description;
    Effect effect;
};

std::vector<Entry> const& effects() {
    static std::vector<Entry> const list = {
        {"drone", "sine with turbulentsin phase modulation (--depth, --power, --size, --rate)", effect_drone},
        {"wind", "fbm noise texture, --frequency is brightness, --rate/--depth shape gusts", effect_wind},
        {"crackle", "worley noise edges as crackles, --frequency is density", effect_crackle},
        {"lfo", "simplex driven vibrato, tremolo and pan on a sine (--rate, --depth)", effect_lfo},
        {"wavetable", "turbulentsin slices as morphing wavetable (--rate is morphs/sec)", effect_wavetable},
    };
    return list;
}

/// Removes DC offset, applies short fades and scales to the requested peak level (in place)
void normalize(Settings const& s, Buffer& buffer, precision level) {
    precision peak = 1e-9_p;
    size_t fade = std::min(static_cast<size_t>(0.02_p * s.sample_rate), s.count / 2U);
    for (auto& channel : buffer) {
        precision mean = 0.0_p;
        for (auto v : channel) {
            mean += v;
        }
        mean /= static_cast<precision>(channel.size());
        for (size_t i = 0; i < channel.size(); ++i) {
            channel[i] -= mean;
            if (i < fade) {
                channel[i] *= static_cast<precision>(i) / static_cast<precision>(fade);
            } else if (i >= channel.size() - fade) {
                channel[i] *= static_cast<precision>(channel.size() - 1U - i) / static_cast<precision>(fade);
            }
            peak = std::max(peak, std::abs(channel[i]));
        }
    }
    for (auto& channel : buffer) {
        for (auto& v : channel) {
            v = v / peak * level;
        }
    }
}

void print_effects() {
    printf("Effects:\n");
    for (auto const& e : effects()) {
        printf("\t%-10s %s\n", e.name, e.description);
    }
}

}  // namespace

int main(int argc, char* argv[]) {
    basal::options::config opts[] = {
        {"-e", "--effect", std::string("drone"), "Effect name (see list below)"},
        {"-o", "--output", std::string("noise_effect.wav"), "Output .wav file"},
        {"-s", "--sample-rate", 44100.0_p, "Sample rate in Hz"},
        {"-p", "--period", 5.0_p, "Duration in seconds"},
        {"-f", "--frequency", 220.0_p, "Base frequency in Hz"},
        {"-r", "--rate", 2.0_p, "Noise traversal rate (cells/sec, or morphs/sec for wavetable)"},
        {"-d", "--depth", 4.0_p, "Modulation depth"},
        {"-w", "--power", 2.0_p, "turbulentsin power"},
        {"-z", "--size", 32.0_p, "turbulentsin initial size"},
        {"-a", "--seed", 0.5_p, "Seed angle in radians for fbm/simplex/worley"},
        {"-c", "--octaves", size_t{5U}, "fbm octaves"},
        {"-l", "--level", 0.8_p, "Peak output level (0..1)"},
        {"-n", "--analyze", false, "Print FFT based variation statistics of the rendered effect"},
        {"-g", "--spectrogram", std::string(""), "Write an FFT spectrogram image (.ppm or .tga) of the effect"},
        {"-t", "--fft-size", size_t{2048U}, "Spectrogram FFT size (power of two)"},
        {"-m", "--max-freq", 8000.0_p, "Spectrogram top frequency in Hz"},
        {"-b", "--db-range", 80.0_p, "Spectrogram dynamic range in dB"},
        {"-h", "--help", false, "Show this help"},
    };
    basal::options::process(basal::dimof(opts), opts, argc, argv);

    std::string name = std::get<std::string>(opts[0].value);
    std::string filename = std::get<std::string>(opts[1].value);
    Settings settings{};
    settings.sample_rate = std::get<precision>(opts[2].value);
    settings.period = std::get<precision>(opts[3].value);
    settings.frequency = std::get<precision>(opts[4].value);
    settings.rate = std::get<precision>(opts[5].value);
    settings.depth = std::get<precision>(opts[6].value);
    settings.power = std::get<precision>(opts[7].value);
    settings.size = std::get<precision>(opts[8].value);
    settings.seed = std::get<precision>(opts[9].value);
    settings.octaves = std::get<size_t>(opts[10].value);
    precision level = std::clamp(std::get<precision>(opts[11].value), 0.0_p, 1.0_p);

    if (std::get<bool>(opts[17].value)) {
        basal::options::print(basal::dimof(opts), opts);
        print_effects();
        return 0;
    }
    if (settings.sample_rate <= 0.0_p or settings.period <= 0.0_p or settings.frequency <= 0.0_p
        or settings.octaves == 0U) {
        printf("Sample rate, period, frequency and octaves must be positive\n");
        return 1;
    }
    settings.count = static_cast<size_t>(settings.period * settings.sample_rate);
    if (settings.count == 0U) {
        printf("Period is too short for the sample rate\n");
        return 1;
    }

    auto const& list = effects();
    auto it = std::find_if(list.begin(), list.end(), [&](Entry const& e) { return name == e.name; });
    if (it == list.end()) {
        printf("Unknown effect '%s'\n", name.c_str());
        print_effects();
        return 1;
    }

    printf("Rendering '%s' %lf s @ %lf Hz, base %lf Hz -> %s\n", name.c_str(), settings.period, settings.sample_rate,
           settings.frequency, filename.c_str());
    Buffer buffer;
    for (auto& channel : buffer) {
        channel.assign(settings.count, 0.0_p);
    }
    it->effect(settings, buffer);
    normalize(settings, buffer, level);

    audio::StereoPCM seq{settings.sample_rate, settings.count};
    seq.for_each([&](size_t c, size_t i, precision) -> audio::StereoPCM::sample {
        return audio::StereoPCM::convert(buffer[c][i]);
    });
    if (not seq.save(filename)) {
        printf("Failed to save %s (must end in .wav)\n", filename.c_str());
        return 1;
    }

    // The same visualization the demo_fft_vis tool applies to any .wav file
    fft_vis::Signal signal;
    signal.sample_rate = settings.sample_rate;
    signal.channels.assign(buffer.begin(), buffer.end());
    if (std::get<bool>(opts[12].value)) {
        auto stats = fft_vis::analyze(signal, 8192U);
        if (stats) {
            printf("Analysis: centroid %.1f..%.1f Hz, rms %.4f..%.4f, min frame-to-frame spectral similarity %.3f\n",
                   stats->min_centroid, stats->max_centroid, stats->min_rms, stats->max_rms, stats->min_similarity);
        } else {
            printf("Too short to analyze\n");
        }
    }
    std::string spectrogram = std::get<std::string>(opts[13].value);
    if (not spectrogram.empty()) {
        fft_vis::Parameters parameters;
        parameters.fft_size = std::get<size_t>(opts[14].value);
        parameters.max_frequency = std::get<precision>(opts[15].value);
        parameters.db_range = std::get<precision>(opts[16].value);
        if (not fft_vis::write_spectrogram(signal, parameters, spectrogram)) {
            return 1;
        }
    }
    return 0;
}
