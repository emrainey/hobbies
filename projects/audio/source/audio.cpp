#include <audio/audio.hpp>

namespace audio {

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
bool Sequence<uint16_t, 1>::save(std::string filename) const {
    if (is_extension(filename, ".wav")) {
        return false;
    }
    return false;
}

template <>
bool Sequence<uint16_t, 2>::save(std::string filename) const {
    if (is_extension(filename, ".wav")) {
        return false;
    }
    return false;
}

}  // namespace audio