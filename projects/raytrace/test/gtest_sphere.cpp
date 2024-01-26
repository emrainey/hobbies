#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <iostream>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "basal/gtest_helper.hpp"
#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

using namespace raytrace;

TEST(SphereTest, NormalReflection) {
    raytrace::point C{1, 1, 1};
    raytrace::objects::sphere s0{C, 1.0};
    raytrace::point P{0, 1, 1};
    ASSERT_TRUE(s0.is_surface_point(P));
    vector N = s0.normal(P);
    vector nx = -R3::basis::X;
    ASSERT_VECTOR_EQ(nx, N);
    vector F{{1, 1, 0}};
    // reflect F at P using N inside
    vector G{s0.reflection(F, P)};
    vector H{{-1, 1, 0}};
    ASSERT_VECTOR_EQ(H.normalized(), G);
}

TEST(SphereTest, Mapping) {
    raytrace::objects::sphere s0{raytrace::point{1, 1, 1}, 1.0};

    raytrace::point surface_points[] = {
        raytrace::point{1, 0, 0},
        raytrace::point{-1, 0, 0},
        raytrace::point{0, 1, 0},
        raytrace::point{0, -1, 0},
    };
    const size_t dimof_sp = dimof(surface_points);
    image::point uv_points_truth[] = {
        image::point(0.0, 0.5),
        image::point(0.5, 0.5),
        image::point(0.25, 0.5),
        image::point(0.75, 0.5),
    };
    const size_t dimof_uv = dimof(uv_points_truth);

    static_assert(dimof_sp == dimof_uv, "Must be equal");
    for (size_t i = 0; i < dimof_uv; i++) {
        image::point uv_map = s0.map(surface_points[i]);
        ASSERT_IMAGE_POINT_EQ(uv_points_truth[i], uv_map);
    }
}

TEST(SphereTest, IntersectionsFromRays) {
    // should not be at origin
    raytrace::point C{-3, -3, -3};
    raytrace::objects::sphere S{C, 3};

    // there are several types of intersections
    // No intersection
    // Center
    // Non-Center
    // Tangent

    raytrace::point A{3, -3, -3};
    raytrace::point B{1, -1, -1};
    raytrace::point D{0, -3, -3};  // on the surface
    raytrace::point E{-3, 0, -3};  // on the surface, but tangential
    raytrace::point F{-1, 1, 2};

    // line through middle
    ray Amx(A, -R3::basis::X);
    ray Apx(A, R3::basis::X);
    // off center
    ray Bmx(B, -R3::basis::X);
    ray Bpx(B, R3::basis::X);
    // on the surface
    ray Dmx(D, -R3::basis::X);
    ray Dpx(D, R3::basis::X);
    // tangential
    ray Emx(E, -R3::basis::X);
    ray Epx(E, R3::basis::X);
    // no where near
    ray Fmx(F, -R3::basis::X);
    ray Fpx(F, R3::basis::X);
    // center outwards
    ray Gmx(C, -R3::basis::X);
    ray Gpx(C, R3::basis::X);

    geometry::intersection iSAmx = S.intersect(Amx);
    geometry::intersection iSApx = S.intersect(Apx);
    geometry::intersection iSBmx = S.intersect(Bmx);
    geometry::intersection iSBpx = S.intersect(Bpx);
    geometry::intersection iSDmx = S.intersect(Dmx);
    geometry::intersection iSDpx = S.intersect(Dpx);
    geometry::intersection iSEmx = S.intersect(Emx);
    geometry::intersection iSEpx = S.intersect(Epx);
    geometry::intersection iSFmx = S.intersect(Fmx);
    geometry::intersection iSFpx = S.intersect(Fpx);
    geometry::intersection iSGmx = S.intersect(Gmx);
    geometry::intersection iSGpx = S.intersect(Gpx);

    ASSERT_EQ(geometry::IntersectionType::Point, geometry::get_type(iSAmx));
    ASSERT_POINT_EQ(D, geometry::as_point(iSAmx));

    ASSERT_EQ(geometry::IntersectionType::None, geometry::get_type(iSApx));

    ASSERT_EQ(geometry::IntersectionType::Point, geometry::get_type(iSBmx));
    vector V = geometry::as_point(iSBmx) - B;
    ASSERT_LT(V.magnitude(), 4.0);
    ASSERT_GT(V.magnitude(), 2.0);

    ASSERT_EQ(geometry::IntersectionType::None, geometry::get_type(iSBpx));

    ASSERT_EQ(geometry::IntersectionType::Point, geometry::get_type(iSDmx));
    ASSERT_POINT_EQ(D, geometry::as_point(iSDmx));

    ASSERT_EQ(geometry::IntersectionType::None, geometry::get_type(iSDpx));

    // tangent lines with points on surface with vectors away from the normal
    // don't return hits
    ASSERT_EQ(geometry::IntersectionType::None, geometry::get_type(iSEmx));
    ASSERT_EQ(geometry::IntersectionType::None, geometry::get_type(iSEpx));

    ASSERT_EQ(geometry::IntersectionType::None, geometry::get_type(iSFmx));
    ASSERT_EQ(geometry::IntersectionType::None, geometry::get_type(iSFpx));

    // rays from the center outwards should intersect, so that we can have transparent objects!
    ASSERT_EQ(geometry::IntersectionType::Point, geometry::get_type(iSGmx));
    ASSERT_EQ(geometry::IntersectionType::Point, geometry::get_type(iSGpx));
}

