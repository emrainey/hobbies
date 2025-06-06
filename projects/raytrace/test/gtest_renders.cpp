#include <gtest/gtest.h>

#include <raytrace/raytrace.hpp>
#include <raytrace/objparser.hpp>
#include <raytrace/objects/model.hpp>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

using namespace raytrace;
using namespace raytrace::objects;
// using namespace raytrace::lights;
using namespace raytrace::colors;
using namespace raytrace::operators;

using namespace basal::literals;

class RenderTest : public ::testing::Test {
public:
    RenderTest()
        : image_height{480}
        , image_width{640}
        , field_of_view{90}
        , checkers{4, colors::blue, colors::yellow}
        , checkers2{0.25_p, colors::blue, colors::white}
        , plastic{colors::cyan, mediums::ambient::none, colors::white, mediums::smoothness::small, roughness::tight}
        , rubber{colors::grey, mediums::ambient::none, colors::grey, mediums::smoothness::none, roughness::tight}
        , steel{colors::grey, mediums::smoothness::polished, roughness::tight}
        , polka{13, colors::cyan, colors::blue}
        , beam_of_light{raytrace::vector{-20, 0, -21}, colors::white, lights::intensities::full}
        , inner_light{raytrace::point{0, 0, 10}, colors::white, lights::intensities::moderate}
        , look_at{0, 0, 10}
        , plane0{}
        , scenes{} {
    }

    ~RenderTest() {
    }

    void SetUp() {
        precision r = 40.0_p;
        raytrace::point look_froms[]
            = {raytrace::point{static_cast<precision>(r * cos(0)), static_cast<precision>(r * sin(0)), 40.0_p},
               raytrace::point{static_cast<precision>(r * cos(iso::pi / 4)),
                               static_cast<precision>(r * sin(iso::pi / 4)), 40.0_p},
               raytrace::point{static_cast<precision>(r * cos(iso::pi / 2)),
                               static_cast<precision>(r * sin(iso::pi / 2)), 40.0_p}};
        plane0.material(&plastic);
        for (size_t i = 0; i < number_of_scenes; i++) {
            scenes.push_back(new scene{});
            views.push_back(new camera{image_height, image_width, field_of_view});
            vector look = look_at - look_froms[i];
            look.normalize();
            look *= 10;
            raytrace::point camera_principal = look_froms[i] + look;
            views[i]->move_to(look_froms[i], camera_principal);
            scenes[i]->add_light(&beam_of_light);
            scenes[i]->add_object(&plane0);
        }
    }

    void TearDown() {
        for (size_t i = 0; i < number_of_scenes; i++) {
            scenes[i]->clear();
            delete scenes[i];
            delete views[i];
        }
        scenes.clear();
    }

    void add_object(object const *ptr) {
        for (size_t i = 0; i < number_of_scenes; i++) {
            scenes[i]->add_object(ptr);
        }
    }

    void add_light(lights::light const *ptr) {
        for (size_t i = 0; i < number_of_scenes; i++) {
            scenes[i]->add_light(ptr);
        }
    }

    void render_all(char const name[]) {
        constexpr size_t len = 128;

        for (size_t i = 0; i < number_of_scenes; i++) {
            char buffer[len];
            snprintf(buffer, len, "rendertest_%s_%zu.ppm", name, i);
            scenes[i]->print(std::cout, buffer);
            scenes[i]->render(*views[i], buffer, 1, 2);
            fprintf(stdout, "Intersections %zu (Point/Points/Line) %zu %zu %zu\n",
                    raytrace::statistics::get().intersections_with_objects,
                    raytrace::statistics::get().intersections_with_point,
                    raytrace::statistics::get().intersections_with_points,
                    raytrace::statistics::get().intersections_with_line);
        }
    }

protected:
    constexpr static size_t number_of_scenes{3ul};

    size_t const image_height;
    size_t const image_width;
    iso::degrees field_of_view;
    mediums::checkerboard checkers;
    mediums::checkerboard checkers2;
    mediums::plain plastic;
    mediums::plain rubber;
    mediums::metal steel;
    mediums::dots polka;
    lights::beam beam_of_light;
    lights::speck inner_light;
    raytrace::point look_at;
    raytrace::objects::plane plane0;
    std::vector<scene *> scenes;
    std::vector<camera *> views;
};

TEST_F(RenderTest, DISABLED_Sphere) {
    raytrace::objects::sphere shape(look_at, 10);
    checkers.mapper(std::bind(&raytrace::objects::sphere::map, &shape, std::placeholders::_1));
    shape.material(&checkers);
    add_object(&shape);
    render_all("sphere");
}

