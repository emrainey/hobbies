/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Unit tests for the vision chroma key replacement algorithms
/// @copyright Copyright (c) 2026
///

#include <gtest/gtest.h>

#include <vision/chroma_replace.hpp>
#include <vision/frame_spec.hpp>
#include <vision/matting.hpp>
#include <vision/protect_spec.hpp>
#include <vision/subject_mask.hpp>

#include <opencv2/opencv.hpp>
#include <cmath>
#include <cstring>
#include <set>
#include <string>
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

namespace {

using matting::TrimapClass;

/// A green screen fading linearly into a red subject through a band of unknown pixels,
/// mimicking a soft boundary (e.g. a shadow edge is the same kind of gradual mix).
cv::Mat gradient(int rows, int cols, int bg_cols, int fg_tail) {
    cv::Mat img(rows, cols, CV_8UC3);
    uint8_t const green[3] = {30, 220, 30};  // BGR: impure, like a real screen
    uint8_t const red[3] = {10, 10, 200};    // BGR: the subject
    for (int y = 0; y < rows; ++y) {
        cv::Vec3b* row = img.ptr<cv::Vec3b>(y);
        for (int x = 0; x < cols; ++x) {
            double t;
            int const band = cols - bg_cols - fg_tail;
            if (x < bg_cols) {
                t = 0.0;
            } else if (x >= cols - fg_tail) {
                t = 1.0;
            } else {
                t = (band > 0) ? static_cast<double>(x - bg_cols) / static_cast<double>(band) : 0.5;
            }
            for (int c = 0; c < 3; ++c) {
                row[x][c] = static_cast<uint8_t>(
                    std::round(static_cast<double>(green[c]) * (1.0 - t) + static_cast<double>(red[c]) * t));
            }
        }
    }
    return img;
}

TrimapClass trimap_at(cv::Mat const& trimap, int y, int x) {
    return static_cast<TrimapClass>(trimap.at<uchar>(y, x));
}

float alpha_at(cv::Mat const& alpha, int y, int x) {
    return alpha.at<float>(y, x);
}

}  // namespace

TEST(ChromaType, ParsesClosedFormMatting) {
    EXPECT_EQ(parse_chroma_type("closedform"), ChromaType::ClosedFormMatting);
    EXPECT_EQ(parse_chroma_type("closed-form"), ChromaType::ClosedFormMatting);
    EXPECT_EQ(parse_chroma_type("CLOSED_FORM"), ChromaType::ClosedFormMatting);
}
TEST(ChromaType, ParsesBayesianMatting) {
    EXPECT_EQ(parse_chroma_type("bayesian"), ChromaType::BayesianMatting);
}
TEST(ChromaType, ParsesKnnMatting) {
    EXPECT_EQ(parse_chroma_type("knn"), ChromaType::KnnMatting);
}
TEST(ChromaType, ParsesGlobalMatting) {
    EXPECT_EQ(parse_chroma_type("global"), ChromaType::GlobalMatting);
    EXPECT_EQ(parse_chroma_type("infoflow"), ChromaType::GlobalMatting);
}

TEST(BuildTrimap, MarksDefiniteRegionsFromHsv) {
    cv::Mat img = gradient(16, 32, 8, 6);
    cv::Mat trimap = matting::build_trimap(img, named_color("green"));
    ASSERT_EQ(trimap.type(), CV_8UC1);
    EXPECT_EQ(trimap_at(trimap, 8, 4), TrimapClass::Background);
    EXPECT_EQ(trimap_at(trimap, 8, 28), TrimapClass::Foreground);
    int unknown = 0;
    for (int y = 0; y < trimap.rows; ++y)
        for (int x = 0; x < trimap.cols; ++x)
            if (trimap_at(trimap, y, x) == TrimapClass::Unknown)
                ++unknown;
    EXPECT_GT(unknown, 0);
}

