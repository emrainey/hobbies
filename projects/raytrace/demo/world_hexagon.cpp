///
/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Renders a raytraced image of a bunch of spheres
/// @date 2021-01-02
/// @copyright Copyright (c) 2021
///

#include <raytrace/raytrace.hpp>

#include "world.hpp"

using namespace raytrace;
using namespace iso::literals;

// Creates a hexagonal tile or column which is used to create a group.
// The triangles are all in world coordinates before being grouped together.
//     A--|--B
//   /   |   \
//  D----.----C
//  \   |   /
//   E--|--F
//
//     G--|--H
//   /   |   \
//  J----.----I
//  \   |   /
//   K--|--L
// This is all an effort to show why one should just use the .obj files instead of
// trying to build complex shapes out of triangles manually. In this case we want to make them
// various heights, so it'll be easier to do it this way for now.
class HexaTile {
public:
    HexaTile(raytrace::point const& P, precision height, raytrace::mediums::medium const* medium)
        : m_height{height}
        , A{-basal::cos_pi_3 + P.x, basal::sin_pi_3 + P.y, height + P.z}
        , G{-basal::cos_pi_3 + P.x, basal::sin_pi_3 + P.y, 0.0_p + P.z}
        , B{basal::cos_pi_3 + P.x, basal::sin_pi_3 + P.y, height + P.z}
        , H{basal::cos_pi_3 + P.x, basal::sin_pi_3 + P.y, 0.0_p + P.z}
        , C{1.0_p + P.x, 0.0_p + P.y, height + P.z}
        , I{1.0_p + P.x, 0.0_p + P.y, 0.0_p + P.z}
        , D{-1.0_p + P.x, 0.0_p + P.y, height + P.z}
        , J{-1.0_p + P.x, 0.0_p + P.y, 0.0_p + P.z}
        , E{-basal::cos_pi_3 + P.x, -basal::sin_pi_3 + P.y, height + P.z}
        , K{-basal::cos_pi_3 + P.x, -basal::sin_pi_3 + P.y, 0.0_p + P.z}
        , F{basal::cos_pi_3 + P.x, -basal::sin_pi_3 + P.y, height + P.z}
        , L{basal::cos_pi_3 + P.x, -basal::sin_pi_3 + P.y, 0.0_p + P.z}
        //--- TOP
        , ABC{A, B, C}
        , ACD{A, C, D}
        , DCF{D, C, F}
        , DFE{D, F, E}
        //--- SIDES
        , BAG{B, A, G}
        , GHB{G, H, B}
        , DAJ{D, A, J}
        , JGA{J, G, A}
        , DEK{D, E, K}
        , KJD{K, J, D}
        , EFL{E, F, L}
        , LKE{L, K, E}
        , FCI{F, C, I}
        , ILF{I, L, F}
        , CBH{C, B, H}
        , HIC{H, I, C}
        //--- BOTTOM
        , GJH{G, J, H}
        , HJI{H, J, I}
        , IJK{I, J, K}
        , KLI{K, L, I}
        , group{P}
    {
        group.add_object(&ABC);
        group.add_object(&ACD);
        group.add_object(&DCF);
        group.add_object(&DFE);
        group.add_object(&BAG);
        group.add_object(&GHB);
        group.add_object(&DAJ);
        group.add_object(&JGA);
        group.add_object(&GJH);
        group.add_object(&HJI);
        group.add_object(&IJK);
        group.add_object(&KLI);
        group.add_object(&DEK);
        group.add_object(&KJD);
        group.add_object(&EFL);
        group.add_object(&LKE);
        group.add_object(&FCI);
        group.add_object(&ILF);
        group.add_object(&CBH);
        group.add_object(&HIC);
        group.material(medium);
    }