TEST_F(RenderTest, DISABLED_Pyramid) {
    raytrace::objects::pyramid shape(look_at, 10);
    shape.material(&plastic);
    add_object(&shape);
    render_all("pyramid");
}

TEST_F(RenderTest, DISABLED_Cube) {
    raytrace::objects::cuboid shape(look_at, 10, 10, 10);
    iso::degrees rx{0};
    iso::degrees ry{0};
    iso::degrees rz{45};
    shape.rotation(rx, ry, rz);
    // shape.material(&checkers);
    // shape.material(&rubber);
    shape.material(&polka);
    add_object(&shape);
    render_all("cuboid");
}

TEST_F(RenderTest, DISABLED_WallBox) {
    raytrace::objects::wall px(look_at, R3::pitch(0.25), 10);
    raytrace::objects::wall py(look_at, R3::roll(-0.25), 10);
    raytrace::objects::wall pz(look_at, R3::identity, 10);
    px.material(&plastic);
    py.material(&plastic);
    pz.material(&plastic);
    raytrace::objects::overlap column{px, py, overlap::type::inclusive};
    raytrace::objects::overlap wall_box{column, pz, overlap::type::inclusive};
    add_object(&wall_box);
    render_all("wall_box");
}

TEST_F(RenderTest, DISABLED_Cylinder) {
    R3::point c0{0, 0, 10};
    R3::point c1{0, 0, 20};
    raytrace::objects::cylinder shape(c0, 10, 10);
    raytrace::objects::ring cap(c1, R3::identity, 0, 10);
    // shape.material(&checkers);
    shape.material(&rubber);
    // cap.material(&checkers);
    cap.material(&rubber);
    add_object(&shape);
    add_object(&cap);
    render_all("cylinder");
}

TEST_F(RenderTest, DISABLED_Cone) {
    raytrace::objects::cone shape(R3::origin, 10, 20);
    // shape.material(&checkers);
    shape.material(&rubber);
    add_object(&shape);
    render_all("cone");
}

TEST_F(RenderTest, DISABLED_Ring) {
    auto M = R3::yaw(0.125) * R3::pitch(-0.125) * R3::roll(0.125);
    raytrace::objects::ring shape1(look_at, M, 5, 10);
    raytrace::objects::ring shape2(look_at, M.inverse(), 5, 10);
    checkers2.mapper(std::bind(&raytrace::objects::ring::map, &shape2, std::placeholders::_1));
    shape1.material(&checkers2);
    shape2.material(&checkers2);
    // shape1.material(&rubber);
    // shape2.material(&rubber);
    add_object(&shape1);
    add_object(&shape2);
    render_all("ring");
}

TEST_F(RenderTest, DISABLED_Triangle) {
    R3::point A{10, 0, 10};
    R3::point B{0, 0, 13};
    R3::point C{0, 10, 10};
    raytrace::objects::triangle shape1({A, B, C});
    raytrace::objects::triangle shape2({C, B, A});
    shape1.material(&rubber);
    shape2.material(&rubber);
    add_object(&shape1);
    add_object(&shape2);
    render_all("triangle");
}

TEST_F(RenderTest, DISABLED_Square) {
    square shape1(look_at, R3::identity, 32);
    square shape2(look_at, R3::roll(0.5), 32);
    checkers2.mapper(std::bind(&raytrace::objects::square::map, &shape1, std::placeholders::_1));
    shape1.material(&checkers2);
    shape2.material(&checkers2);
    add_object(&shape1);
    add_object(&shape2);
    render_all("square");
}

TEST_F(RenderTest, DISABLED_Torus) {
    torus shape(look_at, 10, 3);
    checkers.mapper(std::bind(&raytrace::objects::torus::map, &shape, std::placeholders::_1));
    shape.material(&checkers);
    add_object(&shape);
    render_all("torus");
}

TEST_F(RenderTest, DISABLED_QuadraticCylinder) {
    ellipticalcylinder shape(look_at, 5.0_p, 5.0_p);
    ASSERT_FALSE(shape.has_definite_volume());
    shape.material(&plastic);
    add_object(&shape);
    render_all("ellipticalcylinder");
}

TEST_F(RenderTest, DISABLED_QuadraticCone) {
    ellipticalcone shape(look_at, 0.3_p, 0.3_p);
    ASSERT_FALSE(shape.has_definite_volume());
    shape.material(&plastic);
    add_object(&shape);
    render_all("ellipticalcone");
}

TEST_F(RenderTest, DISABLED_QuadraticEllipsoid) {
    ellipsoid shape(look_at, 10.0_p, 8.0_p, 6.0_p);
    ASSERT_TRUE(shape.has_definite_volume());
    shape.material(&plastic);
    add_object(&shape);
    render_all("ellipsoid");
}

