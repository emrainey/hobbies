#include "basal/gtest_helper.hpp"

#include <basal/basal.hpp>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

using namespace raytrace;

TEST(LensTest, Raycast) {
    precision const lens_diameter = 10.0;
    iso::radians const half_angle = iso::radians(iso::pi / 4);
    ConvexLensData info{lens_diameter, half_angle};
    printf("Diameter: %f, Angle: %f, Radius: %f, Separation: %f, Bulge: %f\n",
            lens_diameter, half_angle.value, info.radius, info.separation, info.bulge);
    raytrace::objects::sphere x0{raytrace::point{info.separation, 0, 0}, info.radius};
    raytrace::objects::sphere x1{raytrace::point{-info.separation, 0, 0}, info.radius};
    raytrace::objects::overlap convex_lens{x0, x1, objects::overlap::type::inclusive};
    raytrace::mediums::transparent glass{mediums::refractive_index::glass, 0.22, colors::dark_gray};
    convex_lens.material(&glass);
    EXPECT_POINT_EQ(raytrace::point(0, 0, 0), convex_lens.position());
    convex_lens.position(raytrace::point{0, 0, 5});
    // now we'll cast a bunch of rays which intersect the lens and check the normals for the correct orientation.
    {
        // center shot
        raytrace::ray r{raytrace::point{-20, 0, 5}, raytrace::vector{1, 0, 0}};
        auto hit = convex_lens.intersect(r);
        EXPECT_EQ(IntersectionType::Point, get_type(hit.intersect));
        EXPECT_POINT_EQ(raytrace::point(-info.bulge, 0, 5), as_point(hit.intersect));
        EXPECT_VECTOR_EQ(raytrace::vector({-1, 0, 0}), hit.normal);
    }
    // off center shot
    {
        raytrace::ray r{raytrace::point{-20, 0, 6}, raytrace::vector{1, 0, 0}};
        auto hit = convex_lens.intersect(r);
        EXPECT_EQ(IntersectionType::Point, get_type(hit.intersect));
        EXPECT_POINT_EQ(raytrace::point(-2, 0, 6), as_point(hit.intersect));
        EXPECT_LT(-1.0_p, hit.normal[0]);
        EXPECT_LT(0.0_p, hit.normal[2]);

        raytrace::ray refraction = convex_lens.refraction(r, hit.normal, as_point(hit.intersect), mediums::earth_atmosphere.refractive_index(as_point(hit.intersect)),
            glass.refractive_index(as_point(hit.intersect)));

        // EXPECT_VECTOR_EQ(raytrace::vector({-1, 0, 0}), refraction.direction());
        EXPECT_GT(1.0_p, refraction.direction()[0]);
        EXPECT_LT(-0.1_p, refraction.direction()[2]);
    }
    // further off center shot
    {
        raytrace::ray r{raytrace::point{-20, 0, 7}, raytrace::vector{1, 0, 0}};
        auto hit = convex_lens.intersect(r);
        EXPECT_EQ(IntersectionType::Point, get_type(hit.intersect));
        // EXPECT_POINT_EQ(raytrace::point(-2, 0, 7), as_point(hit.intersect));
        EXPECT_LT(-info.bulge, as_point(hit.intersect).x);
        EXPECT_LT(-1.0_p, hit.normal[0]);
        EXPECT_LT(0.0_p, hit.normal[2]);

        raytrace::ray refraction = convex_lens.refraction(r, hit.normal, as_point(hit.intersect), mediums::earth_atmosphere.refractive_index(as_point(hit.intersect)),
            glass.refractive_index(as_point(hit.intersect)));

        // EXPECT_VECTOR_EQ(raytrace::vector({-1, 0, 0}), refraction.direction());
        EXPECT_GT(1.0_p, refraction.direction()[0]);
        EXPECT_LT(-0.1_p, refraction.direction()[2]);
    }
}

