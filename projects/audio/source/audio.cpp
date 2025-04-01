#include <audio/audio.hpp>

namespace audio {

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
    uint32_t sample_rate{44'100U};
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

}  // namespace audio