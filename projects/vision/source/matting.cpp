#include "vision/matting.hpp"

#include <opencv2/opencv.hpp>
#if CV_VERSION_MAJOR >= 5 || (CV_VERSION_MAJOR == 4 && CV_VERSION_MINOR >= 7)
#include <opencv2/alphamat.hpp>
#endif

#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace vision {
namespace matting {
namespace detail {

inline void validate(cv::Mat const& image, std::string const& name) {
    if (!image.empty() && image.type() == CV_8UC3) {
        return;
    }
    throw std::invalid_argument(name + " must be a non-empty 8-bit 3-channel (CV_8UC3) image");
}

inline void validate_trimap(cv::Mat const& trimap, cv::Mat const& image) {
    if (trimap.empty() || trimap.type() != CV_8UC1 || trimap.size() != image.size()) {
        throw std::invalid_argument(
            "trimap must be a non-empty 8-bit single-channel (CV_8UC1) image matching the "
            "input size, with classes 0 (background), 128 (unknown) and 255 (foreground)");
    }
}

/// Index of the brightest channel of a BGR key color (0 = blue, 1 = green, 2 = red).
inline int dominant_channel(cv::Vec3b const& key) {
    int best = 0;
    for (int c = 1; c < 3; ++c) {
        if (key[c] > key[best]) {
            best = c;
        }
    }
    return best;
}

/// Solves (L + lambda*M) x = lambda*(M o target) by preconditioned conjugate gradient.
///
/// @param n number of unknowns
/// @param apply_l multiplies by L only (no lambda*M term)
/// @param diag diagonal of L, used as the Jacobi preconditioner
/// @param mask 1.0 on constrained (definite) pixels, 0.0 elsewhere
/// @param lambda hard-constraint weight
/// @param target desired alpha (0 or 1) on constrained pixels
/// @param x in/out: initial guess on entry (e.g. the hard trimap alpha), solution on exit
void matting_cg(size_t n, std::function<void(float const*, float*)> const& apply_l, std::vector<float> const& diag,
                std::vector<float> const& mask, float lambda, std::vector<float> const& target, std::vector<float>& x) {
    std::vector<float> rhs(n), q(n), r(n), z(n), p(n);
    std::vector<float> precond(n);
    for (size_t i = 0; i < n; ++i) {
        rhs[i] = lambda * mask[i] * target[i];
        precond[i] = diag[i] + lambda * mask[i];
        if (!(precond[i] > 0.0f)) {
            precond[i] = 1.0f;
        }
    }
    apply_l(x.data(), q.data());
    float r_dot_z = 0.0f;
    float r_sqr = 0.0f;
    for (size_t i = 0; i < n; ++i) {
        r[i] = rhs[i] - (q[i] + lambda * mask[i] * x[i]);
        z[i] = r[i] / precond[i];
        p[i] = z[i];
        r_dot_z += r[i] * z[i];
        r_sqr += r[i] * r[i];
    }
    float const r_init = std::sqrt(r_sqr);
    if (!(r_init > 1.0e-6f)) {
        return;
    }
    size_t constexpr max_iterations = 256;
    float constexpr tolerance = 1.0e-4f;
    for (size_t iteration = 0; iteration < max_iterations; ++iteration) {
        apply_l(p.data(), q.data());
        float p_dot_q = 0.0f;
        for (size_t i = 0; i < n; ++i) {
            q[i] += lambda * mask[i] * p[i];
            p_dot_q += p[i] * q[i];
        }
        if (!(p_dot_q > 0.0f)) {
            break;  // near-singular; keep the iteration's best estimate
        }
        float const alpha = r_dot_z / p_dot_q;
        float r_sqr_new = 0.0f;
        for (size_t i = 0; i < n; ++i) {
            x[i] += alpha * p[i];
            r[i] -= alpha * q[i];
            r_sqr_new += r[i] * r[i];
        }
        float const r_new = std::sqrt(r_sqr_new);
        if (r_new <= tolerance * r_init) {
            break;
        }
        float r_dot_z_new = 0.0f;
        for (size_t i = 0; i < n; ++i) {
            z[i] = r[i] / precond[i];
            r_dot_z_new += r[i] * z[i];
        }
        float const beta = r_dot_z_new / r_dot_z;
        for (size_t i = 0; i < n; ++i) {
            p[i] = z[i] + beta * p[i];
        }
        r_dot_z = r_dot_z_new;
    }
}

/// 3x3 inverse of a symmetric matrix with diagonal regularization (eps*I).
void symmetric3_regularized_inverse(double const A[][3], double eps, double out[][3]) {
    double const a = A[0][0] + eps;
    double const b = A[0][1];
    double const c = A[0][2];
    double const d = A[1][0];
    double const e = A[1][1] + eps;
    double const f = A[1][2];
    double const g = A[2][0];
    double const h = A[2][1];
    double const i_ = A[2][2] + eps;
    double const det = a * (e * i_ - f * h) - b * (d * i_ - f * g) + c * (d * h - e * g);
    double const inv_det = 1.0 / (det > 0.0 ? det : (det < 0.0 ? -det : 1.0));
    out[0][0] = (e * i_ - f * h) * inv_det;
    out[0][1] = (c * h - b * i_) * inv_det;
    out[0][2] = (b * f - c * e) * inv_det;
    out[1][0] = (f * g - d * i_) * inv_det;
    out[1][1] = (a * i_ - c * g) * inv_det;
    out[1][2] = (c * d - a * f) * inv_det;
    out[2][0] = (d * h - e * g) * inv_det;
    out[2][1] = (b * g - a * h) * inv_det;
    out[2][2] = (a * e - b * d) * inv_det;
}

}  // namespace detail

cv::Mat build_trimap(cv::Mat const& image, cv::Vec3b key, float bg_hue_tol, float sat_min, float fg_hue_tol) {
    detail::validate(image, "image");
    cv::Mat hsv;
    cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);

