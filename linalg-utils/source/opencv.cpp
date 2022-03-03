#include "linalg/opencv.hpp"

namespace linalg {

linalg::matrix convert(::cv::Mat& img) {
    linalg::matrix mat = linalg::matrix(img.rows, img.cols);
    return mat.for_each([&](int y, int x, double& v) {
        if (img.type() == CV_8UC1) {
            v = double(img.at<uint8_t>(y, x)) / 255.0;
        } else if (img.type() == CV_8UC3) {
            v = 0.0;
        } else if (img.type() == CV_16UC1) {
            v = double(img.at<uint16_t>(y, x)) / 65535.0;
        } else if (img.type() == CV_32SC1) {
            v = double(img.at<uint32_t>(y, x)) / double(std::numeric_limits<int32_t>::max());
        } else {  // if (img.type() == CV_8UC3) { // BGR usually
            assert(false);
        }
    });
}

::cv::Mat convert(linalg::matrix& mat, int type) {
    ::cv::Mat img(mat.rows, mat.cols, type);
    mat.for_each([&](int y, int x, const double& v) {
        if (type == CV_8UC1) {
            img.at<uint8_t>(y, x) = uint8_t(std::ceil(v * 255.0));
        } else if (type == CV_8UC3) {
            cv::Scalar_<uint8_t> j = jet(v);
            uint8_t* tmp = img.ptr<uint8_t>(y);
            tmp[x * 3 + 0] = j[0];
            tmp[x * 3 + 1] = j[1];
            tmp[x * 3 + 2] = j[2];
        } else if (type == CV_16UC1) {
            img.at<uint16_t>(y, x) = uint16_t(std::ceil(v * 65535.0));
        } else if (type == CV_32SC1) {
            img.at<uint16_t>(y, x) = uint32_t(std::ceil(v * std::numeric_limits<int32_t>::max()));
        } else {
            assert(false);
        }
    });
    return img;
}

::cv::Mat squarish(linalg::matrix& mat, int type) {
    int num = mat.rows * mat.cols;
    int sqr = std::sqrt(num);
    int W = sqr, H = (num / W) + (((num % W) > 0) ? 1 : 0);
    ::cv::Mat img(H, W, type);
    mat.for_each([&](int y, int x, const double& v) {
        // map y,x to iy,ix
        int n = (y * mat.cols) + x;
        int iy = n / W;
        int ix = n % W;
        if (type == CV_8UC1) {
            img.at<uint8_t>(iy, ix) = uint8_t(std::ceil(v * 255.0));
        } else if (type == CV_8UC3) {
            cv::Scalar_<uint8_t> j = jet(v);
            uint8_t* tmp = img.ptr<uint8_t>(iy);
            tmp[ix * 3 + 0] = j[0];
            tmp[ix * 3 + 1] = j[1];
            tmp[ix * 3 + 2] = j[2];
        } else if (type == CV_16UC1) {
            img.at<uint16_t>(iy, ix) = uint16_t(std::ceil(v * 65535.0));
        } else if (type == CV_32SC1) {
            img.at<uint16_t>(iy, ix) = uint32_t(std::ceil(v * std::numeric_limits<int32_t>::max()));
        } else {
            assert(false);
        }
    });
    return img;
}

::cv::Mat each_row_square_tiled(linalg::matrix& mat, int type, int num_tiles_on_each_row) {
    size_t tdim = std::sqrt(mat.cols);  // the dimensions on each tile
    if (tdim * tdim == mat.cols) {
        int num_tiles = mat.rows;
        int H = ((num_tiles / num_tiles_on_each_row) + ((num_tiles % num_tiles_on_each_row) > 0 ? 1 : 0)) * tdim;
        int W = tdim * num_tiles_on_each_row;
        ::cv::Mat img(H, W, type);
        img = cv::Scalar(255, 255, 255);  // fill with white
        // printf("mat=%dx%d t=%d #tiles=%d/%d img=%dx%d\n", mat.rows, mat.cols, tdim, num_tiles, num_tiles_on_each_row,
        // H, W);
        mat.for_each_row([&](int y) {
            int iy = y / num_tiles_on_each_row;
            int ix = y % num_tiles_on_each_row;
            linalg::matrix ws = mat.row(y).resize(tdim, tdim);
            cv::Rect win(ix * tdim, iy * tdim, tdim, tdim);
            // std::cout << "Win: " << win << std::endl;
            cv::Mat sub = ::linalg::convert(ws, CV_8UC3);
            sub.copyTo(img(win));
        });
        return img;
    } else {
        return ::linalg::convert(mat, type);
    }
}

static double interpolate(double val, double y0, double x0, double y1, double x1) {
    return (val - x0) * (y1 - y0) / (x1 - x0) + y0;
}

static double base(double val) {
    if (val <= -0.75)
        return 0.0;
    else if (val <= -0.25)
        return interpolate(val, 0.0, -0.75, 1.0, -0.25);
    else if (val <= 0.25)
        return 1.0;
    else if (val <= 0.75)
        return interpolate(val, 1.0, 0.25, 0.0, 0.75);
    else
        return 0.0;
}

static double red(double gray) {
    return base(gray - 0.5);
}

static double green(double gray) {
    return base(gray);
}

static double blue(double gray) {
    return base(gray + 0.5);
}

cv::Scalar_<uint8_t> jet(cv::Scalar_<uint8_t>& grey) {
    double v = double(grey[0]) / 255.0;
    return jet(v);
}

cv::Scalar_<uint8_t> jet(double v) {
    // must be between 0 and 1.0
    return cv::Scalar(uint8_t(std::ceil(red(v) * 255.0)), uint8_t(std::ceil(green(v) * 255.0)),
                      uint8_t(std::ceil(blue(v) * 255.0)));
}

}  // namespace linalg
