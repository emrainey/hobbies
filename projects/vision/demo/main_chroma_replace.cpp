
/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Replaces the key color in an input image with a background image using a chroma key algorithm
/// @copyright Copyright (c) 2026
///

#include <basal/basal.hpp>
#include <basal/options.hpp>
#include <opencv2/opencv.hpp>
#include <vision/chroma_replace.hpp>

#include <cstdio>
#include <string>

int main(int argc, char* argv[]) {
    std::string input, type, output, color, background;

    basal::options::config opts[] = {
        {"-i", "--input", std::string(""), "Input image file path containing the key color"},
        {"-t", "--type", std::string("vlahos"), "Chroma key algorithm: vlahos or mishima"},
        {"-o", "--output", std::string("chroma_replaced.png"), "Output image file path"},
        {"-c", "--color", std::string("green"), "Key color to replace: green, blue, red, cyan, magenta, yellow, white, black"},
        {"-b", "--background", std::string(""), "Background image file path used to replace the key color (required)"},
    };

    basal::options::process(basal::dimof(opts), opts, argc, argv);

    basal::exit_unless(basal::options::find(opts, "--input", input), __FILE__, __LINE__,
                        "--input must be a file path");
    basal::exit_unless(basal::options::find(opts, "--type", type), __FILE__, __LINE__,
                        "--type must be a short string: vlahos or mishima");
    basal::exit_unless(basal::options::find(opts, "--output", output), __FILE__, __LINE__,
                        "--output must be a file path");
    basal::options::find(opts, "--color", color);
    basal::exit_unless(basal::options::find(opts, "--background", background), __FILE__, __LINE__,
                        "--background must be a file path");

    basal::options::print(basal::dimof(opts), opts);

    cv::Mat image = cv::imread(input);
    basal::exit_unless(!image.empty() && image.type() == CV_8UC3, __FILE__, __LINE__,
                        "Input image must be a color (8UC3) image: %s", input.c_str());
    cv::Mat bg = cv::imread(background);
    basal::exit_unless(!bg.empty() && bg.type() == CV_8UC3, __FILE__, __LINE__,
                        "Background image must be a color (8UC3) image: %s", background.c_str());

    cv::Mat result;
    try {
        vision::chroma_replace(image, bg, type, vision::named_color(color), result);
    } catch (std::exception const& e) {
        std::printf("chroma-replace failed: %s\n", e.what());
        return 1;
    }

    basal::exit_unless(cv::imwrite(output, result), __FILE__, __LINE__,
                        "Failed to write output image: %s", output.c_str());
    return 0;
}