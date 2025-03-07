///
/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief A Steel Snowman scene
/// @date 2021-01-02
/// @copyright Copyright (c) 2021
///

#include <raytrace/raytrace.hpp>
#include <raytrace/mediums/starfield.hpp>

#include "group.hpp"
#include "world.hpp"

using namespace raytrace;
using namespace iso::literals;

class Tree : public group {
public:
    Tree(raytrace::point const& base)
        : group{}
        , leaves{colors::forest_green, 0.0, colors::forest_green, 0.01, 100}
        , bark{colors::brown, 0.0, colors::brown, 0.01, 100}
        , body{raytrace::point{0.0, 0.0, 3.0}, 3, 6}
        , trunk{raytrace::point{0, 0, 2.5}, 2.5, 0.8} {
        m_objects.push_back(&body);
        m_objects.push_back(&trunk);
        body.material(&leaves);
        trunk.material(&bark);
        auto by = base - R3::origin;
        move_by(by);
    }
    ~Tree() {
    }

protected:
    raytrace::mediums::plain leaves;
    raytrace::mediums::plain bark;
    raytrace::objects::cone body;
    raytrace::objects::cylinder trunk;
};

class SnowmanWorld : public world {
public:
    SnowmanWorld()
        : world{}
        , look_from{-20, 14, 5.5}
        , look_at{0, 0, 5.5}
        , snow(colors::white, mediums::ambient::dim, colors::white, mediums::smoothness::none,
               mediums::roughness::tight)
        , red_eyes(colors::red, mediums::ambient::glowy, colors::red, mediums::smoothness::none,
                   mediums::roughness::tight)
        , carrot(colors::orange, mediums::ambient::dim, colors::orange, mediums::smoothness::none,
                 mediums::roughness::tight)
        , plum{colors::plum, mediums::ambient::none, colors::plum, mediums::smoothness::none, mediums::roughness::loose}
        , ground{raytrace::point{0, 0, 0}, R3::basis::Z}
        , sn_btm{raytrace::point{0, 0, 2}, 2.5}
        , sn_mid{raytrace::point{0, 0, 5}, 2}
        , sn_top{raytrace::point{0, 0, 7.5}, 1.5}
        , left_eye{raytrace::point{-0.5, 1.1, 8}, 0.3}
        , right_eye{raytrace::point{0.5, 1.1, 8}, 0.3}
        , nose{raytrace::point{0.0, 1.3, 7.5}, 0.3, 1.5}
        , hat_btm{raytrace::point{0, 0, 8.8}, R3::vector{{0.0, -0.2, -1.0}}, 0.0, 1.5}
        , hat_core{raytrace::point{0.0, 0.05, 9.05}, raytrace::point{0.0, 0.4, 10.8}, 1.0}
        , hat_brim{raytrace::point{0.0, 0.0, 8.8}, raytrace::point{0.0, 0.05, 9.05}, 1.5}
        , hat_rim{raytrace::point{0, 0.05, 9.05}, R3::vector{{0.0, 0.2, 1.0}}, 0.0, 1.5}
        , hat_ribbon{raytrace::point{0, 0.05, 9.05}, raytrace::point{0, 0.1, 9.3}, 1.1}
        , hat_top{raytrace::point{0, 0.4, 10.8}, R3::vector{{0.0, 0.2, 1.0}}, 0.0, 1.0}
        , left_arm{raytrace::point{1, 0, 6.5}, raytrace::point{4, 0, 5}, 0.2}
        , right_arm{raytrace::point{-1, 0, 6.5}, raytrace::point{-4, 0, 5}, 0.2}
        , left_elbow{raytrace::point{4, 0, 5}, 0.2}
        , right_elbow{raytrace::point{-4, 0, 5}, 0.2}
        , left_forearm{raytrace::point{4, 0, 5}, raytrace::point{2.5, 0, 3}, 0.2}
        , right_forearm{raytrace::point{-4, 0, 5}, raytrace::point{-2.5, 0, 3}, 0.2}
        , left_hand{raytrace::point{2.5, 0, 3}, 0.4}
        , right_hand{raytrace::point{-2.5, 0, 3}, 0.4}
        , tree01{raytrace::point{-9, -5, 0}}
        , tree02{raytrace::point{-5, -12, 0}}
        , tree03{raytrace::point{-7, -8, 0}}
        , tree04{raytrace::point{-7, -1, 0}}
        , tree05{raytrace::point{-4, -3, 0}}
        , tree06{raytrace::point{-15, -11, 0}}
        , tree07{raytrace::point{0, -11, 0}}
        , tree08{raytrace::point{9, -5, 0}}
        , tree09{raytrace::point{5, -12, 0}}
        , tree10{raytrace::point{7, -8, 0}}
        , tree11{raytrace::point{7, -1, 0}}
        , tree12{raytrace::point{4, -3, 0}}
        , moonlight{R3::vector{{-1, -1, -1}}, colors::silver, lights::intensities::full}
        , lights{}
        , starfield{1024} {
        ground.material(&snow);
        sn_btm.material(&mediums::metals::stainless);
        sn_mid.material(&mediums::metals::stainless);
        sn_top.material(&mediums::metals::stainless);
        left_arm.material(&mediums::metals::stainless);
        right_arm.material(&mediums::metals::stainless);
        left_elbow.material(&mediums::metals::stainless);
        right_elbow.material(&mediums::metals::stainless);
        left_forearm.material(&mediums::metals::stainless);
        right_forearm.material(&mediums::metals::stainless);
        left_hand.material(&mediums::metals::stainless);
        right_hand.material(&mediums::metals::stainless);
        left_eye.material(&red_eyes);
        right_eye.material(&red_eyes);
        hat_ribbon.material(&red_eyes);
        nose.material(&carrot);
        hat_btm.material(&plum);
        hat_core.material(&plum);
        hat_brim.material(&plum);
        hat_rim.material(&plum);
        hat_ribbon.material(&red_eyes);
        hat_top.material(&plum);
        for (int i = 0; i <= 10; i++) {
            // lights.push_back(new lights::bulb(raytrace::point{i - 5, 0, 15}, 1.0, colors::white,
            // lights::intensities::blinding, 4)); lights.push_back(new lights::speck(raytrace::point{i - 5, 0, 15},
            // colors::white, 5));
        }
        // move the nose into the right orientation
        nose.rotation(iso::degrees(270), iso::degrees(0), iso::degrees(0));
    }

