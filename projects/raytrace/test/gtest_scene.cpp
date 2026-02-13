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
    raytrace::lights::beam sunlight{raytrace::vector{0, 0, -1}, raytrace::colors::white, lights::intensities::blinding};
    s0.material(&c0);
    s1.material(&raytrace::mediums::metals::bronze);
    s2.material(&raytrace::mediums::dull);

    iso::degrees fov(55);
    // tiny image, simple camera placement
    scene scene;
    raytrace::camera view(120, 180, fov);
    raytrace::point look_from(-1, 0, 0);
    raytrace::point look_at(0, 0, 0);
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

namespace raytrace::mediums {
class glowy : public raytrace::mediums::medium {
public:
    glowy(raytrace::color const& ambient, raytrace::color const& emissive) : medium() {
        m_ambient = ambient;
        m_emissive_color = emissive;
    }
    raytrace::color emissive(raytrace::point const& volumetric_point __attribute__((unused))) const override {
        return m_emissive_color;
    }

protected:
    raytrace::color m_emissive_color;
};
}  // namespace raytrace::mediums

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
