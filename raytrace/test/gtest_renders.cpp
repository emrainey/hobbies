#include <gtest/gtest.h>
#include <raytrace/raytrace.hpp>

#include "linalg/gtest_helper.hpp"
#include "geometry/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

using namespace raytrace;
using namespace raytrace::colors;
using namespace raytrace::operators;


class RenderTest : public ::testing::Test {
public:
    RenderTest()
        : image_height(480)
        , image_width(640)
        , field_of_view(90)
        , checkers(4, colors::blue, colors::yellow)
        , checkers2(0.25, colors::blue, colors::white)
        , plastic(colors::cyan, ambient::none, colors::white, smoothness::small, roughness::tight)
        , rubber(colors::grey, ambient::none, colors::grey, smoothness::none, roughness::tight)
        , steel(colors::grey, smoothness::polished, roughness::tight)
        , polka(13, colors::cyan, colors::blue)
        //, beam_of_light(R3::point(20, 0, 21), colors::white, 1E3)
        , beam_of_light(raytrace::vector{-20, 0, -21}, colors::white, 1E3)
        , inner_light(raytrace::point(0,0,10), colors::white, 1E11)
        , look_at(0, 0, 10)
        , plane0(R3::point(0, 0, 0), R3::basis::Z, 1.0)
        , scenes()
        {}

    ~RenderTest() {}

    void SetUp() {
        element_type r = 40;
        raytrace::point look_froms[] = {
            raytrace::point( r*cos(0), r*sin(0), 40),
            raytrace::point( r*cos(iso::pi/4), r*sin(iso::pi/4), 40),
            raytrace::point( r*cos(iso::pi/2), r*sin(iso::pi/2), 40)
        };
        plane0.material(&plastic);
        for (size_t i = 0; i < number_of_scenes; i++) {
            scenes.push_back(new scene(image_height, image_width, field_of_view));
            vector look = look_at - look_froms[i];
            look.normalize();
            look *= 10;
            raytrace::point camera_principal = look_froms[i] + look;
            scenes[i]->view.move_to(look_froms[i], camera_principal);
            scenes[i]->add_light(&beam_of_light);
            scenes[i]->add_object(&plane0);
        }
    }

    void TearDown() {
        for (size_t i = 0; i < number_of_scenes; i++) {
            scenes[i]->clear();
            delete scenes[i];
        }
        scenes.clear();
    }

    void add_object(const object *ptr) {
        for (size_t i = 0; i < number_of_scenes; i++) {
            scenes[i]->add_object(ptr);
        }
    }

    void add_light(const light *ptr) {
        for (size_t i = 0; i < number_of_scenes; i++) {
            scenes[i]->add_light(ptr);
        }
    }

    void render_all(const char name[]) {
        constexpr size_t len = 128;

        for (size_t i = 0; i < number_of_scenes; i++) {
            char buffer[len];
            snprintf(buffer,len,"rendertest_%s_%zu.ppm", name, i);
            scenes[i]->print(buffer);
            scenes[i]->render(buffer, 1, 2);
        }
    }

protected:
    constexpr static size_t number_of_scenes = 3;
    const size_t image_height;
    const size_t image_width;
    iso::degrees field_of_view;
    mediums::checkerboard checkers;
    mediums::checkerboard checkers2;
    mediums::plain plastic;
    mediums::plain rubber;
    mediums::metal steel;
    mediums::dots polka;
    beam beam_of_light;
    speck inner_light;
    R3::point look_at;
    raytrace::plane plane0;
    std::vector<scene*> scenes;
};

TEST_F(RenderTest, DISABLED_Sphere) {
    raytrace::sphere shape(look_at, 10);
    checkers.mapper(std::bind(&raytrace::sphere::map, &shape, std::placeholders::_1));
    shape.material(&checkers);
    add_object(&shape);
    render_all("sphere");
}

TEST_F(RenderTest, DISABLED_Pyramid) {
    raytrace::pyramid shape(look_at, 10);
    shape.material(&plastic);
    add_object(&shape);
    render_all("pyramid");
}

TEST_F(RenderTest, DISABLED_Cube) {
    raytrace::cuboid shape(look_at, 10, 10, 10);
    iso::degrees rx(0);
    iso::degrees ry(0);
    iso::degrees rz(45);
    shape.rotation(rx, ry, rz);
    //shape.material(&checkers);
    //shape.material(&rubber);
    shape.material(&polka);
    add_object(&shape);
    render_all("cuboid");
}

TEST_F(RenderTest, DISABLED_Cylinder) {
    R3::point c0(0, 0, 10);
    R3::point c1(0, 0, 20);
    raytrace::cylinder shape(c0, 10, 10);
    raytrace::ring cap(c1, R3::basis::Z, 0, 10);
    //shape.material(&checkers);
    shape.material(&rubber);
    // cap.material(&checkers);
    cap.material(&rubber);
    add_object(&shape);
    add_object(&cap);
    render_all("cylinder");
}

TEST_F(RenderTest, DISABLED_Cone) {
    raytrace::cone shape(R3::origin, 10, 20);
    //shape.material(&checkers);
    shape.material(&rubber);
    add_object(&shape);
    render_all("cone");
}

TEST_F(RenderTest, DISABLED_Ring) {
    R3::vector up{{1,1,3}};
    raytrace::ring shape1(look_at, up, 5, 10);
    raytrace::ring shape2(look_at, -up, 5, 10);
    checkers2.mapper(std::bind(&raytrace::ring::map, &shape1, std::placeholders::_1));
    shape1.material(&checkers2);
    shape2.material(&checkers2);
    //shape1.material(&rubber);
    //shape2.material(&rubber);
    add_object(&shape1);
    add_object(&shape2);
    render_all("ring");
}