TEST(Matting, ClosedFormGivesSoftGradientAlpha) {
    cv::Mat img = gradient(16, 40, 12, 10);
    cv::Mat trimap = matting::build_trimap(img, named_color("green"));
    cv::Mat alpha = matting::closed_form_matting(img, trimap);
    ASSERT_EQ(alpha.type(), CV_32FC1);
    ASSERT_EQ(alpha.size(), img.size());
    EXPECT_NEAR(alpha_at(alpha, 8, 4), 0.0, 0.02);   // definite screen
    EXPECT_NEAR(alpha_at(alpha, 8, 34), 1.0, 0.02);  // definite subject
    int in_between = 0;
    for (int x = 0; x < img.cols; ++x) {
        float const a = alpha_at(alpha, 8, x);
        EXPECT_LE(a, 1.0f);
        EXPECT_GE(a, 0.0f);
        if (a > 0.01f && a < 0.99f) {
            ++in_between;
        }
    }
    EXPECT_GT(in_between, 0);                                   // the unknown band is soft, not binary
    EXPECT_LT(alpha_at(alpha, 8, 13), alpha_at(alpha, 8, 27));  // monotonic left-to-right
}

TEST(Matting, BayesianGivesSoftGradientAlpha) {
    cv::Mat img = gradient(16, 40, 12, 10);
    cv::Mat trimap = matting::build_trimap(img, named_color("green"));
    cv::Mat alpha = matting::bayesian_matting(img, trimap);
    ASSERT_EQ(alpha.type(), CV_32FC1);
    EXPECT_NEAR(alpha_at(alpha, 8, 4), 0.0, 0.02);
    EXPECT_NEAR(alpha_at(alpha, 8, 34), 1.0, 0.02);
    int in_between = 0;
    for (int x = 0; x < img.cols; ++x) {
        float const a = alpha_at(alpha, 8, x);
        EXPECT_LE(a, 1.0f);
        EXPECT_GE(a, 0.0f);
        if (a > 0.01f && a < 0.99f) {
            ++in_between;
        }
    }
    EXPECT_GT(in_between, 0);
}

TEST(Matting, KnnGivesSoftGradientAlpha) {
    cv::Mat img = gradient(16, 40, 12, 10);
    cv::Mat trimap = matting::build_trimap(img, named_color("green"));
    cv::Mat alpha = matting::knn_matting(img, trimap);
    ASSERT_EQ(alpha.type(), CV_32FC1);
    ASSERT_EQ(alpha.size(), img.size());
    EXPECT_NEAR(alpha_at(alpha, 8, 4), 0.0, 0.06);
    EXPECT_NEAR(alpha_at(alpha, 8, 34), 1.0, 0.06);
    int in_between = 0;
    for (int x = 0; x < img.cols; ++x) {
        float const a = alpha_at(alpha, 8, x);
        EXPECT_LE(a, 1.0f);
        EXPECT_GE(a, 0.0f);
        if (a > 0.05f && a < 0.95f) {
            ++in_between;
        }
    }
    EXPECT_GT(in_between, 0);
}

#if defined(CV_VERSION_MAJOR) && (CV_VERSION_MAJOR >= 5 || (CV_VERSION_MAJOR == 4 && CV_VERSION_MINOR >= 7))
TEST(Matting, OpenCvGlobalMattingFillsTrimap) {
    cv::Mat img = gradient(16, 40, 12, 10);
    cv::Mat trimap = matting::build_trimap(img, named_color("green"));
    cv::Mat alpha = matting::global_matting(img, trimap);
    ASSERT_EQ(alpha.type(), CV_32FC1);
    ASSERT_EQ(alpha.size(), img.size());
    EXPECT_NEAR(alpha_at(alpha, 8, 4), 0.0, 0.05);
    EXPECT_NEAR(alpha_at(alpha, 8, 34), 1.0, 0.05);
    for (int x = 0; x < img.cols; ++x) {
        float const a = alpha_at(alpha, 8, x);
        EXPECT_LE(a, 1.0f);
        EXPECT_GE(a, 0.0f);
    }
}
#endif

TEST(Matting, AlphaInvertsToKeyConvention) {
    cv::Mat img = gradient(16, 40, 12, 10);
    // Matting solves for foreground alpha (1 = subject); the chroma key alpha is 1 - fg.
    cv::Mat const fg = matting::closed_form_matting(img, matting::build_trimap(img, named_color("green")));
    cv::Mat const key = compute_alpha(ChromaType::ClosedFormMatting, img, named_color("green"));
    EXPECT_NEAR(key.at<float>(8, 4), 1.0f - fg.at<float>(8, 4), 1.0e-3);
}

TEST(ChromaType, ParsesFused) {
    EXPECT_EQ(parse_chroma_type("fused"), ChromaType::FusedMatting);
    EXPECT_EQ(parse_chroma_type("hybrid"), ChromaType::FusedMatting);
}

