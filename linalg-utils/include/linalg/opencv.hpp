#pragma once
/**
 * @file
 * Defintions for conversions from OpenCV to other image definitions inside this system.
 * @copyright Copyright 2019 (C). Erik Rainey.
 */

#include <opencv2/opencv.hpp>
#include <linalg/linalg.hpp>
#include <geometry/geometry.hpp>

/** OpenCV to \ref linalg or \ref fourcc conversion namespace */
namespace linalg {

/** Converts a ling::matrix into a cv::mat and makes the image the same dimensions as before */
linalg::matrix convert(::cv::Mat& img);

/** Converts a ling::matrix into a cv::mat and makes the image the same dimensions as before */
::cv::Mat convert(linalg::matrix& mat, int type);

/** Converts a linalg::matrix to a cv::Mat but makes it as close as possible to being a square image */
::cv::Mat squarish(linalg::matrix& mat, int type);

/** Converts a linalg::matrix to a cv::Mat by making each row a square image tile */
::cv::Mat each_row_square_tiled(linalg::matrix& mat, int type, int num_tiles_on_each_row);

/** Converts grey to JET color scale */
cv::Scalar_<uint8_t> jet(cv::Scalar_<uint8_t>& grey);

/**
 * @param [in] v The value scaled down to within 0.0 to 1.0
 */
cv::Scalar_<uint8_t> jet(double v);

/** Converts a point to another point type. */
inline cv::Point_<double> convert(const geometry::R2::point& a) {
    return cv::Point_<double>(a.x, a.y);
}

}