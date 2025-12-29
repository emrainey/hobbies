#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <iso/iso.hpp>
#include <opencv2/opencv.hpp>
#include <basal/exception.hpp>

/// @brief Convert BGR image to YIQ color space
/// @param bgr Input BGR image (CV_8UC3)
/// @param yiq Output YIQ image (CV_32FC3)
void bgrToYiq(const cv::Mat& bgr, cv::Mat& yiq) {
    assert(bgr.type() == CV_8UC3 && "Input must be 8-bit 3-channel BGR image");

    // Convert to float for calculations
    cv::Mat bgr_float;
    bgr.convertTo(bgr_float, CV_32FC3, 1.0 / 255.0);

    // Initialize output
    yiq = cv::Mat::zeros(bgr.size(), CV_32FC3);

    // YIQ conversion matrix (from RGB, but OpenCV uses BGR so we swap R and B)
    // [Y]   [0.299   0.587   0.114 ] [R]
    // [I] = [0.5959 -0.2745 -0.3213] [G]
    // [Q]   [0.2115 -0.5227  0.3111] [B]

    // For BGR: B=0, G=1, R=2
    for (int y = 0; y < bgr_float.rows; ++y) {
        for (int x = 0; x < bgr_float.cols; ++x) {
            cv::Vec3f bgr_pixel = bgr_float.at<cv::Vec3f>(y, x);
            float b = bgr_pixel[0];
            float g = bgr_pixel[1];
            float r = bgr_pixel[2];

            float Y = 0.299f * r + 0.587f * g + 0.114f * b;
            float I = 0.5959f * r - 0.2745f * g - 0.3213f * b;
            float Q = 0.2115f * r - 0.5227f * g + 0.3111f * b;

            yiq.at<cv::Vec3f>(y, x) = cv::Vec3f(Y, I, Q);
        }
    }
}

/// @brief Convert YIQ image back to BGR color space
/// @param yiq Input YIQ image (CV_32FC3)
/// @param bgr Output BGR image (CV_8UC3)
void yiqToBgr(const cv::Mat& yiq, cv::Mat& bgr) {
    assert(yiq.type() == CV_32FC3 && "Input must be 32-bit float 3-channel YIQ image");

    // Initialize output
    cv::Mat bgr_float = cv::Mat::zeros(yiq.size(), CV_32FC3);

    // Inverse YIQ conversion matrix
    // [R]   [1.0   0.956   0.621] [Y]
    // [G] = [1.0  -0.272  -0.647] [I]
    // [B]   [1.0  -1.106   1.703] [Q]

    for (int y = 0; y < yiq.rows; ++y) {
        for (int x = 0; x < yiq.cols; ++x) {
            cv::Vec3f yiq_pixel = yiq.at<cv::Vec3f>(y, x);
            float Y = yiq_pixel[0];
            float I = yiq_pixel[1];
            float Q = yiq_pixel[2];

            float r = 1.0f * Y + 0.956f * I + 0.621f * Q;
            float g = 1.0f * Y - 0.272f * I - 0.647f * Q;
            float b = 1.0f * Y - 1.106f * I + 1.703f * Q;

            // Clamp values to [0, 1]
            r = cv::saturate_cast<float>(r);
            g = cv::saturate_cast<float>(g);
            b = cv::saturate_cast<float>(b);

            bgr_float.at<cv::Vec3f>(y, x) = cv::Vec3f(b, g, r);  // BGR order
        }
    }

    // Convert back to 8-bit
    bgr_float.convertTo(bgr, CV_8UC3, 255.0);
}

/// @file
/// @brief This file contains the implementation of the Gaussian Pyramid and Temporal Bandpass Filter for a Eulerian
/// Video Magnification (EVM) implementation.
/// @data 2025-02-01
/// @author Erik Rainey
/// @see https://hbenbel.github.io/blog/evm/
/// @see https://people.csail.mit.edu/mrub/papers/vidmag.pdf

constexpr static bool verbose = false;
constexpr static bool display = false;

// Global variables for slider control
static int amplification_factor = 11;     // Default amplification (range 0-200)
static int chrominance_attenuation = 10;  // Default chrominance attenuation (range 1-100, represents 0.01-1.0)

// Slider callback (required but can be empty)
void onAmplificationChange(int value, void* userdata) {
    // Slider automatically updates the global variable
    amplification_factor = value;
    static_cast<void>(userdata);
}

void onChrominanceAttenuationChange(int value, void* userdata) {
    // Slider automatically updates the global variable
    chrominance_attenuation = value;
    static_cast<void>(userdata);
}

