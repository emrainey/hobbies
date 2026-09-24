///
/// @file
/// @brief Sweeps through the 3D fractal brownian motion volume along the vertical
/// (z) axis.
/// @note A "Step" trackbar sets the slice spacing as a power-of-two fraction of the
/// volume (1/64 up to 1), and a "Layer" trackbar selects one particular slice of
/// that sweep to magnify below the stripe. So the stripe shows the full density
/// progression while the enlarged panel focuses on a single layer.
///

#include <fourcc/image.hpp>
#include <noise/noise.hpp>
#include <opencv2/opencv.hpp>

noise::vector3 seeds = noise::generate_seed3();

constexpr const size_t step_max = 6;    // step index, 0..6
constexpr const size_t layer_max = 64;  // finest possible layer count - 1
constexpr const size_t octave_max = 12;
constexpr const size_t magnify = 384;  // size of the enlarged selected layer
constexpr const size_t outline = 2;    // highlight border around the selected tile
constexpr const size_t gap = 8;        // space between the stripe and the panel

size_t step_index = 4;  // => step size 1/16
size_t layer_index = 32;
size_t octaves = 6;

cv::Mat render_image;

/// The vertical spacing between slices, a power-of-two fraction of the volume.
noise::precision step_size(void) {
    return 1.0_p / static_cast<noise::precision>(size_t(1) << step_index);
}

/// Number of slices spanning the volume for the current step size.
size_t layer_count(void) {
    return (size_t(1) << step_index) + 1;
}

/// Maps a tile-relative pixel coordinate in [0, size] to a volume coordinate in [0, 1].
noise::precision norm(size_t coordinate, size_t size) {
    return static_cast<noise::precision>(coordinate) / static_cast<noise::precision>(size);
}

/// Samples one constant-z slice of the 3D FBM into a square greyscale tile.
void render_slice(cv::Mat& tile, noise::precision z, noise::precision feature_scale) {
    for (size_t y = 0; y < tile.rows; y++) {
        for (size_t x = 0; x < tile.cols; x++) {
            noise::point3 pnt{norm(x, tile.cols) * feature_scale, norm(tile.rows - 1 - y, tile.rows) * feature_scale,
                              z * feature_scale};
            noise::precision value = noise::fractal_brownian(pnt, seeds, octaves, 2.0_p, 0.5_p, 0.5_p, 1.0_p);
            tile.at<float>(y, x) = static_cast<float>(value);
        }
    }
}

void generate_image(void) {
    size_t layers = layer_count();
    noise::precision delta = step_size();

    // Fit the whole sweep into a bounded stripe by shrinking the tiles as needed.
    size_t sweep_tile = std::clamp(1024 / layers, size_t(16), size_t(256));
    size_t sweep_width = sweep_tile * layers;
    size_t width = std::max(sweep_width, magnify);
    size_t height = sweep_tile + gap + magnify;
    render_image = cv::Mat(height, width, CV_32FC1, cv::Scalar(0.0f));

    noise::precision feature_scale = 6.0_p;

    for (size_t i = 0; i < layers; i++) {
        noise::precision z = static_cast<noise::precision>(i) * delta;
        cv::Mat tile(sweep_tile, sweep_tile, CV_32FC1);
        render_slice(tile, z, feature_scale);

        cv::Mat dst = render_image(cv::Rect(static_cast<int>(i * sweep_tile), 0, sweep_tile, sweep_tile));
        tile.copyTo(dst);

        // Outline the layer that's shown magnified below.
        if (i == layer_index) {
            cv::rectangle(render_image, cv::Point(static_cast<int>(i * sweep_tile), 0),
                          cv::Point(static_cast<int>(i * sweep_tile + sweep_tile - 1), sweep_tile - 1),
                          cv::Scalar(1.0f), outline);
        }
    }

    noise::precision z_sel = static_cast<noise::precision>(layer_index) * delta;
    cv::Mat big(magnify, magnify, CV_32FC1);
    render_slice(big, z_sel, feature_scale);
    cv::Mat dst = render_image(cv::Rect(0, static_cast<int>(sweep_tile + gap), magnify, magnify));
    big.copyTo(dst);

    printf("step index: %zu (delta 1/%zu), layer: %zu (%zu of %zu), octaves: %zu\r\n", step_index,
           size_t(1) << step_index, layer_index, layer_index + 1, layers, octaves);
}

int main(int argc __attribute__((unused)), char* argv[] __attribute__((unused))) {
    std::string window_name{"3D Fractal Brownian Z Sweep"};
    cv::namedWindow(window_name, cv::WINDOW_AUTOSIZE);
    int step_track = static_cast<int>(step_index);
    int layer_track = static_cast<int>(layer_index);
    int octave_track = static_cast<int>(octaves);
    cv::createTrackbar("Step", window_name, &step_track, static_cast<int>(step_max), nullptr, nullptr);
    cv::createTrackbar("Layer", window_name, &layer_track, static_cast<int>(layer_max), nullptr, nullptr);
    cv::createTrackbar("Octaves", window_name, &octave_track, static_cast<int>(octave_max), nullptr, nullptr);

    bool should_quit = false;
    do {
        step_index = static_cast<size_t>(step_track);
        layer_index = static_cast<size_t>(layer_track);
        octaves = static_cast<size_t>(octave_track);
        layer_index = std::min(layer_index, layer_count() - 1);
        generate_image();
        cv::imshow(window_name, render_image);
        int key = cv::waitKey(0) & 0x00FFFFFF;
        switch (key) {
            case 27:  // [fall-through]
            case 'q':
                should_quit = true;
                break;  // ESC or q
            default:
                break;
        }
    } while (not should_quit);
    return 0;
}