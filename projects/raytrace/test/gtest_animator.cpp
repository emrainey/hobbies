#include <gtest/gtest.h>

#include <raytrace/raytrace.hpp>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

using namespace raytrace;
using namespace raytrace::colors;
using namespace raytrace::operators;
using namespace iso::literals;

using namespace linalg;

using namespace basal::literals;

TEST(AnimatorTest, Default) {
    std::vector<raytrace::animation::Anchor> anchors;
    anchors.push_back(
        raytrace::animation::Anchor{
            raytrace::animation::Attributes{raytrace::point(0, 10, 10), raytrace::point(0, 0, 0), 55.0_deg},
            raytrace::animation::Attributes{raytrace::point(10, 0, 10), raytrace::point(0, 0, -10), 45.0_deg},
            raytrace::animation::Mappers{geometry::mapping::linear, geometry::mapping::linear, geometry::mapping::linear},
            iso::seconds(1.0_p)
        }
    );
    auto animator = animation::Animator{10.0_p, anchors};
    EXPECT_TRUE(animator);
    auto cam = animator(); // first frame
    EXPECT_POINT_EQ(cam.from, raytrace::point(0, 10, 10));
    EXPECT_POINT_EQ(cam.at, raytrace::point(0, 0, 0));
    EXPECT_EQ(cam.fov, 55.0_deg);
    cam = animator(); // second frame
    EXPECT_POINT_EQ(cam.from, raytrace::point(1, 9, 10));
    EXPECT_POINT_EQ(cam.at, raytrace::point(0, 0, -1));
    EXPECT_EQ(cam.fov, 54.0_deg);
    cam = animator(); // third frame
    EXPECT_POINT_EQ(cam.from, raytrace::point(2, 8, 10));
    EXPECT_POINT_EQ(cam.at, raytrace::point(0, 0, -2));
    EXPECT_EQ(cam.fov, 53.0_deg);
    cam = animator(); // fourth frame
    EXPECT_POINT_EQ(cam.from, raytrace::point(3, 7, 10));
    EXPECT_POINT_EQ(cam.at, raytrace::point(0, 0, -3));
    EXPECT_EQ(cam.fov, 52.0_deg);
    cam = animator(); // fifth frame
    EXPECT_POINT_EQ(cam.from, raytrace::point(4, 6, 10));
    EXPECT_POINT_EQ(cam.at, raytrace::point(0, 0, -4));
    EXPECT_EQ(cam.fov, 51.0_deg);
    cam = animator(); // sixth frame
    EXPECT_POINT_EQ(cam.from, raytrace::point(5, 5, 10));
    EXPECT_POINT_EQ(cam.at, raytrace::point(0, 0, -5));
    EXPECT_EQ(cam.fov, 50.0_deg);
    cam = animator(); // seventh frame
    EXPECT_POINT_EQ(cam.from, raytrace::point(6, 4, 10));
    EXPECT_POINT_EQ(cam.at, raytrace::point(0, 0, -6));
    EXPECT_EQ(cam.fov, 49.0_deg);
    cam = animator(); // eighth frame
    EXPECT_POINT_EQ(cam.from, raytrace::point(7, 3, 10));
    EXPECT_POINT_EQ(cam.at, raytrace::point(0, 0, -7));
    EXPECT_EQ(cam.fov, 48.0_deg);
    cam = animator(); // ninth frame
    EXPECT_POINT_EQ(cam.from, raytrace::point(8, 2, 10));
    EXPECT_POINT_EQ(cam.at, raytrace::point(0, 0, -8));
    EXPECT_EQ(cam.fov, 47.0_deg);
    cam = animator(); // tenth frame
    EXPECT_POINT_EQ(cam.from, raytrace::point(9, 1, 10));
    EXPECT_POINT_EQ(cam.at, raytrace::point(0, 0, -9));
    EXPECT_EQ(cam.fov, 46.0_deg);
    cam = animator(); // eleventh frame
    EXPECT_POINT_EQ(cam.from, raytrace::point(10, 0, 10));
    EXPECT_POINT_EQ(cam.at, raytrace::point(0, 0, -10));
    EXPECT_EQ(cam.fov, 45.0_deg);
    EXPECT_FALSE(animator);
}