TEST(Matting, BuildTrimapFromKeyingClassifiesScreenAndSubject) {
    cv::Mat img = gradient(16, 40, 12, 10);
    // Widen the unknown band so a linear RGB ramp has room to cross it.
    cv::Mat const trimap = matting::build_trimap_from_keying(img, named_color("green"), 0.01f, 0.10f);
    EXPECT_EQ(trimap_at(trimap, 8, 4), TrimapClass::Background);   // pure screen
    EXPECT_EQ(trimap_at(trimap, 8, 34), TrimapClass::Foreground);  // pure subject
    // The genuine transition region is where green dominance fades: over the raw ramp,
    // left pixels are still the screen, right pixels are already the subject, and the
    // green-dominance crossing (linked to where R overtakes G) is left unknown.
    int unknown = 0, background = 0, foreground = 0;
    for (int x = 12; x < 30; ++x) {
        switch (trimap_at(trimap, 8, x)) {
            case TrimapClass::Unknown:
                ++unknown;
                break;
            case TrimapClass::Background:
                ++background;
                break;
            case TrimapClass::Foreground:
                ++foreground;
                break;
            default:
                break;
        }
    }
    EXPECT_GE(background, 1);
    EXPECT_GE(foreground, 1);
    EXPECT_GE(unknown, 1);
}

TEST(Matting, FusedGivesSoftGradientAlphaAndKeepsSubject) {
    cv::Mat img = gradient(16, 40, 12, 10);
    cv::Mat const alpha = matting::fused_matting(img, named_color("green"));
    EXPECT_NEAR(alpha_at(alpha, 8, 4), 0.0, 0.02);   // definite screen is keyed
    EXPECT_NEAR(alpha_at(alpha, 8, 34), 1.0, 0.02);  // definite subject is untouched
    float previous = alpha_at(alpha, 8, 12);
    for (int x = 13; x < 30; ++x) {
        float const value = alpha_at(alpha, 8, x);
        EXPECT_GE(value, previous - 0.02f);  // soft, monotonic ramp through the band
        previous = value;
    }
    EXPECT_LT(alpha_at(alpha, 8, 13), alpha_at(alpha, 8, 27));  // no hard jump
}

namespace {

std::vector<std::string> const matting_types = {"closedform", "bayesian", "knn", "global", "fused"};

}  // namespace

TEST(ChromaReplace, MattingTypesReplaceScreenAndKeepSubject) {
    for (std::string const& type : matting_types) {
        cv::Mat img = gradient(16, 40, 12, 10);
        cv::Mat bg = solid(16, 40, cv::Vec3b{0, 255, 0});
        cv::Mat result;
        chroma_replace(img, bg, type, named_color("green"), result);
        // The definite screen becomes the flat background green...
        cv::Vec3b const screen = result.at<cv::Vec3b>(8, 4);
        EXPECT_LT(cv::norm(cv::Mat(screen, false), cv::Mat(cv::Vec3b{0, 255, 0}, false), cv::NORM_L1), 12)
            << "type: " << type;
        // ...and the definite subject survives.
        cv::Vec3b const subject = result.at<cv::Vec3b>(8, 34);
        EXPECT_LT(cv::norm(cv::Mat(subject, false), cv::Mat(cv::Vec3b{10, 10, 200}, false), cv::NORM_L1), 30)
            << "type: " << type;
    }
}

TEST(FrameSpec, SingleFrame) {
    EXPECT_EQ(vision::parse_frames("30"), (std::set<int>{30}));
}

TEST(FrameSpec, ListOfFrames) {
    EXPECT_EQ(vision::parse_frames("5,30,145"), (std::set<int>{5, 30, 145}));
}

TEST(FrameSpec, InclusiveRange) {
    EXPECT_EQ(vision::parse_frames("30-35"), (std::set<int>{30, 31, 32, 33, 34, 35}));
}

TEST(FrameSpec, MixedListAndRanges) {
    EXPECT_EQ(vision::parse_frames("3,7-10,42"), (std::set<int>{3, 7, 8, 9, 10, 42}));
}

TEST(FrameSpec, DeDuplicatesAndSorts) {
    EXPECT_EQ(vision::parse_frames("42,5,3,5,1-3"), (std::set<int>{1, 2, 3, 5, 42}));
}

