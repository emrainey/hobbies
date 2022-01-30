#include <gtest/gtest.h>
#include <raytrace/raytrace.hpp>

#include "linalg/gtest_helper.hpp"
#include "geometry/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

using namespace raytrace;
using namespace raytrace::colors;
using namespace raytrace::operators;

using namespace linalg;

class CameraTest : public ::testing::Test {
public:
    CameraTest()
        : image_height(2)
        , image_width(2)
        , field_of_view(90)
        {}
    ~CameraTest() {}

    void SetUp() {
        m_camera = std::make_unique<raytrace::camera>(image_height, image_width, field_of_view);
        ASSERT_NE(nullptr, m_camera);
    }

    void TearDown() {
        // nothing yet
    }

protected:
    const size_t image_height;
    const size_t image_width;
    const vector camera_translation;
    iso::degrees field_of_view;
    std::unique_ptr<raytrace::camera> m_camera;
};

TEST_F(CameraTest, DefaultAttributesTest) {
    raytrace::vector camera_forward = R3::basis::X;
    raytrace::vector camera_left = R3::basis::Y;
    raytrace::vector camera_up = R3::basis::Z;
    raytrace::point look_at(1.0, 0.0, 0.0);
    raytrace::point look_from = geometry::R3::origin;
    const linalg::matrix camera_intrinsics{{
        { 1,  0, -1},
        { 0,  1, -1},
        { 0,  0,  1}
    }};

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
    raytrace::point look_at(2.0, 4.0, 1.0);
    raytrace::point look_from(1.0, -1.0, -1.0);
    raytrace::vector camera_forward(look_at - look_from);
    raytrace::vector camera_left{{-5.0, 1.0, 0.0}};
    raytrace::vector camera_up{{-2.0, -10.0, 26.0}};
    double f = camera_forward.magnitude();
    // approximately correct
    const linalg::matrix camera_intrinsics{{
        { f,  0, -f},
        { 0,  f, -f},
        { 0,  0,  f}
    }};

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
    // we're going to construct a camera view which is very easy to double check
    // but not one that most situations would use.
    // by having a 2x2 image at 1 pixel scale, the math should be easy to work out by hand
    m_camera.get()->print("Default");

    try {
        iso::degrees rs[] = {
            iso::degrees(0.0),
            iso::degrees(0.0),
            iso::degrees(0.0)
        };
        m_camera.get()->rotation(rs[0], rs[1], rs[2]);
        ASSERT_TRUE(m_camera.get()->rotation() == linalg::matrix::identity(3, 3));

        constexpr const size_t count = 4;
        image::point points[count] = {
            image::point(0.5,0.5), image::point(1.5,0.5),
            image::point(0.5,1.5), image::point(1.5,1.5)
        };
        ray rays[count] = {
            m_camera.get()->cast(points[0]), m_camera.get()->cast(points[1]),
            m_camera.get()->cast(points[2]), m_camera.get()->cast(points[3]),
        };
        for (size_t i = 0; i < count; i++) {
            std::cout << "Cast Ray[" << i << "] = " << rays[i] << std::endl;
        }

        // casted rays should be in a YZ plane at X==1.
        raytrace::point world_ray_locations[] = {
            raytrace::point(1, 0.5,  0.5), raytrace::point(1, -0.5,  0.5),
            raytrace::point(1, 0.5, -0.5), raytrace::point(1, -0.5, -0.5),
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
    const size_t width = 240;
    const size_t height = 120;
    camera cam(height, width, fov);
    raytrace::point look_from(-119, 0, 0);
    raytrace::point look_at(1, 0, 0);
    cam.move_to(look_from, look_at);

    // default camera position looking down X is assumed
    ASSERT_VECTOR_EQ(R3::basis::X, cam.forward().direction().normalized());
    ASSERT_VECTOR_EQ(R3::basis::Y, cam.left().direction().normalized());
    ASSERT_VECTOR_EQ(R3::basis::Z, cam.up().direction().normalized());

    cam.capture.generate_each([&](const image::point& img_point) -> color {
        const ray world_ray = cam.cast(img_point);
        const raytrace::point wrp = world_ray.location(); // just copy
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
