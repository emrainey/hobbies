///
/// @file
/// @brief Tunes a 3D FBM density by browsing horizontal (constant-z) slices.
/// @note Renders perlin3, simplex3 and fractal_brownian (3D) side by side so the
/// different generators can be compared at the same slice depth. Trackbars move
/// the slice through the volume and retune the cell scale / FBM octaves.
///

#include <fourcc/image.hpp>
#include <noise/noise.hpp>
#include <opencv2/opencv.hpp>

constexpr const size_t cell_size = 512;  // pixels per tile
constexpr const size_t columns = 3;      // perlin3 | simplex3 | fbm3
constexpr const size_t width = cell_size * columns;
constexpr const size_t height = cell_size;

cv::Mat render_image(height, width, CV_32FC1);

noise::vector3 seeds = noise::generate_seed3();

constexpr const size_t slice_max = 100;
constexpr const size_t scale_max = 64;
constexpr const size_t octave_max = 12;

noise::precision slice_z = 0.5_p;
noise::precision scale = 24.0_p;
size_t octaves = 6;

/// Maps a tile-relative pixel coordinate in [0, cell_size] to a volume coordinate in [0, 1].
noise::precision norm(size_t coordinate) {
    return static_cast<noise::precision>(coordinate) / static_cast<noise::precision>(cell_size);
}

void generate_image(void) {
    printf("slice z: %lf, scale: %lf, octaves: %zu\r\n", slice_z, scale, octaves);
    noise::precision z = slice_z;
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            size_t column = x / cell_size;
            size_t colx = x % cell_size;
            noise::point3 pnt{norm(colx), norm(height - 1 - y), z};
            noise::precision value = 0.0_p;
            switch (column) {
                case 0:
                    value = noise::perlin(pnt, scale, seeds, 245.4993546_p);
                    break;
                case 1:
                    value = noise::simplex(pnt, scale, seeds, 245.4993546_p);
                    break;
                default:
                    value = noise::fractal_brownian(pnt, seeds, octaves, 2.0_p, 0.5_p, 0.5_p, 1.0_p);
                    break;
            }
            render_image.at<float>(y, x) = static_cast<float>(value);
        }
    }
    printf("Render Complete!\r\n");
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
    std::string window_name{"3D Fractal Brownian Slices"};
    cv::namedWindow(window_name, cv::WINDOW_AUTOSIZE);
    int z_track = static_cast<int>(slice_z * slice_max);
    int scale_track = static_cast<int>(scale);
    int octave_track = static_cast<int>(octaves);
    cv::createTrackbar("Z Slice", window_name, &z_track, static_cast<int>(slice_max), nullptr, nullptr);
    cv::createTrackbar("Scale", window_name, &scale_track, static_cast<int>(scale_max), nullptr, nullptr);
    cv::createTrackbar("Octaves", window_name, &octave_track, static_cast<int>(octave_max), nullptr, nullptr);

    bool should_quit = false;
    do {
        slice_z = static_cast<noise::precision>(z_track) / static_cast<noise::precision>(slice_max);
        scale = static_cast<noise::precision>(scale_track);
        octaves = static_cast<size_t>(octave_track);
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