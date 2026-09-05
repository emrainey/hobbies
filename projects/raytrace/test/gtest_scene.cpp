#include "basal/gtest_helper.hpp"

#include <basal/basal.hpp>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"
#include "raytrace/raytrace.hpp"

TEST(SceneTest, ObjectIntersections) {
    using namespace raytrace;
    using namespace raytrace::objects;

    // define a single object
    raytrace::objects::sphere s0{raytrace::point{0, 0, 0}, 2};
    // define a ray
    ray r0{raytrace::point{0, 0, 5}, vector{{0, 0, -1}}};
    // define the object list
    scene scene;
    scene.add_object(&s0);
    // find the intersections (this should pass through the center)
    objects::hits list = scene.find_intersections(r0);
    // that is a point
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(list[0].intersect));
    // that point is at 0,0,2
    raytrace::point truth(0, 0, 2);
    ASSERT_POINT_EQ(truth, as_point(list[0].intersect));
}

TEST(SceneTest, NearestObjectIntersections) {
    using namespace raytrace;
    using namespace raytrace::objects;

    // define several objects
    raytrace::objects::sphere s0{raytrace::point{0, 0, 0}, 2};
    raytrace::objects::sphere s1{raytrace::point{0, 3, 0}, 2};
    // define a ray
    ray r0{raytrace::point{0, -3, 0}, vector{{0, 1, 0}}};
    // define the object list
    scene scene;
    scene.add_object(&s0);
    scene.add_object(&s1);
    // find the intersections
    objects::hits list = scene.find_intersections(r0);
    // there should both be points (non-facing normals removed already)
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(list[0].intersect));
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(list[1].intersect));
    // find the nearest object to the ray
    objects::hit nearest = scene.nearest_object(r0, list);
    ASSERT_PRECISION_EQ(1, nearest.distance);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(nearest.intersect));
    ASSERT_EQ(&s0, nearest.object);
}

TEST(SceneTest, LowResSpheres) {
    using namespace raytrace;
    using namespace raytrace::objects;

    // define an object
    raytrace::objects::sphere s0{raytrace::point{4, 0, 0}, 0.50_p};
    raytrace::objects::sphere s1{raytrace::point{4, -2, 0}, 0.75_p};
    raytrace::objects::sphere s2{raytrace::point{4, 2, 0}, 0.25_p};
    raytrace::mediums::checkerboard c0{6.0_p, colors::red, colors::green};
    raytrace::lights::beam sunlight{raytrace::vector{-1, 0, -1}, raytrace::colors::white,
                                    lights::intensities::full * 3.0_p};
    s0.material(&c0);
    s1.material(&raytrace::mediums::metals::bronze);
    s2.material(&raytrace::mediums::dull);

    iso::degrees fov(65);
    // tiny image, simple camera placement
    scene scene;
    scene.set_ambient_light(color{1.0_p, 1.0_p, 1.0_p, 0.75_p});
    raytrace::camera view(240, 320, fov);
    raytrace::point look_from(-1, 0, 0);
    raytrace::point look_at(4, 0, 0);
    view.move_to(look_from, look_at);

    ASSERT_POINT_EQ(look_from, view.position());
    ASSERT_VECTOR_EQ(R3::basis::X, view.forward().direction().normalized());
    ASSERT_VECTOR_EQ(R3::basis::Z, view.up().direction().normalized());

    scene.add_object(&s0);
    scene.add_object(&s1);
    scene.add_object(&s2);
    scene.add_light(&sunlight);
    scene.render(view, "low_res_sphere.ppm");
}