    ~SnowmanWorld() {
        for (auto& l : lights) {
            delete l;
        }
    }

    raytrace::point& looking_from() override {
        return look_from;
    }

    raytrace::point& looking_at() override {
        return look_at;
    }

    std::string window_name() const override {
        return std::string("Snowman");
    }

    std::string output_filename() const override {
        return std::string("world_snowman.tga");
    }

    raytrace::color background(raytrace::ray const& world_ray) const override {
        return starfield(world_ray.direction());
    }

    void add_to(scene& scene) override {
        // add the objects to the scene.
        scene.add_object(&ground);
        scene.add_object(&sn_btm);
        scene.add_object(&sn_mid);
        scene.add_object(&sn_top);
        scene.add_object(&left_eye);
        scene.add_object(&right_eye);
        scene.add_object(&nose);
        scene.add_object(&hat_btm);
        scene.add_object(&hat_core);
        scene.add_object(&hat_brim);
        scene.add_object(&hat_rim);
        scene.add_object(&hat_ribbon);
        scene.add_object(&hat_top);
        scene.add_object(&left_arm);
        scene.add_object(&right_arm);
        scene.add_object(&left_elbow);
        scene.add_object(&right_elbow);
        scene.add_object(&left_forearm);
        scene.add_object(&right_forearm);
        scene.add_object(&left_hand);
        scene.add_object(&right_hand);
        for (auto& s : lights) {
            scene.add_light(s);
        }
        scene.add_light(&moonlight);
        tree01.add_to_scene(scene);
        tree02.add_to_scene(scene);
        tree03.add_to_scene(scene);
        tree04.add_to_scene(scene);
        tree05.add_to_scene(scene);
        tree06.add_to_scene(scene);
        tree07.add_to_scene(scene);
        tree08.add_to_scene(scene);
        tree09.add_to_scene(scene);
        tree10.add_to_scene(scene);
        tree11.add_to_scene(scene);
        tree12.add_to_scene(scene);
    }

    raytrace::animation::anchors get_anchors() const override {
        raytrace::animation::anchors anchors;
        anchors.push_back(animation::Anchor{animation::Attributes{look_from, look_at, 35.0_deg},
                                            animation::Attributes{raytrace::point{20, 14, 5.5}, look_at, 35.0_deg},
                                            animation::Mappers{}, iso::seconds{5.0_p}});
        anchors.push_back(animation::Anchor{anchors.back().limit,  // previous limit is this start
                                            animation::Attributes{look_from, look_at, 35.0_deg}, animation::Mappers{},
                                            iso::seconds{5.0_p}});
        return anchors;
    }

protected:
    raytrace::point look_from;
    raytrace::point look_at;
    raytrace::mediums::plain snow;
    raytrace::mediums::plain red_eyes;
    raytrace::mediums::plain carrot;
    raytrace::mediums::plain plum;
    // plane
    raytrace::objects::plane ground;
    // snowman body
    raytrace::objects::sphere sn_btm;
    raytrace::objects::sphere sn_mid;
    raytrace::objects::sphere sn_top;
    // snowman arms
    raytrace::objects::cylinder left_arm;
    raytrace::objects::cylinder right_arm;
    raytrace::objects::sphere left_elbow;
    raytrace::objects::sphere right_elbow;
    raytrace::objects::cylinder left_forearm;
    raytrace::objects::cylinder right_forearm;
    raytrace::objects::sphere left_hand;
    raytrace::objects::sphere right_hand;
    // snowman face
    raytrace::objects::sphere left_eye;
    raytrace::objects::sphere right_eye;
    raytrace::objects::cone nose;
    // hat
    raytrace::objects::ring hat_btm;
    raytrace::objects::cylinder hat_core;
    raytrace::objects::cylinder hat_brim;
    raytrace::objects::ring hat_rim;
    raytrace::objects::cylinder hat_ribbon;
    raytrace::objects::ring hat_top;
    // trees
    Tree tree01;
    Tree tree02;
    Tree tree03;
    Tree tree04;
    Tree tree05;
    Tree tree06;
    Tree tree07;
    Tree tree08;
    Tree tree09;
    Tree tree10;
    Tree tree11;
    Tree tree12;
    // the lights
    // std::vector<raytrace::lights::bulb *>lights;
    std::vector<raytrace::lights::speck*> lights;
    raytrace::lights::beam moonlight;
    raytrace::mediums::StarField starfield;
};

// declare a single instance and return the reference to it
world* get_world() {
    static SnowmanWorld my_world;
    return &my_world;
}
