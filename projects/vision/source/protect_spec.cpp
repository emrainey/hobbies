/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Parses "known foreground" rectangle specs into a protection mask for keying.
/// @copyright Copyright (c) 2026
///

#include <vision/protect_spec.hpp>

#include <basal/basal.hpp>

#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <cctype>
#include <string>

namespace vision {

namespace {

/// Parses a single integer token; throws with a useful message on garbage.
long parse_int(std::string const& token, char const* what) {
    if (token.empty()) {
        basal::exception::throw_unless(false, __FILE__, __LINE__, "protect spec %s must be an integer", what);
    }
    long value = 0;
    for (char const c : token) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            basal::exception::throw_unless(false, __FILE__, __LINE__, "protect spec %s must be an integer: '%s'", what,
                                           token.c_str());
        }
        value = value * 10 + static_cast<long>(c - '0');
    }
    return value;
}

bool is_whitespace(char c) {
    return std::isspace(static_cast<unsigned char>(c)) != 0;
}

}  // namespace

std::vector<cv::Rect> parse_protect_rects(std::string const& spec) {
    // Each rect is four comma-separated integers: x,y,w,h. Rectangles are separated by
    // a semicolon so that a single rect's internal commas do not collide with the list
    // separator, e.g. "100,50,640,400;900,300,300,500".
    std::vector<cv::Rect> rects;
    std::string::size_type start = 0;
    while (start <= spec.size()) {
        std::string::size_type const end = spec.find(';', start);
        std::string token = spec.substr(start, end == std::string::npos ? std::string::npos : end - start);
        token.erase(std::remove_if(token.begin(), token.end(), is_whitespace), token.end());
        if (!token.empty()) {
            long values[4];
            std::string::size_type pos = 0;
            for (int i = 0; i < 4; ++i) {
                if (i < 3) {
                    std::string::size_type const comma = token.find(',', pos);
                    if (comma == std::string::npos) {
                        basal::exception::throw_unless(false, __FILE__, __LINE__,
                                                       "protect rect must have exactly 4 integers x,y,w,h: '%s'",
                                                       token.c_str());
                    }
                    values[i] = parse_int(token.substr(pos, comma - pos), "coordinate");
                    pos = comma + 1;
                } else {
                    if (pos >= token.size() || token.find(',', pos) != std::string::npos) {
                        basal::exception::throw_unless(false, __FILE__, __LINE__,
                                                       "protect rect must have exactly 4 integers x,y,w,h: '%s'",
                                                       token.c_str());
                    }
                    values[i] = parse_int(token.substr(pos), "coordinate");
                }
            }
            long const x = values[0], y = values[1], w = values[2], h = values[3];
            basal::exception::throw_unless(w > 0 && h > 0, __FILE__, __LINE__,
                                           "protect rect width and height must be positive: '%s'", token.c_str());
            rects.emplace_back(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), static_cast<int>(h));
        }
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
    return rects;
}

cv::Mat build_protect_mask(int rows, int cols, std::vector<cv::Rect> const& rects) {
    cv::Mat mask = cv::Mat::zeros(rows, cols, CV_8UC1);
    for (cv::Rect const rect : rects) {
        cv::Rect const clipped = rect & cv::Rect(0, 0, cols, rows);
        if (clipped.area() > 0) {
            mask(clipped).setTo(255);
        }
    }
    return mask;
}

cv::Mat feather_protect_mask(cv::Mat const& mask, int radius) {
    if (mask.empty() || radius <= 0) {
        return mask;
    }
    int const blur = 2 * radius + 1;
    cv::Mat blurred;
    cv::GaussianBlur(mask, blurred, cv::Size(blur, blur), 0.0);
    cv::Mat result;
    blurred.convertTo(result, CV_8UC1, 1.0);
    return result;
}

}  // namespace vision