    cv::Mat key_bgr(1, 1, CV_8UC3, cv::Scalar(key[0], key[1U], key[2]));
    cv::Mat key_hsv;
    cv::cvtColor(key_bgr, key_hsv, cv::COLOR_BGR2HSV);
    float const key_hue = static_cast<float>(key_hsv.at<cv::Vec3b>(0, 0)[0]);

    float const bg_tol = std::clamp(bg_hue_tol, 0.0f, 1.0f);
    float const fg_tol = std::clamp(fg_hue_tol, 0.0f, 1.0f);
    float const s_min = std::clamp(sat_min, 0.0f, 1.0f);

    cv::Mat trimap(image.size(), CV_8UC1, cv::Scalar(static_cast<double>(TrimapClass::Unknown), 0.0, 0.0, 0.0));
    for (int y = 0; y < image.rows; ++y) {
        auto const* px = hsv.ptr<cv::Vec3b>(y);
        uchar* out = trimap.ptr<uchar>(y);
        for (int x = 0; x < image.cols; ++x) {
            float const sat = static_cast<float>(px[x][1U]) / 255.0f;
            float const val = static_cast<float>(px[x][2]) / 255.0f;
            float hue_dist = std::fabs(static_cast<float>(px[x][0]) - key_hue);
            if (hue_dist > 90.0f) {
                hue_dist = 180.0f - hue_dist;
            }
            hue_dist /= 180.0f;
            bool const near_key = hue_dist <= bg_tol;
            bool const far_from_key = hue_dist > fg_tol;
            if (near_key && sat >= s_min && val >= 0.05f) {
                out[x] = static_cast<uchar>(TrimapClass::Background);
            } else if (far_from_key || (sat < s_min && hue_dist > 2.0f * bg_tol)) {
                out[x] = static_cast<uchar>(TrimapClass::Foreground);
            } else {
                out[x] = static_cast<uchar>(TrimapClass::Unknown);
            }
        }
    }
    return trimap;
}

