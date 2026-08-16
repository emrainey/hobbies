/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Unit tests for the vision chroma key replacement algorithms
/// @copyright Copyright (c) 2026
///

#include <gtest/gtest.h>

#include <vision/chroma_replace.hpp>

#include <opencv2/opencv.hpp>
#include <cstring>
#include <vector>

using namespace vision;

namespace {

cv::Mat solid(int rows, int cols, cv::Vec3b color) {
    return cv::Mat(rows, cols, CV_8UC3, cv::Scalar(color[0], color[1], color[2]));
}

}  // namespace

TEST(ChromaType, ParsesVlahos) {
    EXPECT_EQ(parse_chroma_type("vlahos"), ChromaType::Vlahos);
}
TEST(ChromaType, ParsesMishima) {
    EXPECT_EQ(parse_chroma_type("mishima"), ChromaType::Mishima);
}
TEST(ChromaType, CaseInsensitive) {
    EXPECT_EQ(parse_chroma_type("VLAHOS"), ChromaType::Vlahos);
}
TEST(ChromaType, UnknownThrows) {
    EXPECT_THROW(parse_chroma_type("bogus"), basal::exception);
}

TEST(NamedColor, Green) {
    EXPECT_EQ(named_color("green"), cv::Vec3b(0, 255, 0));
}
TEST(NamedColor, Blue) {
    EXPECT_EQ(named_color("blue"), cv::Vec3b(255, 0, 0));
}
TEST(NamedColor, Red) {
    EXPECT_EQ(named_color("red"), cv::Vec3b(0, 0, 255));
}
TEST(NamedColor, CaseInsensitive) {
    EXPECT_EQ(named_color("GREEN"), named_color("green"));
}
TEST(NamedColor, UnknownThrows) {
    EXPECT_THROW(named_color("bogus"), basal::exception);
}

namespace {

/// A "realistic" green screen: bright but impure (tinted with blue and red)
const cv::Vec3b impure_screen{10, 255, 10};
/// A shadowed region of the green screen: darker and more impure
const cv::Vec3b shadow_screen{20, 180, 20};
/// A foreground color with no green at all
const cv::Vec3b foreground{0, 0, 200};

}  // namespace

TEST(ChromaReplace, VlahosSolidGreenMakesImpureScreenPure) {
    cv::Mat img = solid(16, 16, impure_screen);
    img(cv::Rect(6, 6, 4, 4)) = cv::Scalar(foreground[0], foreground[1U], foreground[2U]);
    cv::Mat result;
    chroma_replace(img, solid(16, 16, cv::Vec3b{0, 255, 0}), "vlahos", named_color("green"), result);
    // Keyed pixels become exactly the brighter, consistent pure green
    EXPECT_EQ(result.at<cv::Vec3b>(4, 4U), cv::Vec3b(0, 255, 0));
    // Foreground is preserved
    EXPECT_EQ(result.at<cv::Vec3b>(8, 8U), foreground);
}