TEST(SceneTest, GlowingMaterialEmitsLight) {
    using namespace raytrace;
    using namespace raytrace::objects;
    using namespace raytrace::mediums;

    // Create a sphere with emissive/glowing material
    raytrace::objects::sphere glowing_sphere{raytrace::point{0, 0, 0}, 1.0_p};

    // Create an emissive material (assuming there's an emissive material type)
    // This test assumes there's a way to create glowing/emissive materials
    auto emissive_material = raytrace::mediums::glowy{raytrace::colors::white, raytrace::colors::yellow};
    glowing_sphere.material(&emissive_material);

    // Create scene
    raytrace::scene test_scene;
    test_scene.add_object(&glowing_sphere);

    // Create a ray that intersects the glowing sphere
    raytrace::ray test_ray{raytrace::point{0, 0, 5}, raytrace::vector{0, 0, -1}};

    // Find intersection with the glowing object
    raytrace::objects::hits intersections = test_scene.find_intersections(test_ray);
    ASSERT_GT(intersections.size(), 0);

    // Get the nearest intersection
    objects::hit nearest = test_scene.nearest_object(test_ray, intersections);

    // Get the emitted light color from the intersection point
    raytrace::color emitted_light
        = test_scene.emissive_light(0.75_p, emissive_material, raytrace::as_point(nearest.intersect));

    std::cout << "Emitted Light Color: " << emitted_light << std::endl;

    // Verify that light is actually emitted (non-black color)
    ASSERT_PRECISION_EQ(emitted_light.red(), 0.75_p);
    ASSERT_PRECISION_EQ(emitted_light.green(), 0.75_p);
    ASSERT_PRECISION_EQ(emitted_light.blue(), 0.0_p);
}

TEST(SceneTest, NonEmissiveMaterialDoesNotEmitLight) {
    using namespace raytrace;
    using namespace raytrace::objects;
    using namespace raytrace::mediums;

    // Create a sphere with non-emissive material
    raytrace::objects::sphere non_glowing_sphere{raytrace::point{0, 0, 0}, 1.0_p};
    non_glowing_sphere.material(&mediums::dull);

    // Create scene
    raytrace::scene test_scene;
    test_scene.add_object(&non_glowing_sphere);

    // Create a ray that intersects the sphere
    raytrace::ray test_ray{raytrace::point{0, 0, 5}, raytrace::vector{0, 0, -1}};

    // Find intersection
    raytrace::objects::hits intersections = test_scene.find_intersections(test_ray);
    ASSERT_GT(intersections.size(), 0);

    // Get the nearest intersection
    raytrace::objects::hit nearest = test_scene.nearest_object(test_ray, intersections);

    // Get the emitted light (should be black/none)
    raytrace::color emitted_light
        = test_scene.emissive_light(0.75_p, mediums::metals::stainless, as_point(nearest.intersect));

    // Verify that no light is emitted (black color)
    ASSERT_PRECISION_EQ(0.0_p, emitted_light.red());
    ASSERT_PRECISION_EQ(0.0_p, emitted_light.green());
    ASSERT_PRECISION_EQ(0.0_p, emitted_light.blue());
}

TEST(SceneTest, MultipleEmissiveObjectsContributeLight) {
    using namespace raytrace;
    using namespace raytrace::objects;
    using namespace raytrace::mediums;

    // Create multiple glowing spheres with different colors
    raytrace::objects::sphere red_glowing{raytrace::point{-2, 0, 0}, 0.5_p};
    raytrace::objects::sphere blue_glowing{raytrace::point{2, 0, 0}, 0.5_p};

    auto red_emissive = raytrace::mediums::glowy{raytrace::colors::red, raytrace::colors::red};
    auto blue_emissive = raytrace::mediums::glowy{raytrace::colors::blue, raytrace::colors::blue};
    red_glowing.material(&red_emissive);
    blue_glowing.material(&blue_emissive);

    // Create scene
    raytrace::scene test_scene;
    test_scene.add_object(&red_glowing);
    test_scene.add_object(&blue_glowing);

    // Test ray towards red sphere
    raytrace::ray red_ray{raytrace::point{-2, 0, 5}, raytrace::vector{0, 0, -1}};
    raytrace::objects::hits red_intersections = test_scene.find_intersections(red_ray);
    ASSERT_GT(red_intersections.size(), 0);

    raytrace::objects::hit red_nearest = test_scene.nearest_object(red_ray, red_intersections);
    raytrace::color red_emitted
        = test_scene.emissive_light(0.625_p, red_emissive, raytrace::as_point(red_nearest.intersect));

    // Verify red light emission
    ASSERT_PRECISION_EQ(0.625_p, red_emitted.red());
    ASSERT_PRECISION_EQ(0.0_p, red_emitted.green());
    ASSERT_PRECISION_EQ(0.0_p, red_emitted.blue());

    // Test ray towards blue sphere
    raytrace::ray blue_ray{raytrace::point{2, 0, 5}, raytrace::vector{0, 0, -1}};
    raytrace::objects::hits blue_intersections = test_scene.find_intersections(blue_ray);
    ASSERT_GT(blue_intersections.size(), 0);

    raytrace::objects::hit blue_nearest = test_scene.nearest_object(blue_ray, blue_intersections);
    raytrace::color blue_emitted
        = test_scene.emissive_light(0.625_p, blue_emissive, raytrace::as_point(blue_nearest.intersect));

    // Verify blue light emission
    ASSERT_PRECISION_EQ(0.0_p, blue_emitted.red());
    ASSERT_PRECISION_EQ(0.0_p, blue_emitted.green());
    ASSERT_PRECISION_EQ(0.625_p, blue_emitted.blue());
}