cv::Mat build_trimap_from_keying(cv::Mat const& image, cv::Vec3b key, float fg_keep, float bg_keep,
                                 cv::Mat const& protect) {
    detail::validate(image, "image");
    if (!protect.empty()) {
        if (protect.type() != CV_8UC1 || protect.size() != image.size()) {
            throw std::invalid_argument("protect mask must be CV_8UC1 and match the input size");
        }
    }
    // The green excess is the same per-pixel signal as vlahos_alpha: how much the key
    // channel dominates the other two. It is cheap observation, unlike the matting
    // solver, so it can safely pin which pixels are definitely not the screen.
    float const lo = std::clamp(fg_keep, 0.0f, 1.0f);
    float const hi = std::clamp(bg_keep, lo, 1.0f);

    cv::Mat const* channels[3];
    cv::Mat split_channels[3];
    cv::split(image, split_channels);
    for (int c = 0; c < 3; ++c) {
        channels[c] = &split_channels[c];
    }
    int const dominant = detail::dominant_channel(key);
    int const other[2] = {(dominant + 1) % 3, (dominant + 2) % 3};
    cv::Mat other_max = cv::max(*channels[other[0]], *channels[other[1]]);
    cv::Mat excess;
    cv::subtract(*channels[dominant], other_max, excess);  // 8-bit saturates at zero

    cv::Mat trimap(image.size(), CV_8UC1, cv::Scalar(static_cast<double>(TrimapClass::Unknown), 0.0, 0.0, 0.0));
    double const scale = 1.0 / 255.0;
    for (int y = 0; y < image.rows; ++y) {
        uchar const* ex = excess.ptr<uchar>(y);
        uchar const* pr = protect.empty() ? nullptr : protect.ptr<uchar>(y);
        uchar* out = trimap.ptr<uchar>(y);
        for (int x = 0; x < image.cols; ++x) {
            if (pr != nullptr) {
                // A feathered protect mask fades smoothly at its boundary: solid (bright)
                // pixels are pinned subject, grey fringe pixels are left unknown so the
                // solver blends the matte instead of stepping on a hard edge.
                int const p = pr[x];
                if (p >= 128) {
                    out[x] = static_cast<uchar>(TrimapClass::Foreground);
                    continue;
                }
                if (p > 0) {
                    out[x] = static_cast<uchar>(TrimapClass::Unknown);
                    continue;
                }
            }
            float const e = static_cast<float>(ex[x]) * static_cast<float>(scale);
            if (e >= hi) {
                out[x] = static_cast<uchar>(TrimapClass::Background);
            } else if (e <= lo) {
                out[x] = static_cast<uchar>(TrimapClass::Foreground);
            } else {
                out[x] = static_cast<uchar>(TrimapClass::Unknown);
            }
        }
    }
    return trimap;
}

cv::Mat fused_matting(cv::Mat const& image, cv::Vec3b key, float fg_keep, float bg_keep, cv::Mat const& protect) {
    detail::validate(image, "image");
    return closed_form_matting(image, build_trimap_from_keying(image, key, fg_keep, bg_keep, protect));
}

