#include <gtest/gtest.h>

#include <raytrace/raytrace.hpp>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

using namespace raytrace;
using namespace raytrace::colors;
using namespace raytrace::operators;

using namespace linalg;

TEST(StereoCameraTest, CodedImageLeftRight) {
    using namespace raytrace;
    iso::degrees fov(90);
    const size_t width = 240;
    const size_t height = 120;
    stereo_camera cam(height, width, fov, 1.0, stereo_camera::Layout::LeftRight);
    raytrace::point look_from(-119, 0, 0);
    raytrace::point look_at(1, 0, 0);
    cam.move_to(look_from, look_at);

    cam.first().capture.generate_each([&](const image::point& img_point) -> color {
        const ray world_ray = cam.first().cast(img_point);
        const raytrace::point wrp = world_ray.location();  // just copy
        if (wrp.y > 0) {
            if (wrp.z > 0) {
                return colors::green;
            } else if (wrp.z < 0) {
                return colors::red;
            } else {
                return colors::black;
            }
        } else if (wrp.y < 0) {
            if (wrp.z > 0) {
                return colors::blue;
            } else if (wrp.z < 0) {
                return colors::black;
            } else {
                return colors::black;
            }
        } else {
            return colors::black;
        }
    });

    cam.second().capture.generate_each([&](const image::point& img_point) -> color {
        const ray world_ray = cam.second().cast(img_point);
        const raytrace::point wrp = world_ray.location();  // just copy
        if (wrp.y > 0) {
            if (wrp.z > 0) {
                return colors::cyan;
            } else if (wrp.z < 0) {
                return colors::magenta;
            } else {
                return colors::black;
            }
        } else if (wrp.y < 0) {
            if (wrp.z > 0) {
                return colors::yellow;
            } else if (wrp.z < 0) {
                return colors::white;
            } else {
                return colors::black;
            }
        } else {
            return colors::black;
        }
    });

    raytrace::image capture = cam.merge_images();
    capture.save("coded_stereo_camera_image_lr.ppm");
}


TEST(StereoCameraTest, CodedImageTopBottom) {
    using namespace raytrace;
    iso::degrees fov(90);
    const size_t width = 240;
    const size_t height = 120;
    stereo_camera cam(height, width, fov, 1.0, stereo_camera::Layout::TopBottom);
    raytrace::point look_from(-119, 0, 0);
    raytrace::point look_at(1, 0, 0);
    cam.move_to(look_from, look_at);

    cam.first().capture.generate_each([&](const image::point& img_point) -> color {
        const ray world_ray = cam.first().cast(img_point);
        const raytrace::point wrp = world_ray.location();  // just copy
        if (wrp.y > 0) {
            if (wrp.z > 0) {
                return colors::green;
            } else if (wrp.z < 0) {
                return colors::red;
            } else {
                return colors::black;
            }
        } else if (wrp.y < 0) {
            if (wrp.z > 0) {
                return colors::blue;
            } else if (wrp.z < 0) {
                return colors::black;
            } else {
                return colors::black;
            }
        } else {
            return colors::black;
        }
    });

    cam.second().capture.generate_each([&](const image::point& img_point) -> color {
        const ray world_ray = cam.second().cast(img_point);
        const raytrace::point wrp = world_ray.location();  // just copy
        if (wrp.y > 0) {
            if (wrp.z > 0) {
                return colors::cyan;
            } else if (wrp.z < 0) {
                return colors::magenta;
            } else {
                return colors::black;
            }
        } else if (wrp.y < 0) {
            if (wrp.z > 0) {
                return colors::yellow;
            } else if (wrp.z < 0) {
                return colors::white;
            } else {
                return colors::black;
            }
        } else {
            return colors::black;
        }
    });

    raytrace::image capture = cam.merge_images();
    capture.save("coded_stereo_camera_image_tb.ppm");
}
