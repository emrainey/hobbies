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

cv::Mat keylight_alpha(cv::Mat const& image, cv::Vec3b key, float softness) {
    detail::validate_image(image, "image");
    int const dominant = detail::dominant_channel(key);
    int const other0 = (dominant + 1) % 3;
    int const other1 = (dominant + 2) % 3;
    float const s = std::clamp(softness, 0.01f, 1.0f);

    // Keylight-style screen balance. Real screens are low in the non-key channels and
    // slightly tinted there; gradient/vignetted illumination makes the same screen
    // pixel a different colour at every location. We rescale the two non-key channels
    // toward the key channel so a pixel whose ratio of other-channels to key channel
    // matches the key reads neutral, which makes an uneven screen key uniformly. The
    // ratios are protected against a zero key component (pure named keys like green
    // have zero blue/red, so a plain divide would blow up).
    float const kd = static_cast<float>(key[dominant]);
    float const bg0 = (key[other0] <= 0) ? 1.0f : kd / static_cast<float>(key[other0]);
    float const bg1 = (key[other1] <= 0) ? 1.0f : kd / static_cast<float>(key[other1]);

    cv::Mat alpha(image.size(), CV_32F, cv::Scalar::all(0.0));
    for (int y = 0; y < image.rows; ++y) {
        auto const* px = image.ptr<cv::Vec3b>(y);
        float* out = alpha.ptr<float>(y);
        for (int x = 0; x < image.cols; ++x) {
            float const v[3]
                = {static_cast<float>(px[x][0]), static_cast<float>(px[x][1U]), static_cast<float>(px[x][2])};
            float const kch = v[dominant];
            // Screen-balanced non-key channels: a pure-key pixel maps these up to kch.
            float const ob0 = std::min(v[other0] * bg0, kch);
            float const ob1 = std::min(v[other1] * bg1, kch);
            // Excess of the key channel over the surrounding colour in balanced space;
            // positive only when the key channel really dominates.
            float const excess = kch - 0.5f * (ob0 + ob1);
            // Soft matte: ramp from 0 (no excess) to 1 as the excess crosses a window
            // scaled to the key level, so faint spill is only partially keyed. Dim
            // pixels never reach full alpha because their key channel is low and the
            // window is key-level relative.
            float const window = s * std::max(kd, 1.0f);
            float const key_amt
                = (window <= 0.0f) ? (excess > 0.0f ? 1.0f : 0.0f) : std::clamp(excess / window, 0.0f, 1.0f);
            out[x] = key_amt;
        }
    }
    return alpha;
}