TEST_F(RenderTest, DISABLED_QuadraticParaboloid) {
    paraboloid shape(look_at, 0.5_p, 0.5_p, 1.0_p);
    ASSERT_FALSE(shape.has_definite_volume());
    shape.material(&plastic);
    add_object(&shape);
    render_all("paraboloid");
}

TEST_F(RenderTest, DISABLED_QuadraticHyperboloid) {
    hyperboloid shape(look_at, 3.0_p, 3.0_p, 3.0_p);
    ASSERT_FALSE(shape.has_definite_volume());
    shape.material(&plastic);
    add_object(&shape);
    render_all("hyperboloid");
}

TEST_F(RenderTest, DISABLED_InclusiveOverlap) {
    raytrace::objects::sphere s0{R3::point(5, 0, 10), 10};
    raytrace::objects::sphere s1{R3::point(-5, 0, 10), 10};
    raytrace::objects::overlap shape(s1, s0, overlap::type::inclusive);
    shape.material(&plastic);
    add_object(&shape);
    render_all("inclusive");
}

TEST_F(RenderTest, DISABLED_SubtractiveOverlap) {
    raytrace::objects::sphere s0{R3::point(5, 0, 10), 10};
    raytrace::objects::sphere s1{R3::point(-5, 0, 10), 10};
    raytrace::objects::overlap shape(s1, s0, overlap::type::subtractive);
    shape.material(&plastic);
    add_object(&shape);
    render_all("subtractive_spheres");
}

TEST_F(RenderTest, DISABLED_AdditiveOverlap) {
    raytrace::objects::sphere s0{R3::point(5, 0, 10), 10};
    raytrace::objects::sphere s1{R3::point(-5, 0, 10), 10};
    raytrace::objects::overlap shape(s1, s0, overlap::type::additive);
    shape.material(&plastic);
    shape.position(raytrace::point{7, 7, 7});  // the benefit is that the objects are grouped now...
    add_object(&shape);
    render_all("additive");
}

TEST_F(RenderTest, DISABLED_SubtractiveOverlapCubes) {
    constexpr bool debug = false;
    raytrace::objects::cuboid outer_box(R3::point(0, 0, 10), 10, 10, 10);
    raytrace::objects::cuboid inner_box(R3::point(0, 0, 10), 30, 5, 5);
    inner_box.rotation(iso::degrees(0), iso::degrees(-45), iso::degrees(0));
    raytrace::objects::overlap shape(outer_box, inner_box, overlap::type::subtractive);
    if constexpr (not debug) {
        // FIXME there are a lot of single hit returns from cuboids so it's hard to reason about
        shape.material(&steel);
        add_object(&shape);
        add_light(&inner_light);  // FIXME this is producing weird speckles on the floor
    } else {
        outer_box.material(&steel);
        inner_box.material(&steel);
        add_object(&outer_box);
        add_object(&inner_box);
    }
    render_all("subtractive_cubes");
}

TEST_F(RenderTest, DISABLED_SphereSpotLight) {
    raytrace::ray r0{raytrace::point{-30, 0, 30}, -R3::basis::Z};
    raytrace::lights::spot spot0(r0, colors::white, 1E3, iso::degrees{45});
    raytrace::objects::sphere shape(look_at, 10);
    shape.material(&checkers2);
    add_object(&shape);
    add_light(&spot0);
    render_all("sphere_spot");
}

TEST_F(RenderTest, DISABLED_SphereBunchOfSpecks) {
    raytrace::objects::sphere shape(look_at, 5);
    shape.material(&checkers2);
    add_object(&shape);
    int const count = 5U;  // should be odd to get the best effect
    // create a CountxCount grid of specks above the sphere
    int const half = count / 2;
    std::vector<raytrace::lights::speck *> specks;
    for (int i = -half; i <= half; i++) {
        for (int j = -half; j <= half; j++) {
            precision x = static_cast<precision>(i) * 2.0_p;
            precision y = static_cast<precision>(j) * 2.0_p;
            raytrace::point pnt(x, y, 20.0_p);
            specks.push_back(new raytrace::lights::speck(pnt, colors::white, lights::intensities::full));
            add_light(specks.back());
        }
    }
    render_all("sphere_specks");
    for (auto s : specks) {
        delete s;
    }
}

TEST_F(RenderTest, DISABLED_Model) {
    // use the model in the test directory named cube.obj
    raytrace::objects::Model shape;
    shape.LoadFromFile("../projects/raytrace/test/cube.obj");
    shape.position(look_at);
    shape.material(&plastic);
    add_object(&shape);
    render_all("model_cube");
}