TEST(SphereTest, Refraction) {
    struct {
        precision eta;
        iso::radians entry_exterior_angle;
        iso::radians entry_interior_angle;
        iso::radians exit_interior_angle;
        iso::radians exit_exterior_angle;
    } params[] = {
        {mediums::refractive_index::water, iso::radians{0}, iso::radians{0}, iso::radians{0}, iso::radians{0}},
        {mediums::refractive_index::water, iso::radians{iso::pi / 12}, iso::radians{0.19540396},
         iso::radians{0.12900853}, iso::radians{0.2617994}},
        {mediums::refractive_index::water, iso::radians{iso::pi / 6}, iso::radians{0.38449794},
         iso::radians{0.24539709}, iso::radians{0.52359879}},
    };
    for (auto& param : params) {
        precision eta = param.eta;
        precision entry_y = std::cos(param.entry_exterior_angle.value);
        precision entry_z = std::sin(param.entry_exterior_angle.value);
        raytrace::objects::sphere shape(R3::origin, 1.0);
        raytrace::mediums::transparent med(eta, 0.0, colors::white);
        shape.material(&med);
        raytrace::vector incident{0, -1, 0};
        raytrace::ray shot{raytrace::point{0, 2, entry_z}, incident};
        std::cout << "Shot: " << shot << std::endl;
        //============ Entry
        auto entry_hit = shape.intersect(shot);
        std::cout << "Entry Hit: " << entry_hit << std::endl;
        ASSERT_EQ(geometry::IntersectionType::Point, geometry::get_type(entry_hit));
        raytrace::point entry_surface_point = geometry::as_point(entry_hit);
        std::cout << "Entry Surface point: " << entry_surface_point << std::endl;
        raytrace::point s0{0, entry_y, entry_z};
        ASSERT_POINT_EQ(s0, entry_surface_point);
        raytrace::vector normal = shape.normal(entry_surface_point);
        std::cout << "Entry Normal: " << normal << std::endl;
        iso::radians incident_angle = geometry::angle(-normal, shot.direction());
        ASSERT_NEAR(param.entry_exterior_angle.value, incident_angle.value, basal::epsilon);
        raytrace::ray refracted_ray = shape.refraction(shot, entry_surface_point, 1.0, eta);
        std::cout << "Refracted: " << refracted_ray << std::endl;
        ASSERT_POINT_EQ(entry_surface_point, refracted_ray.location());
        iso::radians refracted_angle = geometry::angle(-normal, refracted_ray.direction());
        ASSERT_NEAR(param.entry_interior_angle.value, refracted_angle.value, basal::epsilon);
        //============ Exit
        auto exit_hit = shape.intersect(refracted_ray);
        std::cout << "Exit Hit: " << exit_hit << std::endl;
        ASSERT_EQ(geometry::IntersectionType::Point, geometry::get_type(exit_hit));
        raytrace::point exit_surface_point = geometry::as_point(exit_hit);
        ASSERT_TRUE(shape.surface(exit_surface_point));
        normal = shape.normal(exit_surface_point);
        std::cout << "Exit Normal: " << normal << std::endl;
        incident_angle = geometry::angle(normal, shot.direction());
        std::cout << "Exit Incident Angle: " << incident_angle.value << std::endl;
        EXPECT_NEAR(param.exit_interior_angle.value, incident_angle.value, basal::epsilon);
        raytrace::ray transmitted_ray = shape.refraction(refracted_ray, exit_surface_point, eta, 1.0);
        std::cout << "Transmitted Ray: " << transmitted_ray << std::endl;
        refracted_angle = geometry::angle(normal, transmitted_ray.direction());
        EXPECT_NEAR(param.exit_exterior_angle.value, refracted_angle.value, basal::epsilon);
        std::cout << "Exit Refracted Angle: " << refracted_angle.value << std::endl;
        std::cout << "===========================" << std::endl;
    }
}
