#include <vector>
#include <iostream>

#include <gtest/gtest.h>
#include <basal/basal.hpp>
#include <raytrace/raytrace.hpp>

#include "linalg/gtest_helper.hpp"
#include "geometry/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

using namespace raytrace;

TEST(SphereTest, NormalReflection) {
    raytrace::point C(1,1,1);
    raytrace::sphere s0(C, 1.0);
    raytrace::point P(0, 1, 1);
    ASSERT_TRUE(s0.surface(P));
    vector N = s0.normal(P);
    vector nx = -R3::basis::X;
    ASSERT_VECTOR_EQ(nx, N);
    vector F{{1, 1, 0}};
    // reflect F at P using N inside
    vector G(s0.reflection(F, P));
    vector H{{-1, 1, 0}};
    ASSERT_VECTOR_EQ(H.normalized(), G);
}

TEST(SphereTest, Mapping) {
    raytrace::sphere s0(raytrace::point(1,1,1), 1.0);

    raytrace::point surface_points[] = {
        raytrace::point( 1, 0, 0),
        raytrace::point(-1, 0, 0),
        raytrace::point( 0, 1, 0),
        raytrace::point( 0,-1, 0),
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
    raytrace::point C(-3, -3, -3);
    raytrace::sphere S(C, 3);

    // there are several types of intersections
    // No intersection
    // Center
    // Non-Center
    // Tangent

    raytrace::point A(3, -3, -3);
    raytrace::point B(1, -1, -1);
    raytrace::point D(0, -3, -3); // on the surface
    raytrace::point E(-3, 0, -3); // on the surface, but tangential
    raytrace::point F(-1, 1,  2);

    // line through middle
    ray Amx(A, -R3::basis::X);
    ray Apx(A,  R3::basis::X);
    // off center
    ray Bmx(B, -R3::basis::X);
    ray Bpx(B,  R3::basis::X);
    // on the surface
    ray Dmx(D, -R3::basis::X);
    ray Dpx(D,  R3::basis::X);
    // tangential
    ray Emx(E, -R3::basis::X);
    ray Epx(E,  R3::basis::X);
    // no where near
    ray Fmx(F, -R3::basis::X);
    ray Fpx(F,  R3::basis::X);
    // center outwards
    ray Gmx(C, -R3::basis::X);
    ray Gpx(C,  R3::basis::X);

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

    // FIXME rays from the center outwards should not intersect
    ASSERT_EQ(geometry::IntersectionType::Point, geometry::get_type(iSGmx));
    ASSERT_EQ(geometry::IntersectionType::Point, geometry::get_type(iSGpx));
}