cv::Mat closed_form_matting(cv::Mat const& image, cv::Mat const& trimap, float lambda) {
    detail::validate(image, "image");
    detail::validate_trimap(trimap, image);

    int const rows = image.rows;
    int const cols = image.cols;
    size_t const n = static_cast<size_t>(rows) * static_cast<size_t>(cols);
    int const win_rows = static_cast<int>(std::max(0, rows - 2));
    int const win_cols = static_cast<int>(std::max(0, cols - 2));
    size_t const nwin = static_cast<size_t>(win_rows) * static_cast<size_t>(win_cols);
    if (nwin == 0) {
        // Edge case too small for any 3x3 window: return the hard trimap alpha.
        cv::Mat alpha(rows, cols, CV_32FC1);
        for (int y = 0; y < rows; ++y) {
            auto const* t = trimap.ptr<uchar>(y);
            float* a = alpha.ptr<float>(y);
            for (int x = 0; x < cols; ++x) {
                a[x] = (t[x] == static_cast<uchar>(TrimapClass::Foreground)) ? 1.0f : 0.0f;
            }
        }
        return alpha;
    }

    cv::Mat imgf;
    image.convertTo(imgf, CV_32FC3, 1.0 / 255.0);

    // Per-window statistics: mean color (3 floats) and symmetric regularized inverse
    // covariance (6 floats) of every 3x3 window.
    std::vector<float> mean(nwin * 3, 0.0f);
    std::vector<float> inv_cov(nwin * 6, 0.0f);
    std::vector<float> laplacian_diag(n, 0.0f);
    double constexpr eps = 1.0e-6;

    auto const window_index = [&](int cy, int cx) {
        return static_cast<size_t>(cy - 1) * static_cast<size_t>(win_cols) + static_cast<size_t>(cx - 1);
    };

    for (int cy = 1; cy < rows - 1; ++cy) {
        for (int cx = 1; cx < cols - 1; ++cx) {
            double sum[3] = {0.0, 0.0, 0.0};
            cv::Vec3f px[9];
            int t = 0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    px[t] = imgf.at<cv::Vec3f>(cy + dy, cx + dx);
                    sum[0] += static_cast<double>(px[t][0]);
                    sum[1] += static_cast<double>(px[t][1U]);
                    sum[2] += static_cast<double>(px[t][2]);
                    ++t;
                }
            }
            double const mu[3] = {sum[0] / 9.0, sum[1] / 9.0, sum[2] / 9.0};
            double cov[3][3] = {};
            for (int k = 0; k < 9; ++k) {
                double const c0 = static_cast<double>(px[k][0]) - mu[0];
                double const c1 = static_cast<double>(px[k][1U]) - mu[1];
                double const c2 = static_cast<double>(px[k][2]) - mu[2];
                cov[0][0] += c0 * c0;
                cov[0][1] += c0 * c1;
                cov[0][2] += c0 * c2;
                cov[1][1] += c1 * c1;
                cov[1][2] += c1 * c2;
                cov[2][2] += c2 * c2;
            }
            cov[0][0] /= 9.0;
            cov[0][1] /= 9.0;
            cov[0][2] /= 9.0;
            cov[1][1] /= 9.0;
            cov[1][2] /= 9.0;
            cov[2][2] /= 9.0;
            cov[1][0] = cov[0][1];
            cov[2][0] = cov[0][2];
            cov[2][1] = cov[1][2];

            double inv[3][3] = {};
            detail::symmetric3_regularized_inverse(cov, eps, inv);

            size_t const win = window_index(cy, cx);
            float* m = &mean[win * 3];
            float* g = &inv_cov[win * 6];
            m[0] = static_cast<float>(mu[0]);
            m[1U] = static_cast<float>(mu[1]);
            m[2] = static_cast<float>(mu[2]);
            g[0] = static_cast<float>(inv[0][0]);
            g[1U] = static_cast<float>(inv[0][1]);
            g[2] = static_cast<float>(inv[0][2]);
            g[3] = static_cast<float>(inv[1][1]);
            g[4] = static_cast<float>(inv[1][2]);
            g[5] = static_cast<float>(inv[2][2]);

            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    cv::Vec3f const p = imgf.at<cv::Vec3f>(cy + dy, cx + dx);
                    double const s0 = static_cast<double>(p[0]) - mu[0];
                    double const s1 = static_cast<double>(p[1U]) - mu[1];
                    double const s2 = static_cast<double>(p[2]) - mu[2];
                    double const t0 = inv[0][0] * s0 + inv[0][1] * s1 + inv[0][2] * s2;
                    double const t1 = inv[1][0] * s0 + inv[1][1] * s1 + inv[1][2] * s2;
                    double const t2 = inv[2][0] * s0 + inv[2][1] * s1 + inv[2][2] * s2;
                    double const quad = t0 * s0 + t1 * s1 + t2 * s2;
                    size_t const px_idx
                        = static_cast<size_t>(cy + dy) * static_cast<size_t>(cols) + static_cast<size_t>(cx + dx);
                    laplacian_diag[px_idx] += static_cast<float>((8.0 - quad) / 9.0);
                }
            }
        }
    }

    // Trimap-derived hard constraints and initial guess.
    std::vector<float> mask(n, 0.0f);
    std::vector<float> target(n, 0.0f);
    for (int y = 0; y < rows; ++y) {
        auto const* t = trimap.ptr<uchar>(y);
        for (int x = 0; x < cols; ++x) {
            size_t const i = static_cast<size_t>(y) * static_cast<size_t>(cols) + static_cast<size_t>(x);
            bool const is_fg = t[x] == static_cast<uchar>(TrimapClass::Foreground);
            bool const is_bg = t[x] == static_cast<uchar>(TrimapClass::Background);
            if (is_fg || is_bg) {
                mask[i] = 1.0f;
                target[i] = is_fg ? 1.0f : 0.0f;
            }
        }
    }
    std::vector<float> x = target;

    auto const apply_l = [&](float const* xv, float* out) {
        for (size_t i = 0; i < n; ++i) {
            out[i] = 0.0f;
        }
        for (int cy = 1; cy < rows - 1; ++cy) {
            for (int cx = 1; cx < cols - 1; ++cx) {
                size_t const win = window_index(cy, cx);
                float const* m = &mean[win * 3];
                float const* g = &inv_cov[win * 6];
                double a_sum = 0.0;
                double w0 = 0.0;
                double w1 = 0.0;
                double w2 = 0.0;
                double s[9][3];
                size_t px_idx[9];
                int t = 0;
                for (int dy = -1; dy <= 1; ++dy) {
                    int const row = cy + dy;
                    for (int dx = -1; dx <= 1; ++dx) {
                        int const col = cx + dx;
                        cv::Vec3f const p = imgf.at<cv::Vec3f>(row, col);
                        double const s0 = static_cast<double>(p[0]) - static_cast<double>(m[0]);
                        double const s1 = static_cast<double>(p[1U]) - static_cast<double>(m[1U]);
                        double const s2 = static_cast<double>(p[2]) - static_cast<double>(m[2]);
                        s[t][0] = s0;
                        s[t][1U] = s1;
                        s[t][2] = s2;
                        size_t const idx
                            = static_cast<size_t>(row) * static_cast<size_t>(cols) + static_cast<size_t>(col);
                        px_idx[t] = idx;
                        double const xp = static_cast<double>(xv[idx]);
                        a_sum += xp;
                        w0 += s0 * xp;
                        w1 += s1 * xp;
                        w2 += s2 * xp;
                        ++t;
                    }
                }
                for (int k = 0; k < 9; ++k) {
                    double const ts0 = static_cast<double>(g[0]) * s[k][0] + static_cast<double>(g[1U]) * s[k][1U]
                                       + static_cast<double>(g[2]) * s[k][2];
                    double const ts1 = static_cast<double>(g[1U]) * s[k][0] + static_cast<double>(g[3]) * s[k][1U]
                                       + static_cast<double>(g[4]) * s[k][2];
                    double const ts2 = static_cast<double>(g[2]) * s[k][0] + static_cast<double>(g[4]) * s[k][1U]
                                       + static_cast<double>(g[5]) * s[k][2];
                    double const prod = ts0 * w0 + ts1 * w1 + ts2 * w2;
                    out[px_idx[k]] += static_cast<float>(static_cast<double>(xv[px_idx[k]]) - (a_sum + prod) / 9.0);
                }
            }
        }
    };

    detail::matting_cg(n, apply_l, laplacian_diag, mask, lambda, target, x);

    cv::Mat alpha(rows, cols, CV_32FC1);
    for (size_t i = 0; i < n; ++i) {
        x[i] = std::clamp(x[i], 0.0f, 1.0f);
    }
    std::memcpy(alpha.data, x.data(), n * sizeof(float));
    return alpha;
}

