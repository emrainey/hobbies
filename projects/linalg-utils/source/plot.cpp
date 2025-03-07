
#include "linalg/plot.hpp"

namespace linalg {

constexpr const static bool debug = false;

void plot_points(std::string name, const std::vector<geometry::point_<2>> &data, const std::vector<size_t> &indexes,
                 const std::vector<cv::Scalar> &colors, int timeout_ms) {
    cv::String window_name(name);
    // local_assert(data.size() == domain.cols, "Domains must match");
    cv::Scalar blue(255, 0, 0);
    cv::Scalar red(0, 0, 255);
    cv::Scalar black(0, 0, 0);
    int border = 20;
    int b2 = border >> 1;
    // find the min/max for x & y
    precision x_min, y_min = x_min = std::numeric_limits<precision>::max();
    precision x_max, y_max = x_max = std::numeric_limits<precision>::min();
    for (auto &d : data) {
        if (y_min > d.y)
            y_min = d.y;
        if (y_max < d.y)
            y_max = d.y;
        if (x_min > d.x)
            x_min = d.x;
        if (x_max < d.x)
            x_max = d.x;
    }
    precision y_range = y_max - y_min;
    if (debug) {
        printf("y_min:%lf y_max:%lf y_range:%lf\n", y_min, y_max, y_range);
    }
    precision x_range = x_max - x_min;
    if (debug) {
        printf("x_min:%lf x_max:%lf x_range:%lf\n", x_min, x_max, x_range);
    }

    size_t width = static_cast<size_t>(x_range);
    size_t height = static_cast<size_t>(y_range);
    if (debug) {
        printf("Active area: %zux%zu\n", width, height);
    }

    cv::Size window_size(border + width, border + height);
    if (debug) {
        printf("Window: %ux%u\n", window_size.width, window_size.height);
    }
    cv::Mat img(window_size, CV_8UC3);

    img = cv::Scalar::all(255);  // clear the image with white
    // where will the render offset be?
    int x_off = b2;
    int y_off = b2;
    int x0 = x_off;
    int y0 = y_off;

    // find x zero
    if (x_min > 0 || x_max < 0) {
        x0 = x_off + (x_range) / 2;
    } else {
        x0 = x_off + (0 - x_min);
    }
    // find y zero
    if (y_min > 0 || y_max < 0) {
        // this won't cross the zero line
        y0 = y_off + (y_range) / 2;
    } else {
        y0 = y_off + (0 - y_min);
    }

    cv::Point y_top(x0, y0 - (y_range / 2));
    cv::Point y_btm(x0, y0 + (y_range / 2));
    cv::line(img, y_top, y_btm, blue, 2);

    cv::Point x_left(x0 - (x_range / 2), y0);
    cv::Point x_right(x0 + (x_range / 2), y0);
    cv::line(img, x_left, x_right, red, 2);

    for (size_t i = 0; i < data.size(); i++) {
        int x = x0 + data[i].x;
        int y = y0 + data[i].y;
        cv::Point pt{x, y};
        cv::circle(img, pt, 1, colors[indexes[i]], 1);
    }

    ::cv::Mat flipped(window_size, CV_8UC3);  // new image for the flip
    ::cv::flip(img, flipped, 0);

    do {
        ::cv::imshow(window_name, flipped);
        int key = ::cv::waitKey(timeout_ms);
        if (key == -1)
            return;
        switch (key & 0xFFFF) {
            case 'q':
                return;
            default:
                break;
        }
    } while (1);
}

}  // namespace linalg
