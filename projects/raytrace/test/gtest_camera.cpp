#include <gtest/gtest.h>

#include <raytrace/raytrace.hpp>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

using namespace raytrace;
using namespace raytrace::colors;
using namespace raytrace::operators;

using namespace linalg;

using namespace basal::literals;

class CameraTest : public ::testing::Test {
public:
    CameraTest() : image_height{2}, image_width{2}, field_of_view{90} {
    }
    ~CameraTest() {
    }

    void SetUp() {
        m_camera = std::make_unique<raytrace::camera>(image_height, image_width, field_of_view);
        ASSERT_NE(nullptr, m_camera);
    }

    void TearDown() {
        // nothing yet
    }

protected:
    size_t const image_height;
    size_t const image_width;
    vector const camera_translation;
    iso::degrees field_of_view;
    std::unique_ptr<raytrace::camera> m_camera;
};

TEST_F(CameraTest, DefaultAttributesTest) {
    raytrace::vector camera_forward = R3::basis::X;
    raytrace::vector camera_left = R3::basis::Y;
    raytrace::vector camera_up = R3::basis::Z;
    raytrace::point look_at(1.0_p, 0.0_p, 0.0_p);
    raytrace::point look_from = geometry::R3::origin;
    linalg::matrix const camera_intrinsics{{{1, 0, -1}, {0, 1, -1}, {0, 0, 1}}};

    // the default camera point is the origin
    ASSERT_POINT_EQ(m_camera.get()->position(), look_from);
    ASSERT_POINT_EQ(m_camera.get()->at(), look_at);

    // the default forward
    ASSERT_RAY_EQ(look_from, camera_forward, m_camera.get()->forward());
    ASSERT_RAY_EQ(look_from, camera_up, m_camera.get()->up());
    ASSERT_RAY_EQ(look_from, camera_left, m_camera.get()->left());
    ASSERT_MATRIX_EQ(camera_intrinsics, m_camera.get()->intrinsics());
}

TEST_F(CameraTest, AttributesTest) {
    raytrace::point look_at(2.0_p, 4.0_p, 1.0_p);
    raytrace::point look_from(1.0_p, -1.0_p, -1.0_p);
    raytrace::vector camera_forward(look_at - look_from);
    raytrace::vector camera_left{{-5.0_p, 1.0_p, 0.0_p}};
    raytrace::vector camera_up{{-2.0_p, -10.0_p, 26.0_p}};
    precision f = camera_forward.magnitude();
    // approximately correct
    linalg::matrix const camera_intrinsics{{{f, 0.0_p, -f}, {0.0_p, f, -f}, {0.0_p, 0.0_p, f}}};

    // move the camera position.
    m_camera.get()->position(look_from);
    m_camera.get()->at(look_at);
    ASSERT_POINT_EQ(look_from, m_camera.get()->position());
    ASSERT_POINT_EQ(look_at, m_camera.get()->at());

    camera_forward.print("Estimated Camera Forward");
    ASSERT_RAY_EQ(look_from, camera_forward, m_camera.get()->forward());
    ASSERT_RAY_EQ(look_from, camera_left, m_camera.get()->left());
    ASSERT_RAY_EQ(look_from, camera_up, m_camera.get()->up());
    ASSERT_MATRIX_EQ(camera_intrinsics, m_camera.get()->intrinsics());
}

TEST_F(CameraTest, CastingRays) {
    // we're going to construct a camera view which is very easy to precision check
    // but not one that most situations would use.
    // by having a 2x2 image at 1 pixel scale, the math should be easy to work out by hand
    m_camera.get()->print("Default");

    try {
        iso::degrees rs[] = {iso::degrees(0.0_p), iso::degrees(0.0_p), iso::degrees(0.0_p)};
        m_camera.get()->rotation(rs[0], rs[1], rs[2]);
        ASSERT_TRUE(m_camera.get()->rotation() == linalg::matrix::identity(3, 3));

        constexpr size_t const count = 4;
        image::point points[count]
            = {image::point(0.5_p, 0.5_p), image::point(1.5_p, 0.5_p), image::point(0.5_p, 1.5_p), image::point(1.5_p, 1.5_p)};
        ray rays[count] = {
            m_camera.get()->cast(points[0]),
            m_camera.get()->cast(points[1]),
            m_camera.get()->cast(points[2]),
            m_camera.get()->cast(points[3]),
        };
        for (size_t i = 0; i < count; i++) {
            std::cout << "Cast Ray[" << i << "] = " << rays[i] << std::endl;
        }

        // casted rays should be in a YZ plane at X==1.
        raytrace::point world_ray_locations[] = {
            raytrace::point{1, 0.5_p, 0.5_p},
            raytrace::point{1, -0.5_p, 0.5_p},
            raytrace::point{1, 0.5_p, -0.5_p},
            raytrace::point{1, -0.5_p, -0.5_p},
        };
        for (size_t i = 0; i < count; i++) {
            ASSERT_TRUE(rays[i].location() == world_ray_locations[i]);
        }
    } catch (basal::exception& e) {
        ASSERT_TRUE(false);
    }
}

TEST(CameraTest2, CodedImage) {
    using namespace raytrace;
    iso::degrees fov(90);
    size_t const width = 240;
    size_t const height = 120;
    camera cam(height, width, fov);
    raytrace::point look_from(-119, 0, 0);
    raytrace::point look_at(1, 0, 0);
    cam.move_to(look_from, look_at);

    // default camera position looking down X is assumed
    ASSERT_VECTOR_EQ(R3::basis::X, cam.forward().direction().normalized());
    ASSERT_VECTOR_EQ(R3::basis::Y, cam.left().direction().normalized());
    ASSERT_VECTOR_EQ(R3::basis::Z, cam.up().direction().normalized());

    cam.capture.generate_each([&](image::point const& img_point) -> color {
        ray const world_ray = cam.cast(img_point);
        raytrace::point const wrp = world_ray.location();  // just copy
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
                return colors::yellow;
            } else {
                return colors::black;
            }
        } else {
            return colors::black;
        }
    });
    cam.capture.save("coded_camera_image.ppm");
}
