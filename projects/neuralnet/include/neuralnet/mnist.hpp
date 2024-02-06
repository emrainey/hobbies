#pragma once
/// @file
/// Definitions of the objects used with MNIST database objects.
/// @copyright Copyright 2019 (C) Erik Rainey.

#include <algorithm>
#include <opencv2/opencv.hpp>

#include "basal/ieee754.hpp"

using namespace basal::literals;

namespace nn {
/// Loads the MNIST data set. Scans the entire set and generates indexes to cover all images.
class mnist {
public:
    /// This dataset is all 28x28 images.
    constexpr static const uint32_t dim = 28;
    /// The number of inputs a network would need to cover all the possible values
    constexpr static const uint32_t inputs = dim * dim;

    /// The storage for each image in the dataset.
    struct image {
        /// This is organized as [cols][rows] for some reason.
        uint8_t data[dim][dim];
    };

    mnist(std::string indexes, std::string images, uint32_t limit = 0);
    virtual ~mnist();

    /// Loads the images from the database files
    bool load();

    /// Indicates the number of entries after loading
    uint32_t get_num_entries() const;

    /// Retrieves an image (28x28) from the dataset
    image &get_image(size_t index);

    /// Gets a cv::Mat of the same image from the dataset
    cv::Mat get_mat(size_t index);

    /// Gets the label from the labels dataset
    uint8_t get_label(size_t index);

protected:
    constexpr static const uint32_t magic = 2049;  ///< Just some number they picked.
    std::string image_file;
    std::string index_file;
    uint32_t num_entries;
    FILE *fimages;
    FILE *findexes;
    std::vector<uint8_t> labels;
    uint32_t width;
    uint32_t height;
    std::vector<image> images;
    uint32_t limit;
};

}  // namespace nn