class GaussianPyramid {
public:
    GaussianPyramid() = delete;
    GaussianPyramid(const GaussianPyramid&) = delete;
    GaussianPyramid& operator=(const GaussianPyramid&) = delete;
    GaussianPyramid(size_t levels) : levels_{levels}, down_{levels_}, up_{levels_ - 1}, diff_{levels_ - 1}, image_{} {
    }

    void build(const cv::Mat& image) {
        image_ = image;  // copy
        if constexpr (verbose) {
            std::cout << "Building Gaussian Pyramid with " << levels_ << " levels. From " << image_.size() << std::endl;
        }
        for (size_t i = 0; i < down_.capacity(); i++) {
            if (i == 0) {
                cv::pyrDown(image_, down_[i]);
                if constexpr (verbose) {
                    std::cout << "Level " << i << " Size: " << down_[i].size() << std::endl;
                }
            } else {
                cv::pyrDown(down_[i - 1], down_[i]);
                if constexpr (verbose) {
                    std::cout << "Level " << i << " Size: " << down_[i].size() << std::endl;
                }
            }
        }
        for (size_t i = down_.size() - 1; i > 0; i--) {
            cv::pyrUp(down_[i], up_[i - 1], down_[i - 1].size());
            if constexpr (verbose) {
                std::cout << "Level " << i << " Size: " << up_[i - 1].size() << std::endl;
            }
            cv::subtract(down_[i - 1], up_[i - 1], diff_[i - 1]);
            if constexpr (verbose) {
                std::cout << "Level " << i << " Size: " << diff_[i - 1].size() << std::endl;
            }
        }
    }

    std::vector<cv::Mat>& getDownPyramid() {
        return down_;
    }

    std::vector<cv::Mat>& getUpPyramid() {
        return up_;
    }

    std::vector<cv::Mat>& getDiffPyramid() {
        return diff_;
    }

private:
    size_t levels_;
    std::vector<cv::Mat> down_;
    std::vector<cv::Mat> up_;
    std::vector<cv::Mat> diff_;
    cv::Mat image_;
};

class TemporalBandpassFilter {
public:
    TemporalBandpassFilter(iso::hertz low, iso::hertz high, iso::hertz framerate, cv::Size size)
        : low_(low), high_(high), fps_(framerate), size_{size} {
        // No longer creating spatial mask - we do temporal filtering per-pixel
        if constexpr (verbose) {
            std::cout << "Temporal filter: " << low_.value << "-" << high_.value << " Hz @ " << fps_.value << " fps"
                      << std::endl;
        }
    }

    std::vector<cv::Mat> filter(const std::vector<cv::Mat>& frames) {
        std::vector<cv::Mat> filteredFrames;
        if (frames.empty())
            return filteredFrames;

        if constexpr (verbose) {
            std::cout << "Temporal filtering " << frames.size() << " frames of size " << frames[0].size() << std::endl;
        }

        // Initialize output frames
        filteredFrames.resize(frames.size());
        for (size_t i = 0; i < frames.size(); i++) {
            filteredFrames[i] = cv::Mat::zeros(frames[0].size(), CV_32F);
        }

        // For each pixel position, extract its temporal signal and filter it
        for (int y = 0; y < frames[0].rows; y++) {
            for (int x = 0; x < frames[0].cols; x++) {
                // Extract temporal signal for this pixel across all frames
                std::vector<float> temporal_signal(frames.size());
                for (size_t t = 0; t < frames.size(); t++) {
                    temporal_signal[t] = frames[t].at<float>(y, x);
                }

                // Apply 1D FFT to temporal signal
                cv::Mat temporal_complex;
                int n = static_cast<int>(temporal_signal.size());
                cv::Mat temporal_planes[] = {
                    cv::Mat(temporal_signal).clone(),  // Real part
                    cv::Mat::zeros(n, 1, CV_32F)       // Imaginary part
                };
                cv::merge(temporal_planes, 2, temporal_complex);

                cv::Mat temporal_dft;
                cv::dft(temporal_complex, temporal_dft, cv::DFT_COMPLEX_OUTPUT);

                // Apply ideal bandpass filter in frequency domain
                applyTemporalBandpassFilter(temporal_dft, frames.size());

                // Inverse FFT back to time domain
                cv::Mat filtered_temporal;
                cv::idft(temporal_dft, filtered_temporal, cv::DFT_REAL_OUTPUT | cv::DFT_SCALE);

                // Store filtered values back into output frames
                for (size_t t = 0; t < frames.size(); t++) {
                    int dt = static_cast<int>(t);
                    filteredFrames[t].at<float>(y, x) = filtered_temporal.at<float>(dt, 0);
                }
            }
        }

        if constexpr (verbose) {
            std::cout << "Temporal filtering complete" << std::endl;
        }

        return filteredFrames;
    }

