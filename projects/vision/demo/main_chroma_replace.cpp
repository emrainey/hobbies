/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Replaces the key color in an input image or video with a background image or video
///        using a chroma key algorithm. When --background is omitted, the key color is
///        replaced with a solid, pure (brighter and more consistent) version of the
///        key color itself. The alpha matte can be remapped with --clip-black/--clip-white
///        to force weakly-keyed colors (such as dark greens in shadows) to a full key.
///        The matting `--type` values (closedform, bayesian, knn, global) reframe the
///        problem as alpha matting: they build a rough trimap from HSV thresholds of the
///        key color and solve for a soft alpha in the unknown band instead of making a
///        hard per-pixel keying decision. `fused` instead builds its trimap from the
///        per-pixel keying signal (green excess) so definite subject pixels are pinned
///        and cannot be eaten by the solver, which only soft-keys the true spill band.
///        Use `--frames` to process only selected frames (written as still images) and
///        `--scale` to downscale for quick iteration. Use `--protect` to pin pixel
///        rectangles as known subject (e.g. a reflective suit that reflects the key
///        screen) so they can never be keyed, and `--protect-auto` to auto-detect the
///        cast per frame with Apple Vision and pin them as subject (both combine).
///        Use `--despill <strength>` to remove excess key color (spill/fringe) from the
///        kept foreground (Ultimatte/Keylight style), `--refine <radius>` to snap the
///        alpha matte onto the color edges of the input with a guided filter, and
///        `-D/--screen-estimate` to key against the actual per-frame screen color
///        (histogram peak of the confident screen region) instead of the pure named
///        `--color`, which fixes gradient/vignetted/unevenly-lit screens. `-T` renders
///        the matting trimaps clean of speckle (morphological open/close) before solving.
/// @copyright Copyright (c) 2026
///

#include <basal/basal.hpp>
#include <basal/options.hpp>
#include <opencv2/opencv.hpp>
#include <vision/chroma_replace.hpp>
#include <vision/frame_spec.hpp>
#include <vision/protect_spec.hpp>
#include <vision/subject_mask.hpp>

#include <cctype>
#include <chrono>
#include <cstdio>
#include <set>
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

/// Builds the output path for a single extracted still, e.g. `out.png` + frame 30 -> `out_frame0030.png`.
std::string still_output_path(std::string const& output, int frame_index) {
    std::string stem = output;
    std::string suffix;
    std::string::size_type const dot = output.rfind('.');
    if (dot != std::string::npos) {
        stem = output.substr(0, dot);
        suffix = output.substr(dot);
    }
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "_frame%04d", frame_index);
    return stem + buffer + suffix;
}

}  // namespace

