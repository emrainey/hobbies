/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Unit tests for the vision chroma key replacement algorithms
/// @copyright Copyright (c) 2026
///

#include <gtest/gtest.h>

#include <vision/chroma_replace.hpp>

#include <opencv2/opencv.hpp>
#include <vector>

using namespace vision;

namespace {

cv::Mat solid(int rows, int cols, cv::Vec3b color) {
    return cv::Mat(rows, cols, CV_8UC3, cv::Scalar(color[0], color[1], color[2]));
}

}  // namespace

TEST(ChromaType, ParsesVlahos) { EXPECT_EQ(parse_chroma_type("vlahos"), ChromaType::Vlahos); }
TEST(ChromaType, ParsesMishima) { EXPECT_EQ(parse_chroma_type("mishima"), ChromaType::Mishima); }
TEST(ChromaType, CaseInsensitive) { EXPECT_EQ(parse_chroma_type("VLAHOS"), ChromaType::Vlahos); }
TEST(ChromaType, UnknownThrows) { EXPECT_THROW(parse_chroma_type("bogus"), basal::exception); }

TEST(NamedColor, Green) { EXPECT_EQ(named_color("green"), cv::Vec3b(0, 255, 0)); }
TEST(NamedColor, Blue) { EXPECT_EQ(named_color("blue"), cv::Vec3b(255, 0, 0)); }
TEST(NamedColor, Red) { EXPECT_EQ(named_color("red"), cv::Vec3b(0, 0, 255)); }
TEST(NamedColor, CaseInsensitive) { EXPECT_EQ(named_color("GREEN"), named_color("green")); }
TEST(NamedColor, UnknownThrows) { EXPECT_THROW(named_color("bogus"), basal::exception); }