cv::Mat compute_alpha(ChromaType type, cv::Mat const& image, cv::Vec3b key, float fg_keep, float bg_keep,
                      cv::Mat const& protect, float softness, int trimap_clean) {
    switch (type) {
        case ChromaType::Vlahos:
            return vlahos_alpha(image, key);
        case ChromaType::Keylight:
            // A negative softness selects the algorithm default; otherwise pass the
            // user's value straight through so the matte transition can be dialed.
            return keylight_alpha(image, key, softness < 0.0f ? 0.1f : softness);
        case ChromaType::Mishima:
            return mishima_alpha(image, key);
        case ChromaType::ClosedFormMatting:
        case ChromaType::BayesianMatting:
        case ChromaType::KnnMatting:
        case ChromaType::GlobalMatting:
        case ChromaType::SharedSampling:
        case ChromaType::FusedMatting: {
            // Matting solves for the *foreground* alpha; the chroma key convention
            // (1.0 = replace with background) is the complement.
            cv::Mat foreground;
            if (type == ChromaType::FusedMatting) {
                // Fuse keying with matting: the per-pixel keying signal (green excess)
                // pins definite subject pixels so the solver cannot bleed key alpha
                // into them, while the genuine spill/shadow band is solved softly by
                // the matting Laplacian. No HSV trimap is involved.
                foreground = matting::fused_matting(image, key, fg_keep, bg_keep, protect, trimap_clean);
            } else {
                cv::Mat trimap = matting::build_trimap(image, key, 0.12f, 0.25f, 0.30f, protect);
                if (trimap_clean > 0) {
                    trimap = matting::clean_trimap(trimap, trimap_clean);
                }
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
                    case ChromaType::SharedSampling:
                        foreground = matting::shared_sampling_matting(image, trimap);
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

cv::Mat despill(cv::Mat const& image, cv::Mat const& alpha, cv::Vec3b key, float strength, float floor) {
    detail::validate_image(image, "image");
    if (alpha.empty() || alpha.type() != CV_32FC1 || alpha.size() != image.size()) {
        throw std::invalid_argument("despill: alpha must be a CV_32FC1 matte matching the image size");
    }
    int const dominant = detail::dominant_channel(key);
    int const other[2] = {(dominant + 1) % 3, (dominant + 2) % 3};
    float const s = std::max(0.0f, strength);
    // Despill floor: the minimum subject fraction (1 - alpha) at which spill is
    // removed. Pixels the solving matte leans strongly toward background (alpha near
    // 1) are left strictly untouched. Without this, a soft matte with alpha slightly
    // under 1 across the screen ring would have a little green removed and wobble the
    // composite background.
    float const f = std::clamp(floor, 0.0f, 1.0f);
    cv::Mat out = image.clone();
    for (int y = 0; y < image.rows; ++y) {
        auto const* px = image.ptr<cv::Vec3b>(y);
        auto* dst = out.ptr<cv::Vec3b>(y);
        auto const* a = alpha.ptr<float>(y);
        for (int x = 0; x < image.cols; ++x) {
            float const k = static_cast<float>(px[x][dominant]);
            float const o0 = static_cast<float>(px[x][other[0]]);
            float const o1 = static_cast<float>(px[x][other[1]]);
            float const excess = k - std::max(o0, o1);
            if (excess <= 0.0f) {
                continue;
            }
            // Ramp the subject fraction from 0 at the floor to 1 for a fully-kept
            // pixel, so despill fades in smoothly instead of stepping.
            float const subject = std::clamp(1.0f - a[x], 0.0f, 1.0f);
            float const effective = (f >= 1.0f) ? 0.0f : (subject - f) / (1.0f - f);
            if (effective <= 0.0f) {
                continue;
            }
            dst[x][dominant] = static_cast<uchar>(std::max(0.0f, k - excess * effective * s));
        }
    }
    return out;
}

cv::Vec3b estimate_screen_color(cv::Mat const& image, cv::Vec3b key) {
    detail::validate_image(image, "image");
    int const dominant = detail::dominant_channel(key);
    int const other[2] = {(dominant + 1) % 3, (dominant + 2) % 3};
    // Quantize colors of confident screen pixels and histogram them; the dominant bin's
    // mean is the estimated screen color (robust to outlier spill/edges).
    constexpr int bins = 32;
    size_t const bin_stride = bins * bins;
    std::vector<size_t> hist(bins * bins * bins, 0);
    size_t count = 0;
    for (int y = 0; y < image.rows; ++y) {
        auto const* px = image.ptr<cv::Vec3b>(y);
        for (int x = 0; x < image.cols; ++x) {
            float const k = static_cast<float>(px[x][dominant]);
            float const o0 = static_cast<float>(px[x][other[0]]);
            float const o1 = static_cast<float>(px[x][other[1]]);
            float const excess = k - std::max(o0, o1);
            if (excess < 40.0f || k < 64.0f) {
                continue;  // not confident screen
            }
            int bi = static_cast<int>(k * bins / 256.0f);
            int b0 = static_cast<int>(o0 * bins / 256.0f);
            int b1 = static_cast<int>(o1 * bins / 256.0f);
            bi = std::min(static_cast<int>(bins) - 1, std::max(0, bi));
            b0 = std::min(static_cast<int>(bins) - 1, std::max(0, b0));
            b1 = std::min(static_cast<int>(bins) - 1, std::max(0, b1));
            ++hist[static_cast<size_t>(bi) * bin_stride + static_cast<size_t>(b0) * bins + static_cast<size_t>(b1)];
            ++count;
        }
    }
    if (count == 0) {
        return key;
    }
    size_t const peak = static_cast<size_t>(std::distance(hist.begin(), std::max_element(hist.begin(), hist.end())));
    int const peak_b0 = static_cast<int>((peak % bin_stride) / bins);
    int const peak_b1 = static_cast<int>(peak % static_cast<size_t>(bins));
    size_t const peak_bi = peak / bin_stride;
    // Mean of the actual pixels landing in the peak bin.
    double sum[3] = {0.0, 0.0, 0.0};
    size_t n = 0;
    for (int y = 0; y < image.rows; ++y) {
        auto const* px = image.ptr<cv::Vec3b>(y);
        for (int x = 0; x < image.cols; ++x) {
            float const k = static_cast<float>(px[x][dominant]);
            float const o0 = static_cast<float>(px[x][other[0]]);
            float const o1 = static_cast<float>(px[x][other[1]]);
            float const excess = k - std::max(o0, o1);
            if (excess < 40.0f || k < 64.0f) {
                continue;
            }
            int const bi = static_cast<int>(k * bins / 256.0f);
            int const b0 = static_cast<int>(o0 * bins / 256.0f);
            int const b1 = static_cast<int>(o1 * bins / 256.0f);
            if (std::min(bins - 1, std::max(0, b0)) == peak_b0 && std::min(bins - 1, std::max(0, b1)) == peak_b1
                && std::min(bins - 1, std::max(0, bi)) == static_cast<int>(peak_bi)) {
                sum[0] += static_cast<double>(px[x][0]);
                sum[1] += static_cast<double>(px[x][1]);
                sum[2] += static_cast<double>(px[x][2]);
                ++n;
            }
        }
    }
    if (n == 0) {
        return key;
    }
    return cv::Vec3b{static_cast<uchar>(std::lround(sum[0] / static_cast<double>(n))),
                     static_cast<uchar>(std::lround(sum[1] / static_cast<double>(n))),
                     static_cast<uchar>(std::lround(sum[2] / static_cast<double>(n)))};
}

void refine_alpha(cv::Mat& alpha, cv::Mat const& image, int radius, float eps) {
    if (alpha.empty() || alpha.type() != CV_32FC1 || image.empty() || image.type() != CV_8UC3
        || image.size() != alpha.size()) {
        throw std::invalid_argument("refine_alpha: alpha must be a CV_32FC1 matte matching the CV_8UC3 image size");
    }
    int const r = std::max(1, radius);
    // Guided filter (He, Sun & Tang 2010) on the alpha, guided by the image luminance.
    cv::Mat I, p;
    cv::cvtColor(image, I, cv::COLOR_BGR2GRAY);
    I.convertTo(I, CV_32F, 1.0 / 255.0);
    alpha.convertTo(p, CV_32F);
    cv::Mat mean_I, mean_p, corr_I, corr_Ip;
    cv::boxFilter(I, mean_I, CV_32F, cv::Size(r, r));
    cv::boxFilter(p, mean_p, CV_32F, cv::Size(r, r));
    cv::boxFilter(I.mul(I), corr_I, CV_32F, cv::Size(r, r));
    cv::boxFilter(I.mul(p), corr_Ip, CV_32F, cv::Size(r, r));
    cv::Mat const var_I = corr_I - mean_I.mul(mean_I);
    cv::Mat const cov_Ip = corr_Ip - mean_I.mul(mean_p);
    cv::Mat const a = cov_Ip / (var_I + static_cast<double>(eps));
    cv::Mat const b = mean_p - a.mul(mean_I);
    cv::Mat mean_a, mean_b, q;
    cv::boxFilter(a, mean_a, CV_32F, cv::Size(r, r));
    cv::boxFilter(b, mean_b, CV_32F, cv::Size(r, r));
    q = mean_a.mul(I) + mean_b;
    cv::max(q, cv::Scalar::all(0.0), q);
    cv::min(q, cv::Scalar::all(1.0), q);
    q.copyTo(alpha);
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

// Temporal EMA matte smoothing for video. Damps per-pixel alpha flicker (a noisy
// screen's matte wiggling frame to frame) without trailing a moving subject's hard
// edge: the smoothing factor is scaled toward zero at near-solid pixels (alpha ~ 0 or
// ~1) so definite screen/subject stays crisp, and reaches full strength only in the
// soft fringe where flicker is visible.
//
// @param[in,out] alpha Current-frame key-convention matte (CV_32FC1, in [0,1]); on
//                      return holds this frame's smoothed value
// @param[in,out] state Persistent accumulator, may be empty on the first frame
// @param smooth         EMA weight toward the previous frame in [0,1]; 0 disables
void smooth_alpha(cv::Mat& alpha, cv::Mat& state, float smooth) {
    if (smooth <= 0.0f) {
        return;
    }
    if (state.empty() || state.size() != alpha.size()) {
        state = alpha.clone();
        return;
    }
    float const f = std::clamp(smooth, 0.0f, 1.0f);
    for (int y = 0; y < alpha.rows; ++y) {
        auto* a = alpha.ptr<float>(y);
        auto const* p = state.ptr<float>(y);
        for (int x = 0; x < alpha.cols; ++x) {
            float const cur = std::clamp(a[x], 0.0f, 1.0f);
            float const prev = std::clamp(p[x], 0.0f, 1.0f);
            // Edge-lag guard: near-solid pixels (dist to 0 or 1 close to 0) get barely
            // any temporal weight, so a hard subject edge does not smear or trail.
            float const dist_solid = std::min(cur, 1.0f - cur);              // 0 at solid, 0.5 mid
            float const smoothness = f * std::min(1.0f, 2.0f * dist_solid);  // 0 at solid, f mid
            a[x] = prev * smoothness + cur * (1.0f - smoothness);
        }
    }
    state = alpha.clone();
}

void chroma_replace(cv::Mat const& image, cv::Mat const& background, std::string const& type, cv::Vec3b key,
                    cv::Mat& result, float clip_black, float clip_white, float fg_keep, float bg_keep,
                    cv::Mat const& protect, float despill_strength, int refine_radius, float softness,
                    float despill_floor, int trimap_clean, float matte_smooth, cv::Mat* matte_state) {
    basal::exception::throw_unless(!image.empty() && image.type() == CV_8UC3, __FILE__, __LINE__,
                                   "chroma_replace: input image must be a non-empty 8UC3 image");
    basal::exception::throw_unless(!background.empty() && background.type() == CV_8UC3, __FILE__, __LINE__,
                                   "chroma_replace: background image must be a non-empty 8UC3 image");
    cv::Mat bg = background;
    if (bg.size() != image.size())
        cv::resize(bg, bg, image.size());
    cv::Mat alpha
        = compute_alpha(parse_chroma_type(type), image, key, fg_keep, bg_keep, protect, softness, trimap_clean);
    remap_alpha(alpha, clip_black, clip_white);
    if (refine_radius > 0) {
        refine_alpha(alpha, image, refine_radius);
    }
    // Temporal matte smoothing: collapse per-frame alpha flicker in the fringe band
    // (hard edges stay crisp via the smoothing helper's edge-lag guard).
    if (matte_smooth > 0.0f && matte_state != nullptr) {
        smooth_alpha(alpha, *matte_state, matte_smooth);
    }

    // Despill the kept foreground before compositing so spill suppression is baked into
    // the output even when keying out (no background). Despill keys on the keyed matte
    // and leaves background-leaning pixels untouched via the despill floor.
    cv::Mat src = image;
    if (despill_strength > 0.0f) {
        src = despill(image, alpha, key, despill_strength, despill_floor);
    }

    cv::Mat bgf, imgf;
    bg.convertTo(bgf, CV_32F);
    src.convertTo(imgf, CV_32F);
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
             float clip_white, float fg_keep, float bg_keep, cv::Mat const& protect, float despill_strength,
             int refine_radius, float softness, float despill_floor, int trimap_clean, float matte_smooth,
             cv::Mat* matte_state) {
    // A solid fill of the pure key color is brighter and more consistent than a
    // physical key screen, which is usually darker and tinted with the other channels.
    cv::Mat solid(image.size(), CV_8UC3,
                  cv::Scalar(static_cast<double>(key[0U]), static_cast<double>(key[1U]), static_cast<double>(key[2U])));
    chroma_replace(image, solid, type, key, result, clip_black, clip_white, fg_keep, bg_keep, protect, despill_strength,
                   refine_radius, softness, despill_floor, trimap_clean, matte_smooth, matte_state);
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
    if (t == "keylight")
        return ChromaType::Keylight;
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
    if (t == "shared" || t == "sampling" || t == "sharedsampling")
        return ChromaType::SharedSampling;
    if (t == "fused" || t == "hybrid")
        return ChromaType::FusedMatting;
    basal::exception::throw_unless(false, __FILE__, __LINE__,
                                   "Unknown --type: %s (expected vlahos, keylight, mishima, closedform, bayesian, "
                                   "knn, global, shared or fused)",
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