    void show() {
        if constexpr (verbose) {
            std::cout << "Temporal Bandpass Filter: " << low_.value << "-" << high_.value << " Hz @ " << fps_.value
                      << " fps" << std::endl;
            std::cout << "Processing frame size: " << size_ << std::endl;
        }
        // No spatial mask to show - temporal filtering is done per-pixel
    }

private:
    /// @brief Apply ideal bandpass filter to temporal frequency domain signal
    /// @param temporal_dft Complex DFT of temporal signal (CV_32FC2)
    /// @param num_frames Number of frames in temporal sequence
    void applyTemporalBandpassFilter(cv::Mat& temporal_dft, size_t num_frames) {
        // Calculate frequency bins
        double freq_resolution = fps_.value / static_cast<double>(num_frames);

        // Convert frequency bounds to bin indices
        int low_bin = static_cast<int>(low_.value / freq_resolution);
        int high_bin = static_cast<int>(high_.value / freq_resolution);

        if constexpr (verbose && false) {  // Reduce verbosity
            std::cout << "Freq resolution: " << freq_resolution << " Hz, Low bin: " << low_bin
                      << ", High bin: " << high_bin << std::endl;
        }

        // Apply ideal bandpass filter: set frequencies outside [low_bin, high_bin] to zero
        for (int i = 0; i < temporal_dft.rows; i++) {
            if (i < low_bin || i > high_bin) {
                // Set both real and imaginary parts to zero
                temporal_dft.at<cv::Vec2f>(i, 0) = cv::Vec2f(0.0f, 0.0f);
            }
            // Also handle negative frequencies (symmetric)
            int neg_freq_bin = temporal_dft.rows - i;
            if (neg_freq_bin < low_bin || neg_freq_bin > high_bin) {
                if (neg_freq_bin < temporal_dft.rows) {
                    temporal_dft.at<cv::Vec2f>(neg_freq_bin, 0) = cv::Vec2f(0.0f, 0.0f);
                }
            }
        }
    }

    /// @brief  Creates a BandPass mask (DEPRECATED - now using temporal filtering)
    /// @return
    iso::hertz low_;
    iso::hertz high_;
    iso::hertz fps_;
    cv::Size size_;
};

