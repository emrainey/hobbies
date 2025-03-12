#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <opencv2/opencv.hpp>
#include <basal/exception.hpp>

int main(int argc, char *argv[]) {
    basal::exception::throw_if(argc > 1, __FILE__, __LINE__, "Must have at least 1 arg, has %u", argc);
    cv::Mat image = cv::imread(argv[1]);
    assert(image.type() == CV_8UC3);
    cv::Mat grey, tmp[5], mag, inv, white, blend, minimum;
    cv::cvtColor(image, grey, cv::COLOR_BGRA2GRAY);
    cv::medianBlur(grey, tmp[0], 9);
    cv::imwrite("median.jpg", tmp[0]);
    cv::Scharr(tmp[0], tmp[1], CV_16S, 1, 0);
    cv::convertScaleAbs(tmp[1], tmp[3]);
    cv::Scharr(tmp[0], tmp[2], CV_16S, 0, 1);
    cv::convertScaleAbs(tmp[2], tmp[4]);
    cv::addWeighted(tmp[3], 0.5, tmp[4], 0.5, 0, mag);
    cv::imwrite("magnitude.jpg", mag);
    white = cv::Mat(mag.size(), CV_8UC1);
    white.setTo(cv::Scalar::all(255));
    cv::subtract(white, mag, inv);
    cv::imwrite("inverted.jpg", inv);
    // blend the median image and the inverted edges image
    cv::addWeighted(inv, 0.5, tmp[0], 0.5, 0, blend);
    cv::imwrite("blended.jpg", blend);
    cv::min(inv, tmp[0], minimum);
    cv::imwrite("minimum.jpg", minimum);
    return 0;
}