TEST(SceneTest, HasEmissiveObjectsFlag) {
    using namespace raytrace;
    using namespace raytrace::objects;
    using namespace raytrace::mediums;

    raytrace::scene test_scene;
    ASSERT_FALSE(test_scene.has_emissive_objects());

    raytrace::objects::sphere s{raytrace::point{0, 0, 0}, 1.0_p};
    ASSERT_FALSE(test_scene.has_emissive_objects());

    test_scene.add_object(&s);
    ASSERT_FALSE(test_scene.has_emissive_objects());

    raytrace::objects::sphere glow_sphere{raytrace::point{2, 0, 0}, 0.5_p};
    auto glow_mat = raytrace::mediums::glowy{raytrace::colors::black, raytrace::colors::yellow};
    glow_sphere.material(&glow_mat);
    test_scene.add_object(&glow_sphere);
    ASSERT_TRUE(test_scene.has_emissive_objects());
}

TEST(SceneTest, EmissiveIlluminationFromObject) {
    using namespace raytrace;
    using namespace raytrace::objects;
    using namespace raytrace::mediums;
    using namespace geometry::operators;

    // Create an emissive sphere (red glowing) placed to the right
    raytrace::objects::sphere emissive_sphere{raytrace::point{3, 0, 0}, 0.5_p};
    auto emissive_mat = raytrace::mediums::glowy{raytrace::colors::black, raytrace::colors::red};
    emissive_sphere.material(&emissive_mat);

    // Create a dull sphere at the origin (the surface we'll test illumination on)
    raytrace::objects::sphere nearby_sphere{raytrace::point{0, 0, 0}, 0.5_p};
    nearby_sphere.material(&mediums::dull);

    // Create scene
    raytrace::scene test_scene;
    test_scene.add_object(&emissive_sphere);
    test_scene.add_object(&nearby_sphere);
    ASSERT_TRUE(test_scene.has_emissive_objects());

    // The rightmost point of the dull sphere at (0,0,0) radius 0.5 is (0.5, 0, 0)
    // The normal at that point faces right: (1, 0, 0)
    // The emissive sphere center is at (3, 0, 0), so direction to light = (2.5, 0, 0)
    // Cast a ray from the left side to find the entry point on the dull sphere
    raytrace::ray entry_ray{raytrace::point{-5, 0, 0}, raytrace::vector{1, 0, 0}};
    raytrace::objects::hits hits = test_scene.find_intersections(entry_ray);
    raytrace::objects::hit nearest = test_scene.nearest_object(entry_ray, hits);
    ASSERT_EQ(&nearby_sphere, nearest.object);

    // The entry point is (-0.5, 0, 0) with normal (-1, 0, 0) — facing LEFT
    raytrace::point entry_world_point = raytrace::as_point(nearest.intersect);
    raytrace::vector entry_normal = nearest.normal.normalized();
    // Direction from entry point to emissive sphere: (3.5, 0, 0)
    // Dot with normal (-1, 0, 0) = -3.5 < 0 → this point faces away from emissive

    // The exit point is (0.5, 0, 0) with normal (1, 0, 0) — facing RIGHT
    // We need to use the exit point which faces the emissive sphere.
    // Assert we got the entry point
    ASSERT_PRECISION_EQ(-0.5_p, entry_world_point.x());

    // Compute emissive illumination from the entry point (should face away, but
    // the emissive sphere is still visible, just the normal dot product limits it)
    color illumination = test_scene.emissive_illumination(entry_world_point, entry_normal);
    // The emissive sphere IS along the line of sight from entry point to (3,0,0),
    // but the normal faces away, so Lambert's law gives 0 contribution
    ASSERT_PRECISION_EQ(0.0_p, illumination.red());

    // Now construct the exit point directly: (0.5, 0, 0) with normal (1, 0, 0)
    raytrace::point exit_world_point{0.5_p, 0.0_p, 0.0_p};
    raytrace::vector exit_normal{1.0_p, 0.0_p, 0.0_p};
    // For a normal facing right, the emissive sphere contributes
    illumination = test_scene.emissive_illumination(exit_world_point, exit_normal);
    ASSERT_PRECISION_NE(0.0_p, illumination.red());
    ASSERT_PRECISION_EQ(0.0_p, illumination.green());
    ASSERT_PRECISION_EQ(0.0_p, illumination.blue());
}

