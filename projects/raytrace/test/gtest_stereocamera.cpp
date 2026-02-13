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
    size_t const width = 240;
    size_t const height = 120;
    stereo_camera cam(height, width, fov, 0.0_p, stereo_camera::Layout::LeftRight);
    raytrace::point look_from(-6000, 0, 0);
    raytrace::point look_at(0, 0, 0);
    cam.move_to(look_from, look_at);

    camera& first = *cam.begin();
    camera& second = *std::next(cam.begin());

    first.capture.generate_each([&](image::point const& img_point) -> color {
        ray const world_ray = first.cast(img_point);
        raytrace::point const wrp = world_ray.location();  // just copy
        if (wrp.y() > 0) {
            if (wrp.z() > 0) {
                return colors::green;
            } else if (wrp.z() < 0) {
                return colors::red;
            } else {
                return colors::pink;
            }
        } else if (wrp.y() < 0) {
            if (wrp.z() > 0) {
                return colors::blue;
            } else if (wrp.z() < 0) {
                return colors::black;
            } else {
                return colors::pink;
            }
        } else {
            return colors::pink;
        }
    });

    second.capture.generate_each([&](image::point const& img_point) -> color {
        ray const world_ray = second.cast(img_point);
        raytrace::point const wrp = world_ray.location();  // just copy
        if (wrp.y() > 0) {
            if (wrp.z() > 0) {
                return colors::cyan;
            } else if (wrp.z() < 0) {
                return colors::magenta;
            } else {
                return colors::pink;
            }
        } else if (wrp.y() < 0) {
            if (wrp.z() > 0) {
                return colors::yellow;
            } else if (wrp.z() < 0) {
                return colors::white;
            } else {
                return colors::pink;
            }
        } else {
            return colors::pink;
        }
    });

    raytrace::image capture = cam.merge_images();
    capture.save("coded_stereo_camera_image_lr.ppm");
}

TEST(StereoCameraTest, CodedImageTopBottom) {
    using namespace raytrace;
    iso::degrees fov(90);
    size_t const width = 240;
    size_t const height = 120;
    stereo_camera cam(height, width, fov, 0.0_p, stereo_camera::Layout::TopBottom);
    raytrace::point look_from(-6000, 0, 0);
    raytrace::point look_at(0, 0, 0);
    cam.move_to(look_from, look_at);

    camera& first = *cam.begin();
    camera& second = *std::next(cam.begin());

    first.capture.generate_each([&](image::point const& img_point) -> color {
        ray const world_ray = first.cast(img_point);
        raytrace::point const wrp = world_ray.location();  // just copy
        if (wrp.y() > 0) {
            if (wrp.z() > 0) {
                return colors::green;
            } else if (wrp.z() < 0) {
                return colors::red;
            } else {
                return colors::black;
            }
        } else if (wrp.y() < 0) {
            if (wrp.z() > 0) {
                return colors::blue;
            } else if (wrp.z() < 0) {
                return colors::black;
            } else {
                return colors::black;
            }
        } else {
            return colors::black;
        }
    });

    second.capture.generate_each([&](image::point const& img_point) -> color {
        ray const world_ray = second.cast(img_point);
        raytrace::point const wrp = world_ray.location();  // just copy
        if (wrp.y() > 0) {
            if (wrp.z() > 0) {
                return colors::cyan;
            } else if (wrp.z() < 0) {
                return colors::magenta;
            } else {
                return colors::black;
            }
        } else if (wrp.y() < 0) {
            if (wrp.z() > 0) {
                return colors::yellow;
            } else if (wrp.z() < 0) {
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

TEST(StereoCameraTest, Separation) {
    using namespace raytrace;
    iso::degrees fov(90);
    size_t const width = 240;
    size_t const height = 120;
    double const separation = 10.0_p;
    double const sqrt2 = std::sqrt(2.0_p);
    double const hsepsqrt2 = separation / (2 * sqrt2);
    using set = raytrace::point[4];
    set sets[] = {
        {raytrace::point{0.0_p, 0.0_p, 0.0_p}, raytrace::point{0.0_p, 100, 0.0_p},
         raytrace::point{-5.0_p, 0.0_p, 0.0_p}, raytrace::point{5.0_p, 0.0_p, 0.0_p}},
        {raytrace::point{0.0_p, 0.0_p, 0.0_p}, raytrace::point{0.0_p, -100, 0.0_p},
         raytrace::point{5.0_p, 0.0_p, 0.0_p}, raytrace::point{-5.0_p, 0.0_p, 0.0_p}},
        {raytrace::point{0.0_p, 0.0_p, 0.0_p}, raytrace::point{100, 0.0_p, 0.0_p}, raytrace::point{0.0_p, 5.0_p, 0.0_p},
         raytrace::point{0.0_p, -5.0_p, 0.0_p}},
        {raytrace::point{0.0_p, 0.0_p, 0.0_p}, raytrace::point{-100, 0.0_p, 0.0_p},
         raytrace::point{0.0_p, -5.0_p, 0.0_p}, raytrace::point{0.0_p, 5.0_p, 0.0_p}},
        {raytrace::point{0.0_p, 0.0_p, 0.0_p}, raytrace::point{100 / sqrt2, 100.0_p / sqrt2, 0.0_p},
         raytrace::point{-hsepsqrt2, hsepsqrt2, 0.0_p}, raytrace::point{hsepsqrt2, -hsepsqrt2, 0.0_p}},
    };
    printf("Running %zu tests\n", dimof(sets));
    for (size_t i = 0; i < dimof(sets); i++) {
        stereo_camera cam(height, width, fov, separation, stereo_camera::Layout::LeftRight);
        raytrace::point look_from = sets[i][0];
        raytrace::point look_at = sets[i][1];
        cam.move_to(look_from, look_at);

        camera& left = *cam.begin();
        camera& right = *std::next(cam.begin());

        raytrace::point left_camera_position = sets[i][2];
        raytrace::point right_camera_position = sets[i][3];

        ASSERT_POINT_EQ(left.position(), left_camera_position);
        ASSERT_POINT_EQ(right.position(), right_camera_position);

        ASSERT_NEAR(tan(5.0_p / 100.0_p), cam.toe_in().value, 0.01_p) << "iteration " << i;
    }
}