TEST_F(RenderTest, DISABLED_Triangle) {
    R3::point A(10, 0, 10);
    R3::point B(0,  0, 13);
    R3::point C(0, 10, 10);
    raytrace::triangle shape1(A, B, C);
    raytrace::triangle shape2(C, B, A);
    shape1.material(&rubber);
    shape2.material(&rubber);
    add_object(&shape1);
    add_object(&shape2);
    render_all("triangle");
}

TEST_F(RenderTest, DISABLED_Square) {
    square shape1(look_at, R3::basis::Z, 16, 16);
    square shape2(look_at,-R3::basis::Z, 16, 16);
    checkers2.mapper(std::bind(&raytrace::square::map, &shape1, std::placeholders::_1));
    shape1.material(&checkers2);
    shape2.material(&checkers2);
    add_object(&shape1);
    add_object(&shape2);
    render_all("square");
}

TEST_F(RenderTest, DISABLED_Torus) {
    torus shape(look_at, 10, 3);
    shape.material(&plastic);
    add_object(&shape);
    render_all("torus");
}

TEST_F(RenderTest, DISABLED_QuadraticParabola) {
    // a bowl sitting on the XY plane
    matrix C{{{ 1.0, 0.0, 0.0, 0.0},
              { 0.0, 1.0, 0.0, 0.0},
              { 0.0, 0.0, 0.0,-2.0},
              { 0.0, 0.0,-2.0, 0.0}}};
    quadratic shape(look_at, C);
    shape.material(&plastic);
    add_object(&shape);
    render_all("parabola");
}

TEST_F(RenderTest, DISABLED_QuadraticSaddle) {
    // FIXME (GTest) this is producing weird results, plane clipping?
    // a saddle is z = a*x*x + b*y*y + c where either a or b is slightly negative ~ 0.1
    matrix C{{{ 0.1, 0.0, 0.0, 0.0},
              { 0.0,-0.1, 0.0, 0.0},
              { 0.0, 0.0, 1.0, 0.0},
              { 0.0, 0.0, 0.0, 0.0}}};
    quadratic shape(look_at, C);
    shape.material(&plastic);
    add_object(&shape);
    render_all("saddle");
}

TEST_F(RenderTest, DISABLED_InclusiveOverlap) {
    raytrace::sphere s0(R3::point(5, 0, 10), 10);
    raytrace::sphere s1(R3::point(-5, 0, 10), 10);
    raytrace::overlap shape(s0, s1, overlap::type::inclusive);
    shape.material(&plastic);
    add_object(&shape);
    render_all("inclusive");
}

TEST_F(RenderTest, DISABLED_SubtractiveOverlap) {
    raytrace::sphere s0(R3::point(5, 0, 10), 10);
    raytrace::sphere s1(R3::point(-5, 0, 10), 10);
    raytrace::overlap shape(s1, s0, overlap::type::subtractive);
    shape.material(&plastic);
    add_object(&shape);
    render_all("subtractive_spheres");
}

TEST_F(RenderTest, DISABLED_AdditiveOverlap) {
    raytrace::sphere s0(R3::point(5, 0, 10), 10);
    raytrace::sphere s1(R3::point(-5, 0, 10), 10);
    raytrace::overlap shape(s1, s0, overlap::type::additive);
    shape.material(&plastic);
    shape.position(raytrace::point(7,7,7)); // the benifit is that the objects are grouped now...
    add_object(&shape);
    render_all("additive");
}

TEST_F(RenderTest, DISABLED_SubtractiveOverlap2) {
    constexpr bool debug = false;
    raytrace::cuboid outer_box(R3::point(0, 0, 10), 10, 10, 10);
    raytrace::cuboid inner_box(R3::point(0, 0, 10), 20, 5, 5);
    raytrace::overlap shape(outer_box, inner_box, overlap::type::subtractive);
    inner_box.rotation(iso::degrees(0), iso::degrees(0), iso::degrees(45));
    if constexpr (not debug) {
        // FIXME there are a lot of single hit returns from cuboids so it's hard to reason about
        shape.material(&steel);
        add_object(&shape);
        add_light(&inner_light);
    } else {
        outer_box.material(&steel);
        inner_box.material(&steel);
        add_object(&outer_box);
        add_object(&inner_box);
    }
    render_all("subtractive_cubes");
}

TEST_F(RenderTest, DISABLED_SphereSpotLight) {
    raytrace::ray r0(raytrace::point(-30, 0, 30), -R3::basis::Z);
    raytrace::spot spot0(r0, colors::white, 1E3, iso::degrees{45});
    raytrace::sphere shape(look_at, 10);
    shape.material(&checkers);
    add_object(&shape);
    add_light(&spot0);
    render_all("sphere_spot");
}

TEST_F(RenderTest, DISABLED_SphereBunchOfSpecks) {
    raytrace::sphere shape(look_at, 5);
    shape.material(&checkers);
    add_object(&shape);
    // create a 20x20 grid of specks above the sphere
    std::vector<raytrace::speck*> specks;
    for (size_t i = 0; i < 20; i++) {
        for (size_t j = 0; j < 20; j++) {
            raytrace::point pnt((i * 20.0) - 200.0, (j * 20.0) - 200.0, 2000.0);
            specks.push_back(new raytrace::speck(pnt, colors::white, 1E3));
            add_light(specks.back());
        }
    }
    render_all("sphere_specks");
    for (auto s : specks) {
        delete s;
    }
}