TEST(SceneTest, NoEmissiveIlluminationWhenBlocked) {
    using namespace raytrace;
    using namespace raytrace::objects;
    using namespace raytrace::mediums;
    using namespace geometry::operators;

    // Create an emissive sphere at (4, 0, 0)
    raytrace::objects::sphere emissive_sphere{raytrace::point{4, 0, 0}, 0.5_p};
    auto emissive_mat = raytrace::mediums::glowy{raytrace::colors::black, raytrace::colors::red};
    emissive_sphere.material(&emissive_mat);

    // Create a blocking sphere at (2, 0, 0) between the surface point and the emissive
    raytrace::objects::sphere blocker_sphere{raytrace::point{2, 0, 0}, 0.5_p};
    blocker_sphere.material(&mediums::dull);

    raytrace::scene test_scene;
    test_scene.add_object(&emissive_sphere);
    test_scene.add_object(&blocker_sphere);
    ASSERT_TRUE(test_scene.has_emissive_objects());

    // Surface point at (1, 0, 0), normal facing right (1, 0, 0)
    // Direction to emissive: (3, 0, 0), dot with normal = 3 > 0
    // But the blocker at (2, 0, 0) radius 0.5 blocks the ray toward (4, 0, 0)
    raytrace::point world_surface_point{1.0_p, 0.0_p, 0.0_p};
    raytrace::vector world_surface_normal{1.0_p, 0.0_p, 0.0_p};

    color illumination = test_scene.emissive_illumination(world_surface_point, world_surface_normal);
    // The blocker sphere is in the way, so no illumination
    ASSERT_PRECISION_EQ(0.0_p, illumination.red());
    ASSERT_PRECISION_EQ(0.0_p, illumination.green());
    ASSERT_PRECISION_EQ(0.0_p, illumination.blue());
}

TEST(SceneTest, TransparentShadow) {
    using namespace raytrace;
    using namespace raytrace::lights;
    using namespace raytrace::mediums;
    using namespace raytrace::colors;
    using namespace geometry::operators;

    // Perfectly transparent glass (zero extinction)
    transparent glass(refractive_index::glass, colors::black, colors::white);

    // Matte white surface
    plain white_surface(colors::white, 0.0_p, colors::white, mediums::smoothness::none, roughness::medium);

    // Beam light shining in -Z direction (incident rays go +Z toward the light source)
    beam sunlight(raytrace::vector{0, 0, -1}, colors::white, 1.0_p);

    // Transparent glass sphere at z=5, radius 2 — between the plane and the light
    raytrace::objects::sphere glass_sphere(raytrace::point{0, 0, 5}, 2.0_p);
    glass_sphere.material(&glass);

    // Ground plane at z=0
    raytrace::objects::plane ground;
    ground.material(&white_surface);

    scene test_scene;
    test_scene.add_light(&sunlight);
    test_scene.add_object(&glass_sphere);
    test_scene.add_object(&ground);
    test_scene.set_ambient_light(colors::black);

    // Camera ray from (0, 0, -10) in +Z direction, hits the plane at (0, 0, 0)
    ray camera_ray(raytrace::point{0, 0, -10}, vector{{0, 0, 1}});
    color result = test_scene.trace(camera_ray, mediums::vacuum, 3);

    // The plane should receive light through the transparent sphere
    EXPECT_PRECISION_EQ(0.5_p, result.red());
    EXPECT_PRECISION_EQ(0.5_p, result.green());
    EXPECT_PRECISION_EQ(0.5_p, result.blue());
}