cv::Mat bayesian_matting(cv::Mat const& image, cv::Mat const& trimap) {
    detail::validate(image, "image");
    detail::validate_trimap(trimap, image);

    int const rows = image.rows;
    int const cols = image.cols;

    cv::Mat alpha(rows, cols, CV_32FC1);
    for (int y = 0; y < rows; ++y) {
        auto const* t = trimap.ptr<uchar>(y);
        float* a = alpha.ptr<float>(y);
        for (int x = 0; x < cols; ++x) {
            a[x] = (t[x] == static_cast<uchar>(TrimapClass::Foreground)) ? 1.0f : 0.0f;
        }
    }

    std::vector<cv::Point> bg_pts;
    std::vector<cv::Point> fg_pts;
    std::vector<size_t> unknown;
    for (int y = 0; y < rows; ++y) {
        auto const* t = trimap.ptr<uchar>(y);
        for (int x = 0; x < cols; ++x) {
            if (t[x] == static_cast<uchar>(TrimapClass::Foreground)) {
                fg_pts.emplace_back(x, y);
            } else if (t[x] == static_cast<uchar>(TrimapClass::Background)) {
                bg_pts.emplace_back(x, y);
            } else {
                unknown.push_back(static_cast<size_t>(y) * static_cast<size_t>(cols) + static_cast<size_t>(x));
            }
        }
    }
    if (bg_pts.empty() || fg_pts.empty() || unknown.empty()) {
        return alpha;
    }

    auto make_samples = [&](std::vector<cv::Point> const& pts) {
        cv::Mat feat(static_cast<int>(pts.size()), 2, CV_32F);
        for (size_t i = 0; i < pts.size(); ++i) {
            float* f = feat.ptr<float>(static_cast<int>(i));
            f[0] = static_cast<float>(pts[i].x) / static_cast<float>(cols);
            f[1U] = static_cast<float>(pts[i].y) / static_cast<float>(rows);
        }
        return cv::flann::Index(feat, cv::flann::KDTreeIndexParams(4));
    };
    auto bg_index = make_samples(bg_pts);
    auto fg_index = make_samples(fg_pts);

    int constexpr k_neighbors = 16;
    cv::flann::SearchParams search(64);
    int const bg_k = std::min(k_neighbors, static_cast<int>(bg_pts.size()));
    int const fg_k = std::min(k_neighbors, static_cast<int>(fg_pts.size()));
    double const sigma = 0.08;  // normalized image-coordinate scale

    for (size_t const& unknown_idx : unknown) {
        int const ux = static_cast<int>(unknown_idx % static_cast<size_t>(cols));
        int const uy = static_cast<int>(unknown_idx / static_cast<size_t>(cols));
        cv::Mat query(1, 2, CV_32F);
        query.at<float>(0, 0) = static_cast<float>(ux) / static_cast<float>(cols);
        query.at<float>(0, 1U) = static_cast<float>(uy) / static_cast<float>(rows);

        cv::Mat bg_idx, bg_dist, fg_idx, fg_dist;
        bg_index.knnSearch(query, bg_idx, bg_dist, bg_k, search);
        fg_index.knnSearch(query, fg_idx, fg_dist, fg_k, search);

        // Spatially weighted foreground and background color models.
        double f_sum = 0.0;
        double f_acc[3] = {0.0, 0.0, 0.0};
        for (int k = 0; k < fg_k; ++k) {
            int const sample = fg_idx.at<int>(0, k);
            cv::Point const p = fg_pts[static_cast<size_t>(sample)];
            cv::Vec3b const c = image.at<cv::Vec3b>(p);
            double const dist = static_cast<double>(fg_dist.at<float>(0, k));
            double const weight = std::exp(-(dist * dist) / (2.0 * sigma * sigma));
            f_acc[0] += static_cast<double>(c[0]) * weight;
            f_acc[1U] += static_cast<double>(c[1U]) * weight;
            f_acc[2] += static_cast<double>(c[2]) * weight;
            f_sum += weight;
        }
        double b_sum = 0.0;
        double b_acc[3] = {0.0, 0.0, 0.0};
        for (int k = 0; k < bg_k; ++k) {
            int const sample = bg_idx.at<int>(0, k);
            cv::Point const p = bg_pts[static_cast<size_t>(sample)];
            cv::Vec3b const c = image.at<cv::Vec3b>(p);
            double const dist = static_cast<double>(bg_dist.at<float>(0, k));
            double const weight = std::exp(-(dist * dist) / (2.0 * sigma * sigma));
            b_acc[0] += static_cast<double>(c[0]) * weight;
            b_acc[1U] += static_cast<double>(c[1U]) * weight;
            b_acc[2] += static_cast<double>(c[2]) * weight;
            b_sum += weight;
        }

        // Weighted mean colors of the local foreground and background models.
        double const f_c[3] = {f_sum > 0.0 ? f_acc[0] / f_sum : 0.0, f_sum > 0.0 ? f_acc[1U] / f_sum : 0.0,
                               f_sum > 0.0 ? f_acc[2] / f_sum : 0.0};
        double const b_c[3] = {b_sum > 0.0 ? b_acc[0] / b_sum : 0.0, b_sum > 0.0 ? b_acc[1U] / b_sum : 0.0,
                               b_sum > 0.0 ? b_acc[2] / b_sum : 0.0};

        // Color-line / Bayesian maximum likelihood alpha: project the pixel color
        // onto the foreground-background color line.
        double const fb[3] = {f_c[0] - b_c[0], f_c[1U] - b_c[1U], f_c[2] - b_c[2]};
        double const norm2 = fb[0] * fb[0] + fb[1U] * fb[1U] + fb[2] * fb[2];
        cv::Vec3b const pix = image.at<cv::Vec3b>(uy, ux);
        double const i_b[3] = {static_cast<double>(pix[0]) - b_c[0], static_cast<double>(pix[1U]) - b_c[1U],
                               static_cast<double>(pix[2]) - b_c[2]};
        double value = (i_b[0] * fb[0] + i_b[1U] * fb[1U] + i_b[2] * fb[2]) / (norm2 + 1.0e-6);
        if (!(value > 0.0)) {
            value = 0.0;
        }
        if (value > 1.0) {
            value = 1.0;
        }
        alpha.at<float>(uy, ux) = static_cast<float>(value);
    }
    return alpha;
}

