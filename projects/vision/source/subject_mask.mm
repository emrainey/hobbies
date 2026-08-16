/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Apple Vision subject segmentation (VNGenerateForegroundInstanceMaskRequest).
/// @copyright Copyright (c) 2026
///
/// Objective-C++ translation unit: only built on Apple platforms (macOS 14+). Converts
/// an OpenCV BGR frame to a CGImage, runs macOS' foreground-instance segmentation (the
/// same model behind Preview's "the subject" tool), and returns a CV_8UC1 protection
/// mask covering every detected instance.

#include <vision/subject_mask.hpp>

#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>
#import <Vision/Vision.h>

#include <opencv2/imgproc.hpp>

namespace vision {

namespace {

/// Copies a CVPixelBuffer containing a single-channel mask into a cv::Mat, resizing to
/// the input dimensions. Handles 8-bit labels and 32-bit float masks.
cv::Mat copy_mask_buffer(CVPixelBufferRef buffer, int target_rows, int target_cols) {
    cv::Mat mask;
    if (buffer == nullptr) {
        return mask;
    }
    size_t const width = CVPixelBufferGetWidth(buffer);
    size_t const height = CVPixelBufferGetHeight(buffer);
    if (width == 0 || height == 0) {
        return mask;
    }
    if (CVPixelBufferLockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly) != kCVReturnSuccess) {
        return mask;
    }
    void* const base = CVPixelBufferGetBaseAddress(buffer);
    size_t const bytes = CVPixelBufferGetBytesPerRow(buffer);
    if (base != nullptr) {
        OSType const format = CVPixelBufferGetPixelFormatType(buffer);
        if (format == kCVPixelFormatType_OneComponent32Float) {
            cv::Mat raw(static_cast<int>(height), static_cast<int>(width), CV_32FC1, base, bytes);
            cv::Mat thr;
            cv::threshold(raw, thr, 0.5f, 255.0, cv::THRESH_BINARY);
            thr.convertTo(mask, CV_8UC1);
        } else {
            cv::Mat raw(static_cast<int>(height), static_cast<int>(width), CV_8UC1, base, bytes);
            cv::Mat thr;
            cv::threshold(raw, thr, 0, 255, cv::THRESH_BINARY);
            mask = thr;
        }
    }
    CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
    if (!mask.empty() && (mask.rows != target_rows || mask.cols != target_cols)) {
        cv::Mat resized;
        cv::resize(mask, resized, cv::Size(target_cols, target_rows), 0.0, 0.0, cv::INTER_NEAREST);
        mask = resized;
    }
    return mask;
}

}  // namespace

cv::Mat detect_subject_mask(cv::Mat const& bgr, int dilate_px) {
    if (bgr.empty() || bgr.type() != CV_8UC3) {
        return {};
    }
    @autoreleasepool {
        // OpenCV reads BGR, Vision wants RGBA. Build a contiguous RGBA buffer and a
        // CGImage over it (the data provider retains the bytes for the call duration).
        cv::Mat rgba;
        cv::cvtColor(bgr, rgba, cv::COLOR_BGR2RGBA);
        if (!rgba.isContinuous()) {
            rgba = rgba.clone();
        }
        size_t const bytes_per_row = rgba.step;

        CGColorSpaceRef const color_space = CGColorSpaceCreateDeviceRGB();
        CFDataRef const data = CFDataCreate(kCFAllocatorDefault, rgba.data, rgba.total() * 4U);
        CGDataProviderRef const provider = CGDataProviderCreateWithCFData(data);
        CGImageRef const image = CGImageCreate(rgba.cols, rgba.rows, 8, 32, bytes_per_row, color_space,
                                               kCGBitmapByteOrderDefault | kCGImageAlphaLast, provider, nullptr, false,
                                               kCGRenderingIntentDefault);

        cv::Mat result;
        if (image != nullptr) {
            VNGenerateForegroundInstanceMaskRequest* const request = [[VNGenerateForegroundInstanceMaskRequest alloc] init];
            VNImageRequestHandler* const handler = [[VNImageRequestHandler alloc] initWithCGImage:image options:@{}];
            NSError* error = nullptr;
            if ([handler performRequests:@[request] error:&error]) {
                cv::Mat combined = cv::Mat::zeros(bgr.rows, bgr.cols, CV_8UC1);
                for (VNInstanceMaskObservation* const observation in request.results) {
                    CVPixelBufferRef mask_buffer = nullptr;
                    if ([observation respondsToSelector:@selector(generateScaledMaskForImageForInstances:
                                                                        fromRequestHandler:error:)]) {
                        mask_buffer = [observation generateScaledMaskForImageForInstances:observation.allInstances
                                                                      fromRequestHandler:handler
                                                                                   error:&error];
                    }
                    if (mask_buffer == nullptr) {
                        mask_buffer = observation.instanceMask;
                    }
                    cv::Mat const instance = copy_mask_buffer(mask_buffer, bgr.rows, bgr.cols);
                    if (mask_buffer != nullptr && [observation respondsToSelector:@selector(generateScaledMaskForImageForInstances:
                                                                                            fromRequestHandler:error:)]) {
                        CFRelease(mask_buffer);
                    }
                    if (!instance.empty()) {
                        cv::bitwise_or(combined, instance, combined);
                    }
                }
                if (cv::countNonZero(combined) > 0) {
                    if (dilate_px > 0) {
                        cv::Mat const element = cv::getStructuringElement(
                            cv::MORPH_RECT, cv::Size(2 * dilate_px + 1, 2 * dilate_px + 1));
                        cv::dilate(combined, combined, element);
                    }
                    result = combined;
                }
            }
        }
        if (image != nullptr) {
            CGImageRelease(image);
        }
        CGDataProviderRelease(provider);
        CFRelease(data);
        CGColorSpaceRelease(color_space);
        return result;
    }
}

}  // namespace vision