int main(int, char*[]) {
    using namespace basal;
    using namespace basal::literals;
    bool should_exit = false;
    static constexpr int levels{4};
    cv::VideoCapture cap(0, cv::CAP_ANY);
    assert(cap.isOpened() && "Could not open video device");
    cv::Mat image;
    std::vector<cv::Mat> frames;
    cap >> image;

    // Downsample to 1/2 scale for faster processing
    cv::Mat image_resized;
    cv::resize(image, image_resized, cv::Size(), 0.5, 0.5, cv::INTER_LINEAR);
    image = image_resized;  // Use resized image

    std::cout << "Original Camera Size: " << image_resized.size() << " -> Processing Size: " << image.size()
              << " Type: " << image.type() << std::endl;
    TemporalBandpassFilter filter(iso::hertz{0.4_p}, iso::hertz{3.0_p}, iso::hertz{30.0_p}, image.size());

    // Create control window with sliders
    cv::namedWindow("EVM Result", cv::WINDOW_AUTOSIZE);
    cv::createTrackbar("Amplification Factor", "EVM Result", &amplification_factor, 200, onAmplificationChange);
    cv::createTrackbar("Chrominance Atten x100", "EVM Result", &chrominance_attenuation, 100,
                       onChrominanceAttenuationChange);

    // Create main output window
    cv::namedWindow("EVM Result", cv::WINDOW_AUTOSIZE);

    // filter.show();
    while (not should_exit) {
        cap >> image;  // full frame
        if (image.empty()) {
            std::cout << "Could not capture image\n" << std::endl;
            break;
        }

        // Downsample to 1/2 scale for faster processing
        cv::Mat image_resized;
        cv::resize(image, image_resized, cv::Size(), 0.25, 0.25, cv::INTER_LINEAR);
        image = image_resized;  // Use resized image for processing

        // convert to YIQ color space
        cv::Mat yiq;
        bgrToYiq(image, yiq);

        GaussianPyramid pyramid(levels);
        pyramid.build(yiq);

        // cv::imshow("Original Video", pyramid.getDownPyramid()[0]);
        // cv::imshow("First Level", pyramid.getDownPyramid()[1]);
        // cv::imshow("Second Level", pyramid.getDownPyramid()[2]);
        // For proper visualization of YIQ pyramid levels, convert back to BGR
        if constexpr (display) {
            cv::Mat bgr_display;
            // yiqToBgr(pyramid.getDownPyramid()[0], bgr_display);
            // cv::imshow("Pyramid Level 0 (BGR)", bgr_display);

            yiqToBgr(pyramid.getDownPyramid()[1], bgr_display);
            cv::imshow("Pyramid Level 1 (BGR)", bgr_display);
        }
        // cv::imshow("First Level", pyramid.getUpPyramid()[0]);
        // cv::imshow("Second Level Up", pyramid.getUpPyramid()[1]);

        // cv::imshow("First Level Diff", pyramid.getDiffPyramid()[0]);
        // cv::imshow("Second Level Diff", pyramid.getDiffPyramid()[1]);
        // filter.show();

        cv::Mat channels[3];
        // split YIQ channels: Y=luminance, I=chrominance, Q=chrominance
        cv::split(yiq, channels);
        // copy the luminance channel (Y) into the frames array for temporal filtering
        frames.push_back(channels[0]);
        static constexpr size_t LagCount{10U};
        if (frames.size() > LagCount) {
            // filter the frames
            std::vector<cv::Mat> filtered = filter.filter(frames);

            // Get current slider values
            double alpha = static_cast<double>(amplification_factor) / 10.0;  // Convert 0-200 to 0-20.0 range
            double attenuation
                = static_cast<double>(chrominance_attenuation) / 100.0;  // Convert 1-100 to 0.01-1.0 range

            if constexpr (verbose) {
                std::cout << "Amplification: " << alpha << ", Chrominance attenuation: " << attenuation << std::endl;
            }

            // EVM Reconstruction: Amplify and add back to original
            // Use the CURRENT frame's Y channel, not the oldest one for proper temporal alignment
            cv::Mat current_y = channels[0];   // Current Y channel (luminance)
            cv::Mat filtered_y = filtered[0];  // Temporally filtered Y channel (from oldest frame)

            // Split original YIQ for reconstruction
            cv::Mat channels[3];
            cv::split(yiq, channels);
            cv::Mat original_i = channels[1];  // I channel (chrominance)
            cv::Mat original_q = channels[2];  // Q channel (chrominance)

            // Apply amplification with chrominance attenuation
            cv::Mat amplified_y;
            cv::multiply(filtered_y, cv::Scalar(alpha), amplified_y);

            // For motion magnification: only modify Y channel, leave I and Q unchanged
            // Reconstruct YIQ by adding amplified changes only to Y channel
            cv::Mat reconstructed_y, reconstructed_i, reconstructed_q;
            cv::add(current_y, amplified_y, reconstructed_y);
            reconstructed_i = original_i.clone();  // Keep original I channel unchanged
            reconstructed_q = original_q.clone();  // Keep original Q channel unchanged

            // Merge reconstructed YIQ channels
            cv::Mat reconstructed_yiq;
            cv::Mat recon_channels[] = {reconstructed_y, reconstructed_i, reconstructed_q};
            cv::merge(recon_channels, 3, reconstructed_yiq);

            // Convert back to BGR for display
            cv::Mat evm_result;
            yiqToBgr(reconstructed_yiq, evm_result);

            // Display main EVM result
            cv::imshow("EVM Result", evm_result);

            // Optional: Show debug visualizations
            if constexpr (display) {
                cv::imshow("Original Y Channel", current_y);

                cv::Mat filtered_display;
                cv::normalize(filtered_y, filtered_display, 0, 1, cv::NORM_MINMAX);
                cv::imshow("Temporal Filter Output", filtered_display);

                cv::Mat amplified_display;
                cv::normalize(amplified_y, amplified_display, 0, 1, cv::NORM_MINMAX);
                cv::imshow("Amplified Changes", amplified_display);
            }

            frames.erase(frames.begin());
            // capture the key press to end this madness
            int key = cv::waitKey(1) & 0x00FFFFFF;  // wait for keypress
            switch (key) {
                case 27:
                    [[fallthrough]];
                case 'q':
                    should_exit = true;
                    break;  // ESC or q
            }
        }
    }
    cap.release();
    cv::destroyAllWindows();

    return 0;
}