TEST(SceneTest, OpaqueBlockedShadow) {
    using namespace raytrace;
    using namespace raytrace::lights;
    using namespace raytrace::mediums;
    using namespace raytrace::colors;
    using namespace geometry::operators;

    // Opaque material for the blocker
    plain dull_surface(colors::white, 0.0_p, colors::white, mediums::smoothness::none, roughness::medium);

    // Beam light shining in -Z direction
    beam sunlight(raytrace::vector{0, 0, -1}, colors::white, lights::intensities::full);

    // Opaque sphere at z=5, radius 2 — between the plane and the light
    raytrace::objects::sphere blocker(raytrace::point{0, 0, 5}, 2.0_p);
    blocker.material(&dull_surface);

    // Ground plane at z=0
    raytrace::objects::plane ground;
    ground.material(&dull_surface);

    scene test_scene;
    test_scene.add_light(&sunlight);
    test_scene.add_object(&blocker);
    test_scene.add_object(&ground);
    test_scene.set_ambient_light(colors::black);

    // Camera ray from (0, 0, -10) in +Z direction, hits the plane at (0, 0, 0)
    ray camera_ray(raytrace::point{0, 0, -10}, vector{{0, 0, 1}});
    color result = test_scene.trace(camera_ray, mediums::vacuum, 3);

    // The plane should be in shadow — all channels black
    ASSERT_PRECISION_EQ(0.0_p, result.red());
    ASSERT_PRECISION_EQ(0.0_p, result.green());
    ASSERT_PRECISION_EQ(0.0_p, result.blue());
}

TEST(SceneTest, TransparentCuboidShadow) {
    using namespace raytrace;
    using namespace raytrace::lights;
    using namespace raytrace::mediums;
    using namespace raytrace::colors;
    using namespace geometry::operators;

    // Perfectly transparent glass (zero extinction)
    transparent glass(refractive_index::glass, colors::black, colors::white);

    // Matte white surface
    plain white_surface(colors::white, 0.0_p, colors::white, mediums::smoothness::none, roughness::medium);

    // Beam light shining in -Z direction (incident rays go +Z toward the light source)
    beam sunlight(raytrace::vector{0, 0, -1}, colors::white, 1.0_p);

    // Transparent glass cuboid at z=5 — between the plane and the light
    raytrace::objects::cuboid glass_cuboid(raytrace::point{0, 0, 5}, 2, 2, 2);
    glass_cuboid.material(&glass);

    // Ground plane at z=0
    raytrace::objects::plane ground;
    ground.material(&white_surface);

    scene test_scene;
    test_scene.add_light(&sunlight);
    test_scene.add_object(&glass_cuboid);
    test_scene.add_object(&ground);
    test_scene.set_ambient_light(colors::black);

    // Camera ray from (0, 0, -10) in +Z direction, hits the plane at (0, 0, 0)
    ray camera_ray(raytrace::point{0, 0, -10}, vector{{0, 0, 1}});
    color result = test_scene.trace(camera_ray, mediums::vacuum, 3);

    // The plane should receive light through the transparent cuboid (same as sphere)
    EXPECT_PRECISION_EQ(0.5_p, result.red());
    EXPECT_PRECISION_EQ(0.5_p, result.green());
    EXPECT_PRECISION_EQ(0.5_p, result.blue());
}

TEST(SceneTest, TransparentCuboidVisual) {
    using namespace raytrace;
    using namespace raytrace::lights;
    using namespace raytrace::mediums;
    using namespace raytrace::colors;
    using namespace geometry::operators;

    // Perfectly transparent glass (zero extinction), white diffuse
    transparent glass(refractive_index::glass, colors::black, colors::white);

    // Matte white surface for the ground
    plain white_surface(colors::white, 0.0_p, colors::white, mediums::smoothness::none, roughness::medium);

    // Beam light shining in +Z direction so the ground behind the cuboid is lit
    beam sunlight(raytrace::vector{0, 0, 1}, colors::white, lights::intensities::full);

    // Transparent glass cuboid centered at (0, 0, -5), spans z=[-7, -3]
    raytrace::objects::cuboid glass_cuboid(raytrace::point{0, 0, -5}, 4, 4, 4);
    glass_cuboid.material(&glass);

    // Ground plane at z=0 (behind the cuboid)
    raytrace::objects::plane ground;
    ground.material(&white_surface);

    scene test_scene;
    test_scene.add_light(&sunlight);
    test_scene.add_object(&glass_cuboid);
    test_scene.add_object(&ground);
    test_scene.set_ambient_light(colors::black);

    // Camera ray from (0, 0, -15) in +Z direction
    // Hits cuboid at z=-7 (entry), exits at z=-3, then hits plane at z=0
    ray camera_ray(raytrace::point{0, 0, -15}, vector{{0, 0, 1}});
    color result = test_scene.trace(camera_ray, mediums::vacuum, 3);

    // The cuboid should transmit most light — the result should be non-black
    EXPECT_PRECISION_NE(0.0_p, result.red());
    EXPECT_PRECISION_NE(0.0_p, result.green());
    EXPECT_PRECISION_NE(0.0_p, result.blue());
}