    raytrace::objects::group& as_group() {
        return group;
    }

protected:
    precision m_height; ///< height of the column.
    raytrace::point A, G;
    raytrace::point B, H;
    raytrace::point C, I;
    raytrace::point D, J;
    raytrace::point E, K;
    raytrace::point F, L;
    //--- TOP
    raytrace::objects::triangle ABC;
    raytrace::objects::triangle ACD;
    raytrace::objects::triangle DCF;
    raytrace::objects::triangle DFE;
    //--- SIDES
    raytrace::objects::triangle BAG;
    raytrace::objects::triangle GHB;
    raytrace::objects::triangle DAJ;
    raytrace::objects::triangle JGA;
    raytrace::objects::triangle DEK;
    raytrace::objects::triangle KJD;
    raytrace::objects::triangle EFL;
    raytrace::objects::triangle LKE;
    raytrace::objects::triangle FCI;
    raytrace::objects::triangle ILF;
    raytrace::objects::triangle CBH;
    raytrace::objects::triangle HIC;
    //--- BOTTOM
    raytrace::objects::triangle GJH;
    raytrace::objects::triangle HJI;
    raytrace::objects::triangle IJK;
    raytrace::objects::triangle KLI;
    // as a group
    raytrace::objects::group group;
};

class HexagonalWorld : public world {
public:
    HexagonalWorld()
        : world{raytrace::point{0.0_p, -10.0_p, 15.0_p}, raytrace::point{0.0_p, 0.0_p, 4.0_p}, "Hexagonal World", "world_hexagon.tga"}
        , specks{}
        , hexagons{} {
        raytrace::point center = R3::origin;
        // there are 6 hexagons in the first ring around the center
        hexagons.push_back(new HexaTile{raytrace::point{0.0_p, 0.0_p, 0.0_p}, 3.5_p, &raytrace::mediums::metals::bronze});
        hexagons.push_back(new HexaTile{raytrace::point{1.5_p, basal::sin_pi_3, 0.0_p}, 3.9_p, &raytrace::mediums::metals::bronze});
        hexagons.push_back(new HexaTile{raytrace::point{0.0_p, 2.0_p * basal::sin_pi_3, 0.0_p}, 4.1_p, &raytrace::mediums::metals::bronze});
        hexagons.push_back(new HexaTile{raytrace::point{-1.5_p, basal::sin_pi_3, 0.0_p}, 4.3_p, &raytrace::mediums::metals::bronze});
        hexagons.push_back(new HexaTile{raytrace::point{-1.5_p, -basal::sin_pi_3, 0.0_p}, 4.0_p, &raytrace::mediums::metals::bronze});
        hexagons.push_back(new HexaTile{raytrace::point{0.0_p, -2.0_p * basal::sin_pi_3, 0.0_p}, 4.2_p, &raytrace::mediums::metals::bronze});
        hexagons.push_back(new HexaTile{raytrace::point{1.5_p, -basal::sin_pi_3, 0.0_p}, 3.7_p, &raytrace::mediums::metals::bronze});

        specks.push_back(new lights::speck(raytrace::point{+6, +6, 9}, colors::white, lights::intensities::intense));
        specks.push_back(new lights::speck(raytrace::point{-6, +6, 9}, colors::white, lights::intensities::intense));
        specks.push_back(new lights::speck(raytrace::point{-6, -6, 9}, colors::white, lights::intensities::intense));
        specks.push_back(new lights::speck(raytrace::point{+6, -6, 9}, colors::white, lights::intensities::intense));
    }

    ~HexagonalWorld() {
        for (auto& s : specks) {
            delete s;
        }
        for (auto& h : hexagons) {
            delete h;
        }
    }

    void add_to(scene& scene) override {
        // add the objects to the scene.
        for (auto& s : specks) {
            scene.add_light(s);
        }
        for (auto& h : hexagons) {
            scene.add_group(&h->as_group());
        }
    }

    raytrace::animation::anchors get_anchors() const override {
        raytrace::animation::anchors anchors;
        anchors.push_back(animation::Anchor{animation::Attributes{looking_from(), looking_at(), 55.0_deg},
                                            animation::Attributes{looking_from(), looking_at(), 55.0_deg}, animation::Mappers{},
                                            iso::seconds{1.0_p}});
        return anchors;
    }

protected:
    std::vector<raytrace::lights::speck*> specks;
    std::vector<HexaTile*> hexagons;
};

// declare a single instance and return the reference to it
world* get_world() {
    static HexagonalWorld my_world;
    return &my_world;
}