cv::Mat knn_matting(cv::Mat const& image, cv::Mat const& trimap) {
    detail::validate(image, "image");
    detail::validate_trimap(trimap, image);

    int const rows = image.rows;
    int const cols = image.cols;
    size_t const n = static_cast<size_t>(rows) * static_cast<size_t>(cols);
    int constexpr k_neighbors = 10;

    cv::Mat features(static_cast<int>(n), 5, CV_32F);
    for (int y = 0; y < rows; ++y) {
        auto const* p = image.ptr<cv::Vec3b>(y);
        for (int x = 0; x < cols; ++x) {
            float* f = features.ptr<float>(
                static_cast<int>(static_cast<size_t>(y) * static_cast<size_t>(cols) + static_cast<size_t>(x)));
            f[0] = static_cast<float>(x) / static_cast<float>(cols);
            f[1U] = static_cast<float>(y) / static_cast<float>(rows);
            f[2] = static_cast<float>(p[x][0]) / 255.0f;
            f[3] = static_cast<float>(p[x][1U]) / 255.0f;
            f[4] = static_cast<float>(p[x][2]) / 255.0f;
        }
    }
    cv::flann::Index index(features, cv::flann::KDTreeIndexParams(4));
    cv::Mat nbr_idx, nbr_dist;
    index.knnSearch(features, nbr_idx, nbr_dist, k_neighbors + 1, cv::flann::SearchParams(64));

    // Build a CSR edge list (excluding self matches) and a global feature scale.
    std::vector<int> counts(n, 0);
    double dist_sqr_sum = 0.0;
    double sep_pairs = 0.0;
    for (size_t i = 0; i < n; ++i) {
        int const* idx_row = nbr_idx.ptr<int>(static_cast<int>(i));
        float const* dist_row = nbr_dist.ptr<float>(static_cast<int>(i));
        for (int k = 0; k < k_neighbors + 1; ++k) {
            if (idx_row[k] == static_cast<int>(i)) {
                continue;
            }
            ++counts[i];
            double const d = static_cast<double>(dist_row[k]);
            dist_sqr_sum += d * d;
            sep_pairs += 1.0;
        }
    }
    double const sigma = (sep_pairs > 0.0) ? std::sqrt(dist_sqr_sum / sep_pairs) : 1.0;
    double const inv_2s2 = (sigma > 1.0e-6) ? -1.0 / (2.0 * sigma * sigma) : -1.0e12;

    std::vector<int> offset(n + 1, 0);
    for (size_t i = 0; i < n; ++i) {
        offset[i + 1U] = offset[i] + counts[i];
    }
    size_t const total = static_cast<size_t>(offset[n]);
    std::vector<int> nbr(total);
    std::vector<float> wt(total, 0.0f);
    std::vector<int> fill = offset;
    for (size_t i = 0; i < n; ++i) {
        int const* idx_row = nbr_idx.ptr<int>(static_cast<int>(i));
        float const* dist_row = nbr_dist.ptr<float>(static_cast<int>(i));
        for (int k = 0; k < k_neighbors + 1; ++k) {
            int const j = idx_row[k];
            if (j == static_cast<int>(i)) {
                continue;
            }
            size_t const slot = static_cast<size_t>(fill[i]);
            ++fill[i];
            nbr[slot] = j;
            double const d = static_cast<double>(dist_row[k]);
            wt[slot] = static_cast<float>(std::exp(d * d * inv_2s2));
        }
    }

    std::vector<float> row_sum(n, 0.0f);
    std::vector<float> col_sum(n, 0.0f);
    for (size_t i = 0; i < n; ++i) {
        float s = 0.0f;
        for (int e = offset[i]; e < offset[i + 1U]; ++e) {
            float const w = wt[static_cast<size_t>(e)];
            s += w;
            col_sum[static_cast<size_t>(nbr[static_cast<size_t>(e)])] += w;
        }
        row_sum[i] = s;
    }
    std::vector<float> laplacian_diag(n);
    for (size_t i = 0; i < n; ++i) {
        laplacian_diag[i] = 0.5f * (row_sum[i] + col_sum[i]);
    }

    std::vector<float> mask(n, 0.0f);
    std::vector<float> target(n, 0.0f);
    for (int y = 0; y < rows; ++y) {
        auto const* t = trimap.ptr<uchar>(y);
        for (int x = 0; x < cols; ++x) {
            size_t const i = static_cast<size_t>(y) * static_cast<size_t>(cols) + static_cast<size_t>(x);
            bool const is_fg = t[x] == static_cast<uchar>(TrimapClass::Foreground);
            bool const is_bg = t[x] == static_cast<uchar>(TrimapClass::Background);
            if (is_fg || is_bg) {
                mask[i] = 1.0f;
                target[i] = is_fg ? 1.0f : 0.0f;
            }
        }
    }
    std::vector<float> x = target;

    auto const apply_l = [&](float const* xv, float* out) {
        for (size_t i = 0; i < n; ++i) {
            out[i] = laplacian_diag[i] * xv[i];
        }
        for (size_t i = 0; i < n; ++i) {
            int const* nbr_row = nbr.data() + offset[i];
            float const* wt_row = wt.data() + offset[i];
            float fwd = 0.0f;
            for (int e = 0; e < counts[i]; ++e) {
                size_t const j = static_cast<size_t>(nbr_row[e]);
                float const w = wt_row[e];
                fwd += w * xv[j];
                out[j] -= 0.5f * w * xv[i];
            }
            out[i] -= 0.5f * fwd;
        }
    };

    detail::matting_cg(n, apply_l, laplacian_diag, mask, 100.0f, target, x);

    cv::Mat alpha(rows, cols, CV_32FC1);
    for (size_t i = 0; i < n; ++i) {
        x[i] = std::clamp(x[i], 0.0f, 1.0f);
    }
    std::memcpy(alpha.data, x.data(), n * sizeof(float));
    return alpha;
}

cv::Mat global_matting(cv::Mat const& image, cv::Mat const& trimap) {
    detail::validate(image, "image");
    detail::validate_trimap(trimap, image);
#if defined(CV_VERSION_MAJOR) && (CV_VERSION_MAJOR >= 5 || (CV_VERSION_MAJOR == 4 && CV_VERSION_MINOR >= 7))
    cv::Mat alpha8;
    cv::alphamat::infoFlow(image, trimap, alpha8);
    if (alpha8.empty() || alpha8.size() != image.size() || alpha8.type() != CV_8UC1) {
        throw std::runtime_error("cv::alphamat::infoFlow produced an unexpected output");
    }
    cv::Mat alpha32;
    alpha8.convertTo(alpha32, CV_32F, 1.0 / 255.0);
    return alpha32;
#else
    throw std::runtime_error("global matting requires an OpenCV build with the alphamat module (4.7 or newer)");
#endif
}

}  // namespace matting
}  // namespace vision