TEST(SceneTest, TransparentCuboidRotated) {
    using namespace raytrace;
    using namespace raytrace::lights;
    using namespace raytrace::mediums;
    using namespace raytrace::colors;
    using namespace geometry::operators;

    transparent glass(refractive_index::glass, colors::black, colors::white);
    plain white_surface(colors::white, 0.0_p, colors::white, mediums::smoothness::none, roughness::medium);
    beam sunlight(raytrace::vector{0, 0, 1}, colors::white, lights::intensities::full);

    // Cuboid rotated 35° around Z (same as cornell box), centered at (0, 0, -5)
    raytrace::objects::cuboid glass_cuboid(raytrace::point{0, 0, -5}, 4, 4, 4);
    glass_cuboid.rotation(iso::degrees(0), iso::degrees(0), iso::degrees(35));
    glass_cuboid.material(&glass);

    raytrace::objects::plane ground;
    ground.material(&white_surface);

    scene test_scene;
    test_scene.add_light(&sunlight);
    test_scene.add_object(&glass_cuboid);
    test_scene.add_object(&ground);
    test_scene.set_ambient_light(colors::black);

    // Camera ray from (0, 0, -15) in +Z direction
    // Hits cuboid, passes through, then hits plane at z=0
    ray camera_ray(raytrace::point{0, 0, -15}, vector{{0, 0, 1}});
    color result = test_scene.trace(camera_ray, mediums::vacuum, 3);

    // Should still be non-black through the rotated cuboid
    EXPECT_PRECISION_NE(0.0_p, result.red());
    EXPECT_PRECISION_NE(0.0_p, result.green());
    EXPECT_PRECISION_NE(0.0_p, result.blue());
}

TEST(SceneTest, TransparentCuboidCornellConfig) {
    using namespace raytrace;
    using namespace raytrace::lights;
    using namespace raytrace::mediums;
    using namespace raytrace::colors;
    using namespace geometry::operators;

    transparent glass(refractive_index::glass, colors::black, colors::white);
    plain white_surface(colors::white, 0.0_p, colors::white, mediums::smoothness::none, roughness::medium);
    bulb light{raytrace::point{-50, -50, 120}, 10.0_p, colors::white, lights::intensities::full, 1};

    // Match cornell box: cuboid at (0, -30, 60), size 20×20×60, rotated 35° Z
    raytrace::objects::cuboid glass_cuboid(raytrace::point{0, -30.0_p, 60}, 10, 10, 30);
    glass_cuboid.rotation(iso::degrees(0), iso::degrees(0), iso::degrees(35));
    glass_cuboid.material(&glass);

    // Ground plane at z=0 (below the cuboid)
    raytrace::objects::plane ground;
    ground.material(&white_surface);

    scene test_scene;
    test_scene.add_light(&light);
    test_scene.add_object(&glass_cuboid);
    test_scene.add_object(&ground);
    test_scene.set_ambient_light(colors::black);

    // Camera ray aimed at the cuboid center (0, -30, 60)
    raytrace::point cuboid_center{0, -30, 60};
    raytrace::vector dir = (cuboid_center - raytrace::point{-220, 0, 80}).normalized();
    ray camera_ray(raytrace::point{-220, 0, 80}, dir);
    color result = test_scene.trace(camera_ray, mediums::vacuum, 3);

    // The cuboid and plane should render — result should be non-black
    EXPECT_PRECISION_NE(0.0_p, result.red());
    EXPECT_PRECISION_NE(0.0_p, result.green());
    EXPECT_PRECISION_NE(0.0_p, result.blue());
}

