#include <gtest/gtest.h>

#include <chrono>
#include <raytrace/raytrace.hpp>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

using namespace raytrace;
using namespace raytrace::colors;
using namespace raytrace::operators;

using namespace linalg;

class PerfCounter : public testing::Test {
public:
    void SetUp() {
        start = std::chrono::steady_clock::now();
    }

    void TearDown() {
        diff = std::chrono::steady_clock::now() - start;
        rate = (precision(number_of_ops) / (diff.count() / 1E9)) / 1E6;
        std::cout << activity << " rate:" << rate << " per sec, Period: " << diff.count() << " nsec" << std::endl;
    }

protected:
    static constexpr size_t number_of_ops = 10'000;
    std::chrono::time_point<std::chrono::steady_clock> start;
    std::chrono::duration<precision> diff;
    precision rate;
    std::string activity;
};

TEST_F(PerfCounter, IntersectionsPlane) {
    raytrace::ray r{raytrace::point{0, 0, 2}, -R3::basis::Z};
    raytrace::objects::plane obj(R3::origin, R3::basis::Z);
    activity = std::string("plane intersections");
    for (size_t count = 0; count < number_of_ops; count++) {
        geometry::intersection hit = obj.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(hit));
    }
}

TEST_F(PerfCounter, IntersectionsSquare) {
    raytrace::ray r{raytrace::point{0.98, 0.98, 2}, -R3::basis::Z};
    raytrace::objects::square obj(R3::origin, R3::basis::Z, 1.0, 1.0);
    activity = std::string("square intersections");
    for (size_t count = 0; count < number_of_ops; count++) {
        geometry::intersection hit = obj.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(hit));
    }
}

TEST_F(PerfCounter, IntersectionsRing) {
    raytrace::ray r{raytrace::point{1.5, 0, 2}, -R3::basis::Z};
    raytrace::objects::ring obj(R3::origin, R3::basis::Z, 1.0, 2.0);
    activity = std::string("ring intersections");
    for (size_t count = 0; count < number_of_ops; count++) {
        geometry::intersection hit = obj.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(hit));
    }
}

TEST_F(PerfCounter, IntersectionsTriangle) {
    raytrace::ray r{raytrace::point{0, 0, 2}, -R3::basis::Z};
    raytrace::objects::triangle obj(raytrace::point{-1, 1, 0}, raytrace::point{+1, 0, 0}, raytrace::point{-1, -1, 0});
    activity = std::string("triangle intersections");
    for (size_t count = 0; count < number_of_ops; count++) {
        geometry::intersection hit = obj.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(hit));
    }
}

TEST_F(PerfCounter, IntersectionsCuboid) {
    raytrace::ray r{raytrace::point{0, 0, 2}, -R3::basis::Z};
    raytrace::objects::cuboid obj(R3::origin, 1.0, 1.0, 1.0);
    activity = std::string("cuboid intersections");
    for (size_t count = 0; count < number_of_ops; count++) {
        geometry::intersection hit = obj.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(hit));
    }
}

TEST_F(PerfCounter, IntersectionsCylinder) {
    raytrace::ray r{raytrace::point{2.0, 0, 0}, -R3::basis::X};
    raytrace::objects::cylinder obj(R3::origin, 1.0, 1.0);
    activity = std::string("cylinder intersections");
    for (size_t count = 0; count < number_of_ops; count++) {
        geometry::intersection hit = obj.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(hit));
    }
}

TEST_F(PerfCounter, IntersectionsCone) {
    raytrace::ray r{raytrace::point{1.0, 0, 0.1}, -R3::basis::X};
    raytrace::objects::cone obj(R3::origin, 1.0, 1.0);
    activity = std::string("cone intersections");
    for (size_t count = 0; count < number_of_ops; count++) {
        geometry::intersection hit = obj.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(hit));
    }
}

TEST_F(PerfCounter, IntersectionsSphere) {
    raytrace::ray r{raytrace::point{0, 0, 2}, -R3::basis::Z};
    raytrace::objects::sphere obj(R3::origin, 1.0);
    activity = std::string("sphere intersections");
    for (size_t count = 0; count < number_of_ops; count++) {
        geometry::intersection hit = obj.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(hit));
    }
}

TEST_F(PerfCounter, IntersectionsPyramid) {
    raytrace::ray r{raytrace::point{0, 0, 2}, -R3::basis::Z};
    raytrace::objects::pyramid obj(R3::origin, 1.0);
    activity = std::string("pyramid intersections");
    for (size_t count = 0; count < number_of_ops; count++) {
        geometry::intersection hit = obj.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(hit));
    }
}

TEST_F(PerfCounter, IntersectionsTorus2Hits) {
    raytrace::ray r{raytrace::point{1.0, 0, 2}, -R3::basis::Z};
    raytrace::objects::torus obj(R3::origin, 1.0, 0.2);
    activity = std::string("torus 2h intersections");
    for (size_t count = 0; count < number_of_ops; count++) {
        geometry::intersection hit = obj.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(hit));
    }
}

TEST_F(PerfCounter, IntersectionsTorus4Hits) {
    raytrace::ray r{raytrace::point{2.0, 0, 0}, -R3::basis::X};
    raytrace::objects::torus obj(R3::origin, 1.0, 0.2);
    activity = std::string("torus 4h intersections");
    for (size_t count = 0; count < number_of_ops; count++) {
        geometry::intersection hit = obj.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(hit));
    }
}

TEST_F(PerfCounter, IntersectionsQuadratic) {
    raytrace::ray r{raytrace::point{2.0, 0, 0}, -R3::basis::X};
    // a bowl sitting on the XY plane
    matrix C{{{1.0, 0.0, 0.0, 0.0}, {0.0, 1.0, 0.0, 0.0}, {0.0, 0.0, 0.0, -2.0}, {0.0, 0.0, -2.0, 0.0}}};
    raytrace::objects::quadratic obj(R3::origin, C);
    activity = std::string("quadratic surface intersections");
    for (size_t count = 0; count < number_of_ops; count++) {
        geometry::intersection hit = obj.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(hit));
    }
}
