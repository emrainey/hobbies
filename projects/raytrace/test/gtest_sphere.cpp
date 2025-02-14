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
    size_t const dimof_sp = dimof(surface_points);
    image::point uv_points_truth[] = {
        image::point(0.0, 0.5),
        image::point(0.5, 0.5),
        image::point(0.25, 0.5),
        image::point(0.75, 0.5),
    };
    size_t const dimof_uv = dimof(uv_points_truth);

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

    geometry::intersection iSAmx = S.intersect(Amx).intersect;
    geometry::intersection iSApx = S.intersect(Apx).intersect;
    geometry::intersection iSBmx = S.intersect(Bmx).intersect;
    geometry::intersection iSBpx = S.intersect(Bpx).intersect;
    geometry::intersection iSDmx = S.intersect(Dmx).intersect;
    geometry::intersection iSDpx = S.intersect(Dpx).intersect;
    geometry::intersection iSEmx = S.intersect(Emx).intersect;
    geometry::intersection iSEpx = S.intersect(Epx).intersect;
    geometry::intersection iSFmx = S.intersect(Fmx).intersect;
    geometry::intersection iSFpx = S.intersect(Fpx).intersect;
    geometry::intersection iSGmx = S.intersect(Gmx).intersect;
    geometry::intersection iSGpx = S.intersect(Gpx).intersect;

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
        auto entry_hit = shape.intersect(shot).intersect;
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
        auto exit_hit = shape.intersect(refracted_ray).intersect;
        std::cout << "Exit Hit: " << exit_hit << std::endl;
        ASSERT_EQ(geometry::IntersectionType::Point, geometry::get_type(exit_hit));
        raytrace::point exit_surface_point = geometry::as_point(exit_hit);
        ASSERT_TRUE(shape.on_surface(exit_surface_point));
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

TEST(SphereTest, ScaledSphere) {
    // center the sphere on 1, 1, 1
    raytrace::point C{1, 1, 1};
    // 2 units in radius
    raytrace::objects::sphere s0{C, 2.0_p};
    // scale the sphere by these factors in x, y, z
    s0.scale(0.5_p, 1.0_p, 2.0_p);
    ASSERT_POINT_EQ(C, s0.position());

    raytrace::point top{1, 1, 2 * 2 + 1};
    raytrace::point bottom{1, 1, -2 * 2 + 1};
    raytrace::point left{-2 * 0.5 + 1, 1, 1};
    raytrace::point right{2 * 0.5 + 1, 1, 1};
    raytrace::point front{1, 2 * 1 + 1, 1};
    raytrace::point back{1, -2 * 1 + 1, 1};

    // the top of the scaled sphere should be here (+z)
    ASSERT_TRUE(s0.is_surface_point(top));
    // the bottom of the scaled sphere should be here (-z)
    ASSERT_TRUE(s0.is_surface_point(bottom));
    // the left of the scaled sphere should be here (-x)
    ASSERT_TRUE(s0.is_surface_point(left));
    // the right of the scaled sphere should be here (+x)
    ASSERT_TRUE(s0.is_surface_point(right));
    // the front of the scaled sphere should be here (+y)
    ASSERT_TRUE(s0.is_surface_point(front));
    // the back of the scaled sphere should be here (-y)
    ASSERT_TRUE(s0.is_surface_point(back));

    // determine the intersection of a ray from above to the where the top point should be
    raytrace::ray top_down{raytrace::point{1, 1, 6}, -R3::basis::Z}; // from above
    geometry::intersection iTD = s0.intersect(top_down).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, geometry::get_type(iTD));
    ASSERT_POINT_EQ(top, geometry::as_point(iTD));

    // determine the intersection of a ray from below to the where the bottom point should be
    raytrace::ray bottom_up{raytrace::point{1, 1, -6}, R3::basis::Z}; // from below
    geometry::intersection iBU = s0.intersect(bottom_up).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, geometry::get_type(iBU));
    ASSERT_POINT_EQ(bottom, geometry::as_point(iBU));

    // determine the intersection of a ray from the left to the where the left point should be
    raytrace::ray left_right{raytrace::point{-6, 1, 1}, R3::basis::X}; // from left
    geometry::intersection iLR = s0.intersect(left_right).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, geometry::get_type(iLR));
    ASSERT_POINT_EQ(left, geometry::as_point(iLR));

    // determine the intersection of a ray from the right to the where the right point should be
    raytrace::ray right_left{raytrace::point{6, 1, 1}, -R3::basis::X}; // from right
    geometry::intersection iRL = s0.intersect(right_left).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, geometry::get_type(iRL));
    ASSERT_POINT_EQ(right, geometry::as_point(iRL));

    // determine the intersection of a ray from the front to the where the front point should be
    raytrace::ray front_back{raytrace::point{1, 6, 1}, -R3::basis::Y}; // from front
    geometry::intersection iFB = s0.intersect(front_back).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, geometry::get_type(iFB));
    ASSERT_POINT_EQ(front, geometry::as_point(iFB));

    // determine the intersection of a ray from the back to the where the back point should be
    raytrace::ray back_front{raytrace::point{1, -6, 1}, R3::basis::Y}; // from back
    geometry::intersection iBF = s0.intersect(back_front).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, geometry::get_type(iBF));
    ASSERT_POINT_EQ(back, geometry::as_point(iBF));
}