TEST(FrameSpec, WhitespaceTolerated) {
    EXPECT_EQ(vision::parse_frames(" 5 , 30 - 35 "), (std::set<int>{5, 30, 31, 32, 33, 34, 35}));
}

TEST(FrameSpec, ZeroBasedRejects) {
    EXPECT_THROW(vision::parse_frames("0"), basal::exception);
    EXPECT_THROW(vision::parse_frames("-3"), basal::exception);
}

TEST(FrameSpec, EmptyRangeThrows) {
    EXPECT_THROW(vision::parse_frames("35-30"), basal::exception);
}

TEST(FrameSpec, GarbageThrows) {
    EXPECT_THROW(vision::parse_frames("abc"), basal::exception);
    EXPECT_THROW(vision::parse_frames("5,x"), basal::exception);
}

TEST(ProtectSpec, SingleRect) {
    std::vector<cv::Rect> const rects = parse_protect_rects("100,50,640,400");
    ASSERT_EQ(rects.size(), 1U);
    EXPECT_EQ(rects[0], cv::Rect(100, 50, 640, 400));
}

TEST(ProtectSpec, MultipleRects) {
    std::vector<cv::Rect> const rects = parse_protect_rects("100,50,640,400;900,300,300,500");
    ASSERT_EQ(rects.size(), 2U);
    EXPECT_EQ(rects[0], cv::Rect(100, 50, 640, 400));
    EXPECT_EQ(rects[1], cv::Rect(900, 300, 300, 500));
}

TEST(ProtectSpec, WhitespaceTolerated) {
    std::vector<cv::Rect> const rects = parse_protect_rects("  100, 50, 640, 400 ; 900,300,300,500  ");
    ASSERT_EQ(rects.size(), 2U);
    EXPECT_EQ(rects[0], cv::Rect(100, 50, 640, 400));
    EXPECT_EQ(rects[1], cv::Rect(900, 300, 300, 500));
}

TEST(ProtectSpec, EmptySpecYieldsNoRects) {
    EXPECT_EQ(parse_protect_rects("").size(), 0U);
}

TEST(ProtectSpec, GarbageThrows) {
    EXPECT_THROW(parse_protect_rects("abc"), basal::exception);
    EXPECT_THROW(parse_protect_rects("100,50,640"), basal::exception);
    EXPECT_THROW(parse_protect_rects("100,50,0,400"), basal::exception);
}

TEST(ProtectSpec, MaskHighlightsRectPixels) {
    cv::Mat const mask = build_protect_mask(100, 200, {cv::Rect(50, 20, 10, 10)});
    EXPECT_EQ(mask.type(), CV_8UC1);
    EXPECT_EQ(mask.at<uchar>(20, 50), 255);
    EXPECT_EQ(mask.at<uchar>(29, 59), 255);
    EXPECT_EQ(mask.at<uchar>(19, 50), 0);
    EXPECT_EQ(mask.at<uchar>(30, 50), 0);
}

TEST(ProtectSpec, MaskClampsOutOfRangeRects) {
    cv::Mat const mask = build_protect_mask(100, 200, {cv::Rect(190, 90, 20, 20)});
    EXPECT_EQ(mask.at<uchar>(95, 195), 255);  // inside the clipped portion
    EXPECT_EQ(mask.at<uchar>(99, 199), 255);
    EXPECT_GE(mask.total() - cv::countNonZero(mask), 4U);  // nothing outside bounds
}

TEST(ProtectSpec, EmptyRectsYieldsNoProtection) {
    cv::Mat const mask = build_protect_mask(100, 200, {});
    EXPECT_EQ(cv::countNonZero(mask), 0);
}

TEST(Matting, FusedKeepsProtectedRegionEvenWhenKeyed) {
    // A gradient that is fully screen-like (green-dominant everywhere) would normally
    // be keyed entirely; protecting a rectangle must pin it as subject.
    cv::Mat img = gradient(16, 40, 38, 0);  // green-dominant ramp across the whole width
    cv::Mat const probe = build_protect_mask(img.rows, img.cols, {cv::Rect(20, 0, 6, img.rows)});
    cv::Mat const free = matting::fused_matting(img, named_color("green"));
    cv::Mat const protected_ = matting::fused_matting(img, named_color("green"), 0.01f, 0.05f, probe);
    EXPECT_NEAR(alpha_at(protected_, 8, 22), 1.0, 0.02);                   // inside the protect band: kept
    EXPECT_NEAR(alpha_at(free, 8, 22), 0.0, 0.15);                         // without protection: keyed
    EXPECT_NEAR(alpha_at(protected_, 8, 34), alpha_at(free, 8, 34), 0.1);  // deep screen still keyed
}