TEST(SceneTest, CornellBoxFullRay) {
    using namespace raytrace;
    using namespace raytrace::lights;
    using namespace raytrace::mediums;
    using namespace raytrace::colors;
    using namespace geometry::operators;

    // Replicate the full cornell box scene
    mediums::plain plain_white(colors::white, mediums::ambient::none, colors::white, mediums::smoothness::none,
                               mediums::roughness::tight);
    mediums::plain plain_red(colors::red, mediums::ambient::none, colors::red, mediums::smoothness::none,
                             mediums::roughness::tight);
    mediums::plain plain_blue(colors::blue, mediums::ambient::none, colors::blue, mediums::smoothness::none,
                              mediums::roughness::tight);
    mediums::transparent glass(mediums::refractive_index::glass, colors::black, colors::white);

    // walls
    objects::plane wall0{raytrace::point{0, 80, 80}, R3::roll(iso::radians{iso::pi / 2})};    // left (blue)
    objects::plane wall1{raytrace::point{0, -80, 80}, R3::roll(iso::radians{-iso::pi / 2})};  // right (red)
    objects::plane wall2{raytrace::point{80, 0, 80}, R3::pitch(iso::radians{-iso::pi / 2})};  // back
    objects::plane wall3{};                                                                   // floor
    objects::plane wall4{raytrace::point{0, 0, 160}, R3::pitch(iso::radians{iso::pi})};       // ceiling
    objects::cuboid box{raytrace::point{0, -30, 60}, 20, 20, 60};                             // glass cuboid
    objects::sphere ball{raytrace::point{0, 30, 30}, 30};                                     // stainless ball
    objects::cuboid marble_cube{raytrace::point{0, -25, 20}, 10, 10, 10};                     // marble cube

    wall0.material(&plain_blue);
    wall1.material(&plain_red);
    wall2.material(&plain_white);
    wall3.material(&plain_white);
    wall4.material(&plain_white);
    box.rotation(iso::degrees(0), iso::degrees(0), iso::degrees(35));
    box.material(&glass);
    ball.material(&mediums::metals::stainless);
    marble_cube.material(&mediums::metals::stainless);

    speck top_light{raytrace::point{0, 0, 150}, colors::white, lights::intensities::radiant * 8.0_p};

    scene test_scene;
    test_scene.add_light(&top_light);
    test_scene.add_object(&wall0);
    test_scene.add_object(&wall1);
    test_scene.add_object(&wall2);
    test_scene.add_object(&wall3);
    test_scene.add_object(&wall4);
    test_scene.add_object(&box);
    test_scene.add_object(&ball);
    test_scene.add_object(&marble_cube);
    test_scene.set_ambient_light(colors::black);

    // Ray toward the ball center (0, 30, 30) — should hit the ball
    raytrace::vector dir_to_ball = (raytrace::point{0, 30, 30} - raytrace::point{-220, 0, 80}).normalized();
    ray ball_ray(raytrace::point{-220, 0, 80}, dir_to_ball);
    color ball_result = test_scene.trace(ball_ray, mediums::vacuum, 3);
    // The ball should reflect light — should be non-black
    EXPECT_PRECISION_NE(0.0_p, ball_result.red());
    EXPECT_PRECISION_NE(0.0_p, ball_result.green());
    EXPECT_PRECISION_NE(0.0_p, ball_result.blue());

    // Ray toward the cuboid center (0, -30, 60) — should pass through glass
    raytrace::vector dir_to_cuboid = (raytrace::point{0, -30, 60} - raytrace::point{-220, 0, 80}).normalized();
    ray cuboid_ray(raytrace::point{-220, 0, 80}, dir_to_cuboid);
    color cuboid_result = test_scene.trace(cuboid_ray, mediums::vacuum, 3);
    // Should see through the glass to the back wall
    EXPECT_PRECISION_NE(0.0_p, cuboid_result.red());
    EXPECT_PRECISION_NE(0.0_p, cuboid_result.green());
    EXPECT_PRECISION_NE(0.0_p, cuboid_result.blue());

    // Ray toward the marble cube center (0, -25, 20)
    raytrace::vector dir_to_cube = (raytrace::point{0, -25, 20} - raytrace::point{-220, 0, 80}).normalized();
    ray cube_ray(raytrace::point{-220, 0, 80}, dir_to_cube);
    color cube_result = test_scene.trace(cube_ray, mediums::vacuum, 3);
    // Should see the marble cube
    EXPECT_PRECISION_NE(0.0_p, cube_result.red());
    EXPECT_PRECISION_NE(0.0_p, cube_result.green());
    EXPECT_PRECISION_NE(0.0_p, cube_result.blue());
}