TEST(ChromaReplace, RemapAlphaClipGain) {
    cv::Mat alpha(1, 5, CV_32FC1);
    float const values[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
    std::memcpy(alpha.ptr<float>(), values, sizeof(values));
    remap_alpha(alpha, 0.25f, 0.75f);
    EXPECT_FLOAT_EQ(alpha.at<float>(0, 0), 0.0f);
    EXPECT_FLOAT_EQ(alpha.at<float>(0, 1), 0.0f);
    EXPECT_FLOAT_EQ(alpha.at<float>(0, 2), 0.5f);
    EXPECT_FLOAT_EQ(alpha.at<float>(0, 3), 1.0f);
    EXPECT_FLOAT_EQ(alpha.at<float>(0, 4), 1.0f);
}

TEST(ChromaReplace, RemapAlphaDegenerateClips) {
    cv::Mat alpha(1, 2, CV_32FC1);
    alpha.at<float>(0, 0) = 0.0f;
    alpha.at<float>(0, 1) = 1.0f;
    remap_alpha(alpha, 0.7f, 0.3f);  // clip_white below clip_black -> collapsed to 0
    EXPECT_FLOAT_EQ(alpha.at<float>(0, 0), 0.0f);
    EXPECT_FLOAT_EQ(alpha.at<float>(0, 1), 0.0f);
}

TEST(ChromaReplace, VlahosSolidGreenBrightensShadowedScreen) {
    cv::Mat img = solid(16, 16, shadow_screen);
    cv::Mat result;
    chroma_replace(img, solid(16, 16, cv::Vec3b{0, 255, 0}), "vlahos", named_color("green"), result);
    cv::Vec3b const before = shadow_screen;
    cv::Vec3b const after = result.at<cv::Vec3b>(8, 8U);
    EXPECT_GT(after[1U], before[1U]);  // Brighter green
    EXPECT_LT(after[0U], before[0U]);  // Less blue contamination
    EXPECT_LT(after[2U], before[2U]);  // Less red contamination
}

TEST(ChromaReplace, MishimaSolidGreenMakesImpureScreenPure) {
    cv::Mat img = solid(32, 32, impure_screen);
    cv::Mat result;
    chroma_replace(img, solid(32, 32, cv::Vec3b{0, 255, 0}), "mishima", named_color("green"), result);
    cv::Scalar const m = cv::mean(result);
    EXPECT_GE(m[1U], 254.0);  // Fully green
    EXPECT_LE(m[0U], 1.0);    // No blue contamination
    EXPECT_LE(m[2U], 1.0);    // No red contamination
}

TEST(ChromaKeyOut, MatchesSolidBackgroundCompositing) {
    cv::Mat img = solid(16, 16, impure_screen);
    img(cv::Rect(6, 6, 4, 4)) = cv::Scalar(foreground[0], foreground[1U], foreground[2U]);
    for (std::string const& type : {"vlahos", "mishima"}) {
        cv::Mat a, b;
        key_out(img, type, named_color("green"), a);
        chroma_replace(img, solid(16, 16, cv::Vec3b{0, 255, 0}), type, named_color("green"), b);
        EXPECT_DOUBLE_EQ(cv::norm(a, b, cv::NORM_INF), 0.0) << "mismatch for type: " << type;
    }
}

TEST(ChromaKeyOut, GreenScreenBecomesPureGreenAndForegroundPreserved) {
    cv::Mat img = solid(16, 16, impure_screen);
    img(cv::Rect(6, 6, 4, 4)) = cv::Scalar(foreground[0], foreground[1U], foreground[2U]);
    cv::Mat result;
    key_out(img, "vlahos", named_color("green"), result);
    EXPECT_EQ(result.at<cv::Vec3b>(4, 4U), cv::Vec3b(0, 255, 0));
    EXPECT_EQ(result.at<cv::Vec3b>(8, 8U), foreground);
}

TEST(ChromaKeyOut, DefaultClipMatchesExplicitNoOpClip) {
    cv::Mat img = solid(16, 16, shadow_screen);
    cv::Mat a, b;
    key_out(img, "vlahos", named_color("green"), a);
    key_out(img, "vlahos", named_color("green"), b, 0.0f, 1.0f);
    EXPECT_DOUBLE_EQ(cv::norm(a, b, cv::NORM_INF), 0.0);
}

TEST(ChromaKeyOut, ClipWhiteBrightensShadowedScreenToPureGreen) {
    cv::Mat img = solid(16, 16, shadow_screen);
    cv::Mat result;
    key_out(img, "vlahos", named_color("green"), result, 0.0f, 0.6f);
    EXPECT_EQ(result.at<cv::Vec3b>(8, 8U), cv::Vec3b(0, 255, 0));
}

TEST(ChromaKeyOut, ClipBlackKeepsWeakGreenInForeground) {
    cv::Mat img = solid(16, 16, cv::Vec3b{10, 40, 20});
    cv::Mat result;
    key_out(img, "vlahos", named_color("green"), result, 0.1f, 1.0f);
    EXPECT_EQ(result.at<cv::Vec3b>(8, 8U), cv::Vec3b(10, 40, 20));
}

TEST(ChromaKeyOut, BlueScreenBecomesPureBlue) {
    cv::Mat img = solid(16, 16, cv::Vec3b{200, 10, 20});  // Impure blue screen
    img(cv::Rect(6, 6, 4, 4)) = cv::Scalar(foreground[0], foreground[1U], foreground[2U]);
    cv::Mat result;
    key_out(img, "vlahos", named_color("blue"), result, 0.0f, 0.6f);
    EXPECT_EQ(result.at<cv::Vec3b>(4, 4U), cv::Vec3b(255, 0, 0));
    EXPECT_EQ(result.at<cv::Vec3b>(8, 8U), foreground);
}
