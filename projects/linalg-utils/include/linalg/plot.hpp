#pragma once
/// @file
/// Defintions for conversions from OpenCV to other other libraries.
/// @copyright Copyright 2019 (C). Erik Rainey.

#include <geometry/point.hpp>
#include <opencv2/opencv.hpp>

namespace linalg {

/// A function to plot points in an OpenCV image and display it
void plot_points(std::string name, const std::vector<geometry::point_<2>> &data, const std::vector<size_t> &indexes,
                 const std::vector<cv::Scalar> &colors, int timeout_ms = 0);

}  // namespace linalg
