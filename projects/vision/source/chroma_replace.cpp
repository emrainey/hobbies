#include "vision/chroma_replace.hpp"
#include "vision/matting.hpp"

#include <opencv2/opencv.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

namespace vision {
namespace detail {

inline void validate_image(cv::Mat const& image, std::string const& name) {
    if (image.empty() || image.type() != CV_8UC3) {
        throw std::invalid_argument(name + " must be a non-empty 8-bit 3-channel (CV_8UC3) image");
    }
}

/// Index of the dominant channel of the key color (0=B, 1=G, 2=R)
[[nodiscard]] inline int dominant_channel(cv::Vec3b const& key) {
    int dominant = 0;
    if (key[1U] > key[dominant]) {
        dominant = 1;
    }
    if (key[2] > key[dominant]) {
        dominant = 2;
    }
    return dominant;
}

}  // namespace detail

[[nodiscard]] cv::Mat vlahos_alpha(cv::Mat const& image, cv::Vec3b key) {
    detail::validate_image(image, "image");
    int const dominant = detail::dominant_channel(key);
    cv::Mat const* channels[3];
    cv::Mat split_channels[3];
    cv::split(image, split_channels);
    for (int c = 0; c < 3; ++c) {
        channels[c] = &split_channels[c];
    }
    int const other[2] = {(dominant + 1) % 3, (dominant + 2) % 3};
    cv::Mat other_max = cv::max(*channels[other[0]], *channels[other[1U]]);
    cv::Mat alpha;
    cv::subtract(*channels[dominant], other_max, alpha);  // 8-bit saturates at zero
    alpha.convertTo(alpha, CV_32F, 1.0 / 255.0);
    return alpha;
}

[[nodiscard]] cv::Mat mishima_alpha(cv::Mat const& image, cv::Vec3b key) {
    basal::exception::throw_unless(!image.empty() && image.type() == CV_8UC3, __FILE__, __LINE__,
                                   "mishima_alpha: image must be a non-empty 8UC3 image");
    // Two hidden states: 0 = key (replace), 1 = foreground (keep)
    float const sigma = 48.0f;  // Per-channel observation Gaussian sigma
    float const inv_2s2 = 1.0f / (2.0f * sigma * sigma);
    float const stay = 0.98f;  // P(state_{t+1} = state_t)
    float const change = 1.0f - stay;

    cv::Mat alpha(image.size(), CV_32F, cv::Scalar::all(0.0));
    size_t const cols = static_cast<size_t>(image.cols);
    std::vector<float> emit_key(cols), emit_fg(cols);
    std::vector<float> fwd_key(cols), fwd_fg(cols);
    std::vector<float> bwd_key(cols), bwd_fg(cols);

    for (int y = 0; y < image.rows; ++y) {
        auto const* row = image.ptr<uchar>(y);
        float* out = alpha.ptr<float>(y);
        for (int x = 0; x < image.cols; ++x) {
            size_t x_ = static_cast<size_t>(x);
            float const d_b = static_cast<float>(row[3 * x_ + 0]) - key[0];
            float const d_g = static_cast<float>(row[3 * x_ + 1U]) - key[1U];
            float const d_r = static_cast<float>(row[3 * x_ + 2]) - key[2];
            float const e = std::exp(-(d_b * d_b + d_g * d_g + d_r * d_r) * inv_2s2);
            emit_key[x_] = e;
            emit_fg[x_] = 1.0f - e;
        }
        // Forward pass (scaled so that the sum over states is 1 at each step)
        for (int x = 0; x < image.cols; ++x) {
            size_t x_ = static_cast<size_t>(x);
            float f0, f1;
            if (x == 0) {
                f0 = 0.5f * emit_key[0];
                f1 = 0.5f * emit_fg[0];
            } else {
                f0 = (fwd_key[x_ - 1U] * stay + fwd_fg[x_ - 1U] * change) * emit_key[x_];
                f1 = (fwd_key[x_ - 1U] * change + fwd_fg[x_ - 1U] * stay) * emit_fg[x_];
            }
            float const sum = f0 + f1;
            if (sum <= 0.0f) {
                f0 = 0.5f;
                f1 = 0.5f;
            } else {
                f0 /= sum;
                f1 /= sum;
            }
            fwd_key[x_] = f0;
            fwd_fg[x_] = f1;
        }
        // Backward pass (scaled so that the sum over states is 1 at each step)
        bwd_key[cols - 1U] = 1.0f;
        bwd_fg[cols - 1U] = 1.0f;
        for (int x = image.cols - 2; x >= 0; --x) {
            size_t x_ = static_cast<size_t>(x);
            float n0 = stay * emit_key[x_ + 1U] * bwd_key[x_ + 1U] + change * emit_fg[x_ + 1U] * bwd_fg[x_ + 1U];
            float n1 = change * emit_key[x_ + 1U] * bwd_key[x_ + 1U] + stay * emit_fg[x_ + 1U] * bwd_fg[x_ + 1U];
            float const sum = n0 + n1;
            if (sum <= 0.0f) {
                n0 = 0.5f;
                n1 = 0.5f;
            } else {
                n0 /= sum;
                n1 /= sum;
            }
            bwd_key[x_] = n0;
            bwd_fg[x_] = n1;
        }
        // Posterior P(key | observations) -> alpha matte
        for (int x = 0; x < image.cols; ++x) {
            size_t x_ = static_cast<size_t>(x);
            float const g0 = fwd_key[x_] * bwd_key[x_];
            float const g1 = fwd_fg[x_] * bwd_fg[x_];
            float const sum = g0 + g1;
            out[x_] = (sum <= 0.0f) ? 0.5f : g0 / sum;
        }
    }
    return alpha;
}

cv::Mat compute_alpha(ChromaType type, cv::Mat const& image, cv::Vec3b key, float fg_keep, float bg_keep,
                      cv::Mat const& protect) {
    switch (type) {
        case ChromaType::Vlahos:
            return vlahos_alpha(image, key);
        case ChromaType::Mishima:
            return mishima_alpha(image, key);
        case ChromaType::ClosedFormMatting:
        case ChromaType::BayesianMatting:
        case ChromaType::KnnMatting:
        case ChromaType::GlobalMatting:
        case ChromaType::FusedMatting: {
            // Matting solves for the *foreground* alpha; the chroma key convention
            // (1.0 = replace with background) is the complement.
            cv::Mat foreground;
            if (type == ChromaType::FusedMatting) {
                // Fuse keying with matting: the per-pixel keying signal (green excess)
                // pins definite subject pixels so the solver cannot bleed key alpha
                // into them, while the genuine spill/shadow band is solved softly by
                // the matting Laplacian. No HSV trimap is involved.
                foreground = matting::fused_matting(image, key, fg_keep, bg_keep, protect);
            } else {
                cv::Mat const trimap = matting::build_trimap(image, key);
                switch (type) {
                    case ChromaType::ClosedFormMatting:
                        foreground = matting::closed_form_matting(image, trimap);
                        break;
                    case ChromaType::BayesianMatting:
                        foreground = matting::bayesian_matting(image, trimap);
                        break;
                    case ChromaType::KnnMatting:
                        foreground = matting::knn_matting(image, trimap);
                        break;
                    case ChromaType::GlobalMatting:
                        foreground = matting::global_matting(image, trimap);
                        break;
                    default:
                        break;
                }
            }
            cv::Mat key_alpha;
            cv::subtract(cv::Scalar::all(1.0), foreground, key_alpha);
            return key_alpha;
        }
    }
    basal::exception::throw_unless(false, __FILE__, __LINE__, "Unknown chroma type");
    return {};  // unreachable
}

void remap_alpha(cv::Mat& alpha, float clip_black, float clip_white) {
    basal::exception::throw_unless(!alpha.empty() && alpha.type() == CV_32FC1, __FILE__, __LINE__,
                                   "remap_alpha: alpha must be a non-empty 32FC1 matte");
    float const lo = std::clamp(clip_black, 0.0f, 1.0f);
    float const hi = std::clamp(clip_white, 0.0f, 1.0f);
    if (hi <= lo) {
        alpha.setTo(0.0);
        return;
    }
    cv::subtract(alpha, cv::Scalar(static_cast<double>(lo)), alpha);
    cv::multiply(alpha, cv::Scalar(1.0 / (static_cast<double>(hi) - static_cast<double>(lo))), alpha);
    cv::max(alpha, cv::Scalar::all(0.0), alpha);
    cv::min(alpha, cv::Scalar::all(1.0), alpha);
}

void chroma_replace(cv::Mat const& image, cv::Mat const& background, std::string const& type, cv::Vec3b key,
                    cv::Mat& result, float clip_black, float clip_white, float fg_keep, float bg_keep,
                    cv::Mat const& protect) {
    basal::exception::throw_unless(!image.empty() && image.type() == CV_8UC3, __FILE__, __LINE__,
                                   "chroma_replace: input image must be a non-empty 8UC3 image");
    basal::exception::throw_unless(!background.empty() && background.type() == CV_8UC3, __FILE__, __LINE__,
                                   "chroma_replace: background image must be a non-empty 8UC3 image");
    cv::Mat bg = background;
    if (bg.size() != image.size())
        cv::resize(bg, bg, image.size());
    cv::Mat alpha = compute_alpha(parse_chroma_type(type), image, key, fg_keep, bg_keep, protect);
    remap_alpha(alpha, clip_black, clip_white);

    cv::Mat bgf, imgf;
    bg.convertTo(bgf, CV_32F);
    image.convertTo(imgf, CV_32F);
    cv::Mat alpha3;
    cv::merge(std::vector<cv::Mat>{alpha, alpha, alpha}, alpha3);
    cv::Mat term_bg, term_fg, one_minus;
    cv::multiply(bgf, alpha3, term_bg);
    cv::subtract(cv::Scalar::all(1.0), alpha3, one_minus);
    cv::multiply(imgf, one_minus, term_fg);
    cv::Mat sum;
    cv::add(term_bg, term_fg, sum);
    sum.convertTo(result, CV_8U);
}

void key_out(cv::Mat const& image, std::string const& type, cv::Vec3b key, cv::Mat& result, float clip_black,
             float clip_white, float fg_keep, float bg_keep, cv::Mat const& protect) {
    // A solid fill of the pure key color is brighter and more consistent than a
    // physical key screen, which is usually darker and tinted with the other channels.
    cv::Mat solid(image.size(), CV_8UC3,
                  cv::Scalar(static_cast<double>(key[0U]), static_cast<double>(key[1U]), static_cast<double>(key[2U])));
    chroma_replace(image, solid, type, key, result, clip_black, clip_white, fg_keep, bg_keep, protect);
}

std::string to_lower(std::string const& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

ChromaType parse_chroma_type(std::string const& type) {
    // Normalize separators so that "closed-form", "closed_form" and "closedform" all work.
    std::string t = to_lower(type);
    std::replace(t.begin(), t.end(), '-', '\0');
    t.erase(std::remove(t.begin(), t.end(), '\0'), t.end());
    std::replace(t.begin(), t.end(), '_', '\0');
    t.erase(std::remove(t.begin(), t.end(), '\0'), t.end());
    if (t == "vlahos")
        return ChromaType::Vlahos;
    if (t == "mishima")
        return ChromaType::Mishima;
    if (t == "closedform" || t == "levin")
        return ChromaType::ClosedFormMatting;
    if (t == "bayesian" || t == "chuang")
        return ChromaType::BayesianMatting;
    if (t == "knn" || t == "chen")
        return ChromaType::KnnMatting;
    if (t == "global" || t == "opencv" || t == "infoflow")
        return ChromaType::GlobalMatting;
    if (t == "fused" || t == "hybrid")
        return ChromaType::FusedMatting;
    basal::exception::throw_unless(false, __FILE__, __LINE__,
                                   "Unknown --type: %s (expected vlahos, mishima, closedform, bayesian, knn, global "
                                   "or fused)",
                                   type.c_str());
    return ChromaType::Vlahos;
}

cv::Vec3b named_color(std::string const& name) {
    std::string const n = to_lower(name);
    struct {
        char const* name;
        cv::Vec3b bgr;
    } const colors[] = {
        {"green", cv::Vec3b{0, 255, 0}},     {"blue", cv::Vec3b{255, 0, 0}},      {"red", cv::Vec3b{0, 0, 255}},
        {"cyan", cv::Vec3b{255, 255, 0}},    {"magenta", cv::Vec3b{255, 0, 255}}, {"yellow", cv::Vec3b{0, 255, 255}},
        {"white", cv::Vec3b{255, 255, 255}}, {"black", cv::Vec3b{0, 0, 0}},
    };
    for (auto const& c : colors)
        if (n == c.name)
            return c.bgr;
    basal::exception::throw_unless(false, __FILE__, __LINE__, "Unknown --color: %s", name.c_str());
    return cv::Vec3b{0, 0, 0};
}

}  // namespace vision
