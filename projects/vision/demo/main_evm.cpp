#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <iso/iso.hpp>
#include <opencv2/opencv.hpp>
#include <basal/exception.hpp>

/// @file
/// @brief This file contains the implementation of the Gaussian Pyramid and Temporal Bandpass Filter for a Eulerian
/// Video Magnification (EVM) implementation.
/// @data 2025-02-01
/// @author Erik Rainey
/// @see https://hbenbel.github.io/blog/evm/
/// @see https://people.csail.mit.edu/mrub/papers/vidmag.pdf

class GaussianPyramid {
public:
    GaussianPyramid() = delete;
    GaussianPyramid(const GaussianPyramid&) = delete;
    GaussianPyramid& operator=(const GaussianPyramid&) = delete;
    GaussianPyramid(int levels) : levels_(levels - 1) {
        down_.resize(levels_);
        up_.resize(levels_ - 1);  // further number less
        diff_.resize(levels_ - 1);
    }

    void build(const cv::Mat& image) {
        image_ = image;  // copy
        for (size_t i = 0; i < down_.capacity(); i++) {
            if (i == 0) {
                cv::pyrDown(image_, down_[i]);
            } else {
                cv::pyrDown(down_[i - 1], down_[i]);
            }
        }
        for (size_t i = down_.size() - 1; i > 0; i--) {
            cv::pyrUp(down_[i], up_[i - 1], down_[i - 1].size());
            cv::subtract(down_[i - 1], up_[i - 1], diff_[i - 1]);
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
    cv::Mat image_;
    std::vector<cv::Mat> down_;
    std::vector<cv::Mat> up_;
    std::vector<cv::Mat> diff_;
    int levels_;
};

class TemporalBandpassFilter {
public:
    TemporalBandpassFilter(iso::hertz low, iso::hertz high, iso::hertz framerate, cv::Size size)
        : low_(low), high_(high), fps_(framerate), size_{size} {
        // Create bandpass mask
        filter_mask_ = createBandpassMask();
    }

    std::vector<cv::Mat> filter(const std::vector<cv::Mat>& frames) {
        std::vector<cv::Mat> filteredFrames;
        if (frames.empty())
            return filteredFrames;
        for (const auto& frame : frames) {
            // check the type of the frame to be CV_8U
            throw_exception_unless(frame.type() == CV_8U, "Frame size does not match filter size %u", frame.type());

            // Convert to frequency domain with a real and imaginary pair. The imaginary plane is zeros for now
            cv::Mat planes[] = {cv::Mat_<float>(frame), cv::Mat::zeros(frame.size(), CV_32F)};
            cv::Mat complexImg;
            cv::merge(planes, 2, complexImg);

            // Discrete Fourier Transform
            cv::Mat dft_image;
            cv::dft(complexImg, dft_image, cv::DFT_COMPLEX_OUTPUT);
            printf("DFT: %d, %d Type: %d\n", dft_image.cols, dft_image.rows, dft_image.type());
            // cv::imshow("DFT", dft_image);
            // cv::waitKey(0);

            // Shift zero-frequency component to center
            cv::Mat shifted;
            cv::Mat rearranged_dft;
            // copies all values from dft_image to rearranged_dft and pads the rest with zeros (but there's no border?)
            cv::copyMakeBorder(dft_image, rearranged_dft, 0, 0, 0, 0, cv::BORDER_CONSTANT, cv::Scalar::all(0));
            // cut the image in quarters and rearrange them
            int cx = rearranged_dft.cols / 2;
            int cy = rearranged_dft.rows / 2;

            // make a quadrants of the rearranged_dft image (these are location within rearranged_dft still, they are
            // not new images)
            cv::Mat q0(rearranged_dft, cv::Rect(0, 0, cx, cy));
            cv::Mat q1(rearranged_dft, cv::Rect(cx, 0, cx, cy));
            cv::Mat q2(rearranged_dft, cv::Rect(0, cy, cx, cy));
            cv::Mat q3(rearranged_dft, cv::Rect(cx, cy, cx, cy));

            // rearrange the quadrants of rearranged_dft image using a tmp image "quad"
            // seems like this could be a custom function to speed it up (FIXME)
            cv::Mat quad;
            q0.copyTo(quad);
            q3.copyTo(q0);
            quad.copyTo(q3);
            q1.copyTo(quad);
            q2.copyTo(q1);
            quad.copyTo(q2);

            // Apply mask which will filter out some data by multiplying it by zero
            cv::Mat spectrums;
            std::cout << "Rearranged DFT: " << rearranged_dft.size() << " Type: " << rearranged_dft.type()
                      << " Channels: " << rearranged_dft.channels() << std::endl;
            std::cout << "Filter Mask: " << filter_mask_.size() << " Type: " << filter_mask_.type()
                      << " Channels: " << filter_mask_.channels() << std::endl;
            cv::mulSpectrums(rearranged_dft, filter_mask_, spectrums, 0);
            std::cout << "Spectrums: " << spectrums.size() << " Type: " << spectrums.type()
                      << " Channels: " << spectrums.channels() << std::endl;

            // Inverse shift
            q0 = spectrums(cv::Rect(0, 0, cx, cy));
            q1 = spectrums(cv::Rect(cx, 0, cx, cy));
            q2 = spectrums(cv::Rect(0, cy, cx, cy));
            q3 = spectrums(cv::Rect(cx, cy, cx, cy));

            quad = q0.clone();
            q0 = q3.clone();
            q3 = quad.clone();
            quad = q1.clone();
            q1 = q2.clone();
            q2 = quad.clone();

            // Inverse Fourier Transform
            cv::idft(spectrums, complexImg, cv::DFT_REAL_OUTPUT | cv::DFT_SCALE);

            // Extract magnitude
            cv::Mat filteredFrame;
            cv::magnitude(complexImg.col(0), complexImg.col(1), filteredFrame);

            filteredFrames.push_back(filteredFrame);
        }

        return filteredFrames;
    }

    void show() {
        // cv::imshow("Bandpass Mask (Float)", filter_mask_);
    }

private:
    /// @brief  Creates a BandPass mask (in FLOAT) for multiplying against the DFT image.
    /// @return
    cv::Mat createBandpassMask() {
        cv::Mat mask = cv::Mat::zeros(size_.height, size_.width, CV_32FC2);
        cv::Size half = mask.size() / 2;
        std::cout << "Half: " << half << std::endl;

        // Compute frequency bounds in pixel domain
        int lowBound = static_cast<int>(low_.value / (fps_.value / half.height));
        int highBound = static_cast<int>(high_.value / (fps_.value / half.height));

        // Create bandpass mask
        cv::Rect highFreqRect(half.width - highBound, half.height - highBound, 2 * highBound, 2 * highBound);
        cv::Rect lowFreqRect(half.width - lowBound, half.height - lowBound, 2 * lowBound, 2 * lowBound);
        std::cout << "High Freq Rect: " << highFreqRect << std::endl;
        std::cout << "Low Freq Rect: " << lowFreqRect << std::endl;

        mask(highFreqRect).setTo(1.0);
        mask(lowFreqRect).setTo(0.0);

        return mask;
    }

    iso::hertz low_;
    iso::hertz high_;
    iso::hertz fps_;
    cv::Size size_;
    cv::Mat filter_mask_;
};

int main(int, char*[]) {
    // bool verbose = false;
    bool should_exit = false;
    // static constexpr int levels = 4;
    cv::VideoCapture cap(0);
    cv::Mat image;
    std::vector<cv::Mat> frames;
    cap >> image;
    TemporalBandpassFilter filter(iso::hertz{0.4}, iso::hertz{3}, iso::hertz{30}, image.size());
    filter.show();
    while (not should_exit) {
        cap >> image;  // full frame
        if (image.empty()) {
            std::cout << "Could not capture image\n" << std::endl;
            break;
        }
        // convert to YUV (similar to YIQ)
        cv::Mat yuv;
        cv::cvtColor(image, yuv, cv::COLOR_BGR2YUV);

        GaussianPyramid pyramid(4);
        pyramid.build(yuv);

        // cv::imshow("Original Video", pyramid.getDownPyramid()[0]);
        // cv::imshow("First Level", pyramid.getDownPyramid()[1]);
        // cv::imshow("Second Level", pyramid.getDownPyramid()[2]);

        // cv::imshow("First Level", pyramid.getUpPyramid()[0]);
        // cv::imshow("Second Level Up", pyramid.getUpPyramid()[1]);

        // cv::imshow("First Level Diff", pyramid.getDiffPyramid()[0]);
        // cv::imshow("Second Level Diff", pyramid.getDiffPyramid()[1]);
        // filter.show();

        cv::Mat channels[3];
        // convert the YUV to greyscale by just removing the Y channel
        cv::split(yuv, channels);
        // copy the luminance channel into the frames array to start the temporal filter
        frames.push_back(channels[0]);
        static constexpr size_t LagCount{10U};
        if (frames.size() > LagCount) {
            // filter the frames
            std::vector<cv::Mat> filtered = filter.filter(frames);
            // display the filtered frame
            // cv::imshow("Filtered Video", filtered[0]);
            cv::imshow("Filtered Video", frames[0]);
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
    cv::destroyAllWindows();

    return 0;
}