TEST(Matting, FeatheredProtectSolvesSoftlyAtEdge) {
    // A fully screen-like image. A solid protect band (hard 255) pins the band subject;
    // the feathered copy leaves a grey fringe at its edge that becomes an unknown band,
    // so the matte ramps instead of stepping.
    cv::Mat img = gradient(16, 40, 38, 0);
    auto const edge_test = [&](cv::Mat const& protect, int probe_x) {
        cv::Mat const trimap = matting::build_trimap_from_keying(img, named_color("green"), 0.01f, 0.05f, protect);
        return trimap_at(trimap, 8, probe_x);
    };

    cv::Mat const solid = build_protect_mask(img.rows, img.cols, {cv::Rect(12, 0, 24, img.rows)});
    // The feathered mask has a grey band near its original edge.
    cv::Mat const soft = feather_protect_mask(solid, 3);
    EXPECT_GT(soft.at<uchar>(8, 11), 0);
    EXPECT_LT(soft.at<uchar>(8, 11), 255);
    EXPECT_EQ(soft.at<uchar>(8, 23), 255);  // interior stays solid
    // Grey fringe -> Unknown band, so the matte blends instead of a hard step.
    EXPECT_EQ(edge_test(soft, 11), TrimapClass::Unknown);
    float const outside = alpha_at(matting::fused_matting(img, named_color("green"), 0.01f, 0.05f, soft), 8, 6);
    float const interior = alpha_at(matting::fused_matting(img, named_color("green"), 0.01f, 0.05f, soft), 8, 23);
    float const fringe = alpha_at(matting::fused_matting(img, named_color("green"), 0.01f, 0.05f, soft), 8, 11);
    EXPECT_NEAR(outside, 0.0, 0.1);    // far from the protected band: keyed
    EXPECT_NEAR(interior, 1.0, 0.05);  // solid protect interior: subject
    EXPECT_GT(fringe, outside);        // the fringe is kept back a little...
    EXPECT_LT(fringe, interior);       // ...but soft, not a hard 0->1 step
}

TEST(ProtectSpec, FeatherZeroReturnsUnchanged) {
    cv::Mat const mask = build_protect_mask(32, 32, {cv::Rect(8, 8, 16, 16)});
    cv::Mat const out = feather_protect_mask(mask, 0);
    EXPECT_EQ(cv::countNonZero(mask), cv::countNonZero(out));
}

TEST(ProtectSpec, FeatherCreatesGreyFringe) {
    cv::Mat const mask = build_protect_mask(32, 32, {cv::Rect(10, 10, 12, 12)});
    cv::Mat const out = feather_protect_mask(mask, 4);
    EXPECT_EQ(out.at<uchar>(16, 16), 255);    // interior solid
    uchar const edge = out.at<uchar>(10, 9);  // just outside the rectangle
    EXPECT_GT(edge, 0);
    EXPECT_LT(edge, 255);
    EXPECT_EQ(out.at<uchar>(10, 4), 0);  // far outside untouched
}

TEST(SubjectMask, EmptyInputReturnsEmpty) {
    EXPECT_TRUE(detect_subject_mask(cv::Mat{}).empty());
}

TEST(SubjectMask, NonBgrTypeReturnsEmpty) {
    // Vision expects a CV_8UC3 BGR image; anything else bails out before the request.
    cv::Mat gray(16, 16, CV_8UC1, cv::Scalar(200));
    EXPECT_TRUE(detect_subject_mask(gray).empty());
}

TEST(SubjectMask, FeaturelessSceneYieldsNoProtection) {
    // A uniform image has no separable subject, so no mask should be produced even when
    // the Vision request itself runs (this exercises the full image->CGImage pipeline).
    cv::Mat flat(64, 64, CV_8UC3, cv::Scalar(30, 220, 30));
    EXPECT_TRUE(detect_subject_mask(flat, 0).empty());
}
