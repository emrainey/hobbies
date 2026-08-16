/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Replaces the key color in an input image or video with a background image or video
///        using a chroma key algorithm. When --background is omitted, the key color is
///        replaced with a solid, pure (brighter and more consistent) version of the
///        key color itself. The alpha matte can be remapped with --clip-black/--clip-white
///        to force weakly-keyed colors (such as dark greens in shadows) to a full key.
/// @copyright Copyright (c) 2026
///

#include <basal/basal.hpp>
#include <basal/options.hpp>
#include <opencv2/opencv.hpp>
#include <vision/chroma_replace.hpp>

#include <cctype>
#include <cstdio>
#include <string>

namespace {

std::string to_lower(std::string const& s) {
    std::string out = s;
    for (auto& c : out) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return out;
}

std::string extension(std::string const& path) {
    auto const dot = path.rfind('.');
    if (dot == std::string::npos) {
        return {};
    }
    return to_lower(path.substr(dot + 1));
}

bool is_video_extension(std::string const& ext) {
    return ext == "mov" || ext == "mp4" || ext == "m4v" || ext == "avi" || ext == "mkv";
}

int fourcc_for(std::string const& ext) {
    if (ext == "mp4" || ext == "m4v") {
        return cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    }
    if (ext == "avi") {
        return cv::VideoWriter::fourcc('X', 'V', 'I', 'D');
    }
    if (ext == "mkv") {
        return cv::VideoWriter::fourcc('F', 'F', 'V', '1');
    }
    return cv::VideoWriter::fourcc('q', 't', 'r', 'v');  // QuickTime 7, for .mov
}

}  // namespace

int main(int argc, char* argv[]) {
    std::string input, type, output, color, background;
    basal::precision clip_black = 0.0, clip_white = 1.0;

    basal::options::config opts[] = {
        {"-i", "--input", std::string(""), "Input image or video file path containing the key color"},
        {"-t", "--type", std::string("vlahos"), "Chroma key algorithm: vlahos or mishima"},
        {"-o", "--output", std::string("chroma_replaced.mov"), "Output image or video file path"},
        {"-c", "--color", std::string("green"),
         "Key color to replace: green, blue, red, cyan, magenta, yellow, white, black"},
        {"-k", "--clip-black", basal::precision(0.0),
         "Alpha clip black in [0,1]: pixels at or below this keep the foreground"},
        {"-w", "--clip-white", basal::precision(1.0),
         "Alpha clip white in [0,1]: pixels at or above this are fully replaced (e.g. 0.6 brightens shadows)"},
        {"-b", "--background", std::string(""),
         "Background image/video file path; when omitted, the key color is replaced with a solid, pure version of the "
         "key color"},
    };

    basal::options::process(basal::dimof(opts), opts, argc, argv);

    basal::exit_unless(basal::options::find(opts, "--input", input), __FILE__, __LINE__, "--input must be a file path");
    basal::exit_unless(basal::options::find(opts, "--type", type), __FILE__, __LINE__,
                       "--type must be a short string: vlahos or mishima");
    basal::exit_unless(basal::options::find(opts, "--output", output), __FILE__, __LINE__,
                       "--output must be a file path");
    basal::options::find(opts, "--color", color);
    basal::options::find(opts, "--clip-black", clip_black);
    basal::options::find(opts, "--clip-white", clip_white);
    basal::options::find(opts, "--background", background);  // Optional

    basal::options::print(basal::dimof(opts), opts);

    cv::Vec3b const key = vision::named_color(color);

    cv::VideoCapture capture(input);
    basal::exit_unless(capture.isOpened(), __FILE__, __LINE__, "Failed to open input file (image or video): %s",
                       input.c_str());

    bool const have_background = !background.empty();
    cv::VideoCapture background_capture;
    if (have_background) {
        background_capture.open(background);
        basal::exit_unless(background_capture.isOpened(), __FILE__, __LINE__,
                           "Failed to open background file (image or video): %s", background.c_str());
    }

    std::string const ext = extension(output);
    bool const video_output = is_video_extension(ext);

    double fps = 30.0;
    if (double const f = capture.get(cv::CAP_PROP_FPS); f > 0.0) {
        fps = f;
    }

    cv::VideoWriter writer;
    cv::Mat frame, background_frame, result;
    bool background_exhausted = false;
    int frame_count = 0;
    while (capture.read(frame) && !frame.empty()) {
        if (frame.type() != CV_8UC3) {
            frame.convertTo(frame, CV_8UC3);
        }

        if (video_output && frame_count == 0) {
            writer = cv::VideoWriter(output, fourcc_for(ext), fps, frame.size());
            basal::exit_unless(writer.isOpened(), __FILE__, __LINE__, "Failed to open video writer for output file: %s",
                               output.c_str());
        }

        if (have_background) {
            // On a failed read, background_frame keeps its previous value, so an
            // exhausted background simply holds its last frame (e.g. a still image).
            if (!background_capture.read(background_frame)) {
                if (background_frame.empty()) {
                    std::fprintf(stderr, "warning: no frames could be read from background: %s\n", background.c_str());
                    break;
                }
                if (!background_exhausted) {
                    background_exhausted = true;
                    std::fprintf(stderr, "warning: background ended early; holding its last frame\n");
                }
            }
            if (background_frame.type() != CV_8UC3) {
                background_frame.convertTo(background_frame, CV_8UC3);
            }
            vision::chroma_replace(frame, background_frame, type, key, result, static_cast<float>(clip_black),
                                   static_cast<float>(clip_white));
        } else {
            // No background: replace the key color with a solid, pure (brighter,
            // more consistent) version of the key color itself.
            vision::key_out(frame, type, key, result, static_cast<float>(clip_black), static_cast<float>(clip_white));
        }

        if (video_output) {
            writer.write(result);
        } else {
            cv::imwrite(output, result);  // Still image output: the last frame wins
        }
        ++frame_count;
    }
    writer.release();

    basal::exit_unless(frame_count > 0, __FILE__, __LINE__, "No frames were read from input: %s", input.c_str());
    std::printf("Processed %d frame(s) -> %s\n", frame_count, output.c_str());
    return 0;
}
