#include <audio/audio.hpp>

namespace audio {

precision waveform(iso::hertz frequency, iso::seconds offset) {
    return 0.5_p * std::sin(iso::tau * frequency.value * offset.value);
}

namespace equal_temperament {

iso::hertz ToFrequency(Tone note, int octave) {
    int position = ToPianoKey(note, octave);
    iso::hertz frequency{440.0_p};
    precision scaling = std::pow(twelveth_root_of_2, (position - ToPianoKey(Tone::A, 4)));
    frequency *= scaling;
    return frequency;
}

}  // namespace equal_temperament

namespace wav {
struct chunk {
    char id[4];     ///< The four character code for the chunk.
    uint32_t size;  ///< Counts the number of bytes in the chunk not counting the chunk itself.
};
struct header {
    chunk riff = {{'R', 'I', 'F', 'F'}, 0U};
    char const tag[4] = {'W', 'A', 'V', 'E'};
};
template <typename SAMPLE_TYPE, size_t CHANNELS>
struct fmt {
    chunk fmt = {{'f', 'm', 't', ' '}, sizeof(fmt) - sizeof(chunk)};
    uint16_t format{1U};  // 1 = PCM, 3 = Float
    uint16_t const channels{CHANNELS};
    uint32_t sample_rate{static_cast<uint32_t>(specification::cd_sample_rate)};
    uint32_t bytes_per_sec{0U};
    uint16_t const bytes_per_block{CHANNELS * sizeof(SAMPLE_TYPE)};
    uint16_t const bits_per_sample{sizeof(SAMPLE_TYPE) * 8U};
};
static_assert(sizeof(header) + sizeof(fmt<uint16_t, 1>) == 36U, "WAV header size is incorrect");

}  // namespace wav

static bool is_extension(std::string filename, std::string ext) {
    // std::filesystem::path fp = filename;
    // return fp.extension() == ext;
    char const* pext = strrchr(filename.c_str(), '.');
    if (strcmp(pext, ext.c_str()) == 0) {
        return true;
    }
    return false;
}

template <>
bool Sequence<uint16_t, 1>::load(std::string filename) {
    if (is_extension(filename, ".wav")) {
        return false;
    }
    return false;
}

template <>
bool Sequence<uint16_t, 2>::load(std::string filename) {
    if (is_extension(filename, ".wav")) {
        return false;
    }
    return false;
}

template <>
bool Sequence<int16_t, 1>::save(std::string filename) const {
    if (is_extension(filename, ".wav")) {
        // write out a .wav file
        FILE* fp = fopen(filename.c_str(), "wb");
        if (fp) {
            wav::header hdr;
            wav::fmt<sample, channels> fmt;
            fmt.fmt.size = sizeof(fmt) - sizeof(wav::chunk);
            fmt.format = 1U;  // PCM
            fmt.sample_rate = static_cast<uint32_t>(sample_rate());
            fmt.bytes_per_sec = fmt.sample_rate * fmt.bytes_per_block;
            wav::chunk data = {{'d', 'a', 't', 'a'}, 0U};
            data.size = static_cast<uint32_t>(count() * channels * sizeof(sample));
            hdr.riff.size = sizeof(hdr) + sizeof(fmt) + sizeof(data) + data.size - sizeof(wav::chunk);
            // write the data to file
            fwrite(&hdr, sizeof(hdr), 1, fp);
            fwrite(&fmt, sizeof(fmt), 1, fp);
            fwrite(&data, sizeof(data), 1, fp);
            for_each([&](size_t, size_t, precision, sample const& s) {
                // write the data to file
                fwrite(&s, sizeof(sample), 1, fp);
            });
            fclose(fp);
            return true;
        }
    }
    return false;
}

template <>
bool Sequence<int16_t, 2>::save(std::string filename) const {
    if (is_extension(filename, ".wav")) {
        // write out a .wav file
        FILE* fp = fopen(filename.c_str(), "wb");
        if (fp) {
            wav::header hdr;
            wav::fmt<sample, channels> fmt;
            fmt.fmt.size = sizeof(fmt) - sizeof(wav::chunk);
            fmt.format = 1U;  // PCM
            fmt.sample_rate = static_cast<uint32_t>(sample_rate());
            fmt.bytes_per_sec = fmt.sample_rate * fmt.bytes_per_block;
            wav::chunk data = {{'d', 'a', 't', 'a'}, 0U};
            data.size = static_cast<uint32_t>(count() * channels * sizeof(sample));
            hdr.riff.size = sizeof(hdr) + sizeof(fmt) + sizeof(data) + data.size - sizeof(wav::chunk);
            // write the data to file
            fwrite(&hdr, sizeof(hdr), 1, fp);
            fwrite(&fmt, sizeof(fmt), 1, fp);
            fwrite(&data, sizeof(data), 1, fp);
            for_each([&](size_t, size_t, precision, sample const& s) {
                // write the data to file
                fwrite(&s, sizeof(sample), 1, fp);
            });
            fclose(fp);
            return true;
        }
    }
    return false;
}

iso::seconds SecondsPerBeat(Tempo tempo) {
    // 60 BPM is 1 second per beat
    return iso::seconds{60.0_p / static_cast<precision>(tempo)};
}

iso::seconds SecondsPerDuration(Tempo tempo, Signature signature, Duration length) {
    // 60 BPM @ 3/4 time is 1 second per quarter note, 2 seconds per half note, 3 seconds per dotted half note
    // 60 BPM @ 4/4 time is also 1 second per quarter note, 2 seconds per half note, 4 seconds per whole note
    precision ratio = static_cast<precision>(basal::to_underlying(signature.value))
                      / static_cast<precision>(basal::to_underlying(length));
    auto spb = SecondsPerBeat(tempo);
    spb *= ratio;
    return spb;
}

std::vector<basal::precision> envelope(std::vector<basal::precision> const& input) {
    std::vector<basal::precision> output;
    output.reserve(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        precision fract = static_cast<precision>(i) / static_cast<precision>(input.size());
        output.push_back(input[i] * envelope(fract));
    }
    return output;
}

basal::precision envelope(basal::precision ratio) {
    basal::exception::throw_unless(ratio >= 0.0_p && ratio <= 1.0_p, __FILE__, __LINE__,
                                   "Ratio must be between 0 and 1");
    return (0.5_p + 0.5_p * std::cos(iso::tau * ratio + iso::pi));
}

namespace out {
basal::precision fade(basal::precision ratio) {
    basal::exception::throw_unless(ratio >= 0.0_p && ratio <= 1.0_p, __FILE__, __LINE__,
                                   "Ratio must be between 0 and 1");
    return (1.0_p - ratio);
}

basal::precision hill(basal::precision ratio) {
    basal::exception::throw_unless(ratio >= 0.0_p && ratio <= 1.0_p, __FILE__, __LINE__,
                                   "Ratio must be between 0 and 1");
    return (1.0_p - exp(5.0_p * (ratio - 1.0_p)));
}

basal::precision cliff(basal::precision ratio) {
    basal::exception::throw_unless(ratio >= 0.0_p && ratio <= 1.0_p, __FILE__, __LINE__,
                                   "Ratio must be between 0 and 1");
    return exp(-ratio * 5.7_p);
}

basal::precision ramp(basal::precision ratio) {
    basal::exception::throw_unless(ratio >= 0.0_p && ratio <= 1.0_p, __FILE__, __LINE__,
                                   "Ratio must be between 0 and 1");
    return (1.0_p - sqrt(ratio));
}
}  // namespace out

namespace in {

basal::precision fade(basal::precision ratio) {
    basal::exception::throw_unless(ratio >= 0.0_p && ratio <= 1.0_p, __FILE__, __LINE__,
                                   "Ratio must be between 0 and 1");
    return ratio;
}

basal::precision ramp(basal::precision ratio) {
    basal::exception::throw_unless(ratio >= 0.0_p && ratio <= 1.0_p, __FILE__, __LINE__,
                                   "Ratio must be between 0 and 1");
    return sqrt(ratio);
}
}  // namespace in

basal::precision pop(basal::precision ratio) {
    basal::exception::throw_unless(ratio >= 0.0_p && ratio <= 1.0_p, __FILE__, __LINE__,
                                   "Ratio must be between 0 and 1");
    return 2.0_p * (std::pow(ratio, ratio / 12.0_p) - std::pow(ratio, sqrt(ratio)));
}

template <>
MonoPCM Mixer<MonoPCM>::even(MonoPCM const& a, MonoPCM const& b) {
    // mix the two sequences together
    size_t const count = std::min(a.count(), b.count());
    MonoPCM result{a.sample_rate(), count};
    result.for_each([&](size_t c, size_t i, precision) -> MonoPCM::sample {
        int32_t va = a(c, i);
        int32_t vb = b(c, i);
        int16_t v = static_cast<int16_t>((va + vb) / 2);
        return v;
    });
    return result;
}

template <>
StereoPCM Mixer<StereoPCM>::even(StereoPCM const& a, StereoPCM const& b) {
    // mix the two sequences together
    size_t const count = std::min(a.count(), b.count());
    StereoPCM result{a.sample_rate(), count};
    result.for_each([&](size_t c, size_t i, precision) -> StereoPCM::sample {
        int32_t va = a(c, i);
        int32_t vb = b(c, i);
        int16_t v = static_cast<int16_t>((va + vb) / 2);
        return v;
    });
    return result;
}

}  // namespace audio