int main(int argc, char* argv[]) {
    std::string input, type, output, color, background;
    basal::precision clip_black = 0.0, clip_white = 1.0, scale = 1.0;
    basal::precision fg_keep = 0.01, bg_keep = 0.05;
    std::string frames_spec, protect_spec;
    bool protect_auto = false;
    int protect_feather = 3;
    basal::precision despill = 0.0;
    int refine_radius = 0;
    bool screen_estimate = false;
    basal::precision softness = -1.0;
    basal::precision despill_floor = 0.2;
    int trimap_clean = 0;

    basal::options::config opts[] = {
        {"-i", "--input", std::string(""), "Input image or video file path containing the key color"},
        {"-t", "--type", std::string("vlahos"),
         "Chroma key algorithm: vlahos, keylight, mishima, closedform, bayesian, knn, global or fused"},
        {"-o", "--output", std::string("chroma_replaced.mov"), "Output image or video file path"},
        {"-f", "--frames", std::string(""),
         "Only process the given 1-based frame indices, written as stills; a spec is a comma-separated list of "
         "indices and inclusive ranges, e.g. 30 or 5,30-40. Requires an image output extension"},
        {"-s", "--scale", basal::precision(1.0),
         "Downscale or upscale input frames by this factor before processing (e.g. 0.5 for quick previews)"},
        {"-c", "--color", std::string("green"),
         "Key color to replace: green, blue, red, cyan, magenta, yellow, white, black"},
        {"-g", "--fg-keep", basal::precision(0.01),
         "fused only: green excess at or below this is pinned as subject (kept), default 0.01"},
        {"-q", "--bg-keep", basal::precision(0.05),
         "fused only: green excess at or above this is pinned as key screen (replaced), default 0.05"},
        {"-p", "--protect", std::string(""),
         "fused only: pixel rectangles to pin as kept subject, written x,y,w,h separated by semicolons, e.g. "
         "100,50,640,400;900,300,300,500"},
        {"-a", "--protect-auto", bool(false),
         "fused only: Auto-detect people/subjects per frame with Apple Vision and pin them as kept subject "
         "(combines with --protect rectangles)"},
        {"-e", "--protect-feather", int(3),
         "fused only: blur radius in pixels applied to the protect mask edge, making the matte blend smoothly "
         "instead of stepping on a hard boundary; 0 disables"},
        {"-d", "--despill", basal::precision(0.0),
         "Spill suppression in [0,1]; removes excess key color from foreground fringe (e.g. green cast on a subject's "
         "hair/shoulders) after keying, weighted by how much each pixel is kept as subject. 0 disables, 1 fully pulls "
         "the excess key channel down to the level of the other two"},
        {"-R", "--refine", int(0),
         "Guided-filter alpha refinement radius in pixels (e.g. 3): snaps matte edges onto the color edges of the "
         "input, sharpening soft/hard boundaries and removing speckle. 0 disables"},
        {"-D", "--screen-estimate", bool(false),
         "Estimate the actual screen color from the input (histogram-peak of the confident screen region) each frame "
         "and key against that instead of the pure named --color, which fixes gradient/vignetted/illumination-uneven "
         "screens"},
        {"-k", "--clip-black", basal::precision(0.0),
         "Alpha clip black in [0,1]: pixels at or below this keep the foreground"},
        {"-w", "--clip-white", basal::precision(1.0),
         "Alpha clip white in [0,1]: pixels at or above this are fully replaced (e.g. 0.6 brightens shadows)"},
        {"-S", "--softness", basal::precision(-1.0),
         "keylight only: matte of softness in [0,1]; lower values key screen pixels more aggressively (fewer splotches "
         "in the background), higher values keep a wider soft fringe. -1 uses the algorithm default (0.1)"},
        {"-F", "--despill-floor", basal::precision(0.2),
         "Despill floor: the minimum subject fraction (1 - alpha) a pixel must have before spill is removed, in [0,1]. "
         "Slightly-under-1 alpha pixels from a soft solver matte (e.g. fused, shadowed screens) are left strictly "
         "untouched, which stops despill from wobbling the composite background. Lower (e.g. 0) for maximum spill "
         "removal, higher (e.g. 0.3) for a calmer background"},
        {"-T", "--trimap-clean", int(0),
         "matting only: trimap morphological cleanup radius in pixels (e.g. 1-2); erode/dilate the trimap before "
         "solving to remove speckle / stray single-pixel foreground or background islands. 0 disables"},
        {"-b", "--background", std::string(""),
         "Background image/video file path; when omitted, the key color is replaced with a solid, pure version of the "
         "key color"},
    };

    basal::options::process(basal::dimof(opts), opts, argc, argv);

    basal::exit_unless(basal::options::find(opts, "--input", input), __FILE__, __LINE__, "--input must be a file path");
    basal::exit_unless(basal::options::find(opts, "--type", type), __FILE__, __LINE__,
                       "--type must be a short string: vlahos, keylight, mishima, closedform, bayesian, knn, global or "
                       "fused");
    basal::exit_unless(basal::options::find(opts, "--output", output), __FILE__, __LINE__,
                       "--output must be a file path");
    basal::options::find(opts, "--color", color);
    basal::options::find(opts, "--clip-black", clip_black);
    basal::options::find(opts, "--clip-white", clip_white);
    basal::options::find(opts, "--background", background);            // Optional
    basal::options::find(opts, "--frames", frames_spec);               // Optional
    basal::options::find(opts, "--scale", scale);                      // Optional
    basal::options::find(opts, "--fg-keep", fg_keep);                  // Optional (fused only)
    basal::options::find(opts, "--bg-keep", bg_keep);                  // Optional (fused only)
    basal::options::find(opts, "--protect", protect_spec);             // Optional (fused only)
    basal::options::find(opts, "--protect-auto", protect_auto);        // Optional (fused only)
    basal::options::find(opts, "--protect-feather", protect_feather);  // Optional (fused only)
    basal::options::find(opts, "--despill", despill);                  // Optional
    basal::options::find(opts, "--refine", refine_radius);             // Optional
    basal::options::find(opts, "--screen-estimate", screen_estimate);  // Optional
    basal::options::find(opts, "--softness", softness);                // Optional
    basal::options::find(opts, "--despill-floor", despill_floor);      // Optional
    basal::options::find(opts, "--trimap-clean", trimap_clean);        // Optional (matting)

    basal::options::print(basal::dimof(opts), opts);

    bool const stills_mode = !frames_spec.empty();
    std::set<int> const frames = stills_mode ? vision::parse_frames(frames_spec) : std::set<int>{};
    std::string const ext = extension(output);
    bool const video_output = is_video_extension(ext);
    bool const image_output
        = ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp" || ext == "tif" || ext == "tiff";
    basal::exit_unless(!stills_mode || image_output, __FILE__, __LINE__,
                       "--frames writes still images, so --output must end in an image extension such as .png");
    if (scale <= 0.0) {
        std::fprintf(stderr, "warning: --scale must be positive; using 1.0\n");
        scale = 1.0;
    }

    std::vector<cv::Rect> const protect_rects = vision::parse_protect_rects(protect_spec);

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

    double fps = 30.0;
    if (double const f = capture.get(cv::CAP_PROP_FPS); f > 0.0) {
        fps = f;
    }

    cv::VideoWriter writer;
    cv::Mat frame, background_frame, result, protect_mask;
    bool background_exhausted = false;
    bool protect_built = false;
    int frame_index = 0;  // 1-based position in the source video
    int frame_count = 0;  // number of processed frames
    double const total_frames = capture.get(cv::CAP_PROP_FRAME_COUNT);
    auto const started = std::chrono::steady_clock::now();
    while (capture.read(frame) && !frame.empty()) {
        ++frame_index;
        if (stills_mode && !frames.count(frame_index)) {
            continue;  // Skip unrequested frames
        }
        auto const frame_started = std::chrono::steady_clock::now();
        if (frame.type() != CV_8UC3) {
            frame.convertTo(frame, CV_8UC3);
        }
        // The protect rects are in source-pixel coordinates, so rasterize them at the
        // source resolution once and only scale the mask when the frame is scaled.
        if (!protect_built) {
            if (!protect_rects.empty()) {
                protect_mask = vision::build_protect_mask(frame.rows, frame.cols, protect_rects);
            }
            protect_built = true;
        }
        if (scale != 1.0) {
            cv::resize(frame, frame, cv::Size(), scale, scale, cv::INTER_AREA);
            if (!protect_mask.empty()) {
                cv::resize(protect_mask, protect_mask, cv::Size(), scale, scale, cv::INTER_NEAREST);
            }
        }

        // Auto-protect: per frame, detect the cast with Apple Vision and OR its mask into
        // the manual protect rects (both at the working frame size).
        cv::Mat active_protect = protect_mask;
        if (protect_auto) {
            cv::Mat const auto_mask = vision::detect_subject_mask(frame, 3);
            if (!auto_mask.empty()) {
                if (active_protect.empty()) {
                    active_protect = auto_mask;
                } else {
                    cv::bitwise_or(active_protect, auto_mask, active_protect);
                }
            }
        }
        // Per-frame screen color estimate: real key screens are gradient, vignetted and
        // unevenly lit, so key against the actual dominant screen hue rather than the
        // pure named color. Falls back to the named color when no confident screen is found.
        cv::Vec3b active_key = key;
        if (screen_estimate) {
            active_key = vision::estimate_screen_color(frame, key);
        }
        // Feather the combined protect mask so the matte blends at its edges; grey fringe
        // pixels become an unknown band that the solver soft-keys instead of a hard step.
        if (!active_protect.empty() && protect_feather > 0) {
            active_protect = vision::feather_protect_mask(active_protect, protect_feather);
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
            vision::chroma_replace(frame, background_frame, type, active_key, result, static_cast<float>(clip_black),
                                   static_cast<float>(clip_white), static_cast<float>(fg_keep),
                                   static_cast<float>(bg_keep), active_protect, static_cast<float>(despill),
                                   refine_radius, static_cast<float>(softness), static_cast<float>(despill_floor),
                                   trimap_clean);
        } else {
            // No background: replace the key color with a solid, pure (brighter,
            // more consistent) version of the key color itself.
            vision::key_out(frame, type, active_key, result, static_cast<float>(clip_black),
                            static_cast<float>(clip_white), static_cast<float>(fg_keep), static_cast<float>(bg_keep),
                            active_protect, static_cast<float>(despill), refine_radius, static_cast<float>(softness),
                            static_cast<float>(despill_floor), trimap_clean);
        }

        if (stills_mode) {
            cv::imwrite(still_output_path(output, frame_index), result);
        } else if (video_output) {
            writer.write(result);
        } else {
            cv::imwrite(output, result);  // Still image output: the last frame wins
        }
        ++frame_count;

        auto const frame_done = std::chrono::steady_clock::now();
        double const frame_s
            = std::chrono::duration_cast<std::chrono::duration<double>>(frame_done - frame_started).count();
        double const elapsed_s
            = std::chrono::duration_cast<std::chrono::duration<double>>(frame_done - started).count();
        double const display_frames = stills_mode ? static_cast<double>(frames.size()) : total_frames;
        std::printf("frame %d/%d (%.0f%%) ", frame_count, static_cast<int>(display_frames),
                    100.0 * frame_count / display_frames);
        std::printf("%.1fs/frame, %.1fs elapsed", frame_s, elapsed_s);
        if (display_frames > 0.0 && frame_count > 0) {
            std::printf(", ~%.1fs remaining", elapsed_s * (display_frames - frame_count) / frame_count);
        }
        std::printf("\n");
        std::fflush(stdout);
    }
    writer.release();

    basal::exit_unless(frame_count > 0, __FILE__, __LINE__, "No frames were read from input: %s", input.c_str());
    std::printf("Processed %d frame(s) -> %s\n", frame_count, stills_mode ? "<stills>" : output.c_str());
    return 0;
}
