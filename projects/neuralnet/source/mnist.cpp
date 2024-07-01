
#include "neuralnet/mnist.hpp"

namespace nn {

mnist::mnist(std::string indexes, std::string images, uint32_t limit)
    : image_file{images}
    , index_file{indexes}
    , num_entries{0}
    , fimages{nullptr}
    , findexes{nullptr}
    , labels{}
    , width{0}
    , height{0}
    , images{}
    , limit{limit} {
}

mnist::~mnist() {
}

bool mnist::load() {
    fimages = fopen(image_file.c_str(), "rb+");
    findexes = fopen(index_file.c_str(), "rb+");
    if (fimages == nullptr || findexes == nullptr) {
        return false;
    }
    uint32_t tmp = 0;
    ssize_t elem = fread(&tmp, sizeof(tmp), 1, fimages);
    if (elem == 1) {
        tmp = __builtin_bswap32(tmp);
        if (tmp != 2051) {
            printf("Failed to find magic value! Found (0x%08x)\n", tmp);
            return false;
        }
    }

    elem = fread(&tmp, sizeof(tmp), 1, findexes);
    if (elem == 1) {
        tmp = __builtin_bswap32(tmp);
        if (tmp != magic) {
            printf("Failed to find magic value! Found (0x%08x)\n", tmp);
            return false;
        }
    }

    elem = fread(&num_entries, sizeof(num_entries), 1, fimages);
    elem = fread(&tmp, sizeof(tmp), 1, findexes);
    tmp = __builtin_bswap32(tmp);
    num_entries = __builtin_bswap32(num_entries);
    if (tmp == num_entries) {
        printf("Loading %u (limit %u) entries!...", num_entries, limit);
        fflush(stdout);
    } else {
        printf("Num of Entries mismatch! %u vs %u\n", num_entries, tmp);
        return false;
    }
    printf("There are %u entries in the database\n", num_entries);
    if (limit > 0 and limit < num_entries) {
        num_entries = limit;
    }

    elem = fread(&width, sizeof(width), 1, fimages);
    elem = fread(&height, sizeof(height), 1, fimages);
    width = __builtin_bswap32(width);
    height = __builtin_bswap32(height);
    if (width != dim || height != dim) {
        printf("Failed to load image size correctly!\n");
        return false;
    }
    labels.resize(num_entries * sizeof(uint8_t));
    images.resize(num_entries * sizeof(image));
    // read in quickly!
    elem = fread(&labels[0], sizeof(uint8_t), num_entries, findexes);
    if (elem != num_entries) {
        printf("Failed to read in all labels\n");
        return false;
    }
    elem = fread(&images[0], sizeof(image), num_entries, fimages);
    if (elem != num_entries) {
        printf("Failed to read in all images\n");
        return false;
    }
    fclose(findexes);
    fclose(fimages);
    printf("Complete!\n");
    fflush(stdout);
    return true;
}

uint32_t mnist::get_num_entries() const {
    return num_entries;
}

mnist::image& mnist::get_image(size_t index) {
    if (index < num_entries) {
        return images[index];
    } else {
        static image nil;
        memset(&nil, 0, sizeof(nil));
        return nil;
    }
}

cv::Mat mnist::get_mat(size_t index) {
    if (index < num_entries) {
        return cv::Mat(dim, dim, CV_8UC1, &images[index], dim);
    } else {
        // empty
        return cv::Mat(dim, dim, CV_8UC1);
    }
}

uint8_t mnist::get_label(size_t index) {
    if (index < num_entries) {
        return labels[index];
    } else {
        return 255u;
    }
}

}  // namespace nn
