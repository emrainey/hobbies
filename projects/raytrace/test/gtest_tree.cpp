#include "basal/gtest_helper.hpp"

#include <basal/basal.hpp>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

using namespace raytrace;

TEST(TreeTest, IndexOf) {
    Bounds bounds{raytrace::point{-70, -42, -99}, raytrace::point{55, 147, 22}};
    tree::Node node{bounds};
    // check the middle
    raytrace::point point{-7.5_p, 52.5_p, -38.5_p};
    EXPECT_POINT_EQ(point, bounds.center());
    // check each extrema for being in the correct octant
    point = raytrace::point{-70, -42, -99};
    EXPECT_EQ(0, node.IndexOf(point));
    point = raytrace::point{-70, -42, 22};
    EXPECT_EQ(1, node.IndexOf(point));
    point = raytrace::point{-70, 147, -99};
    EXPECT_EQ(2, node.IndexOf(point));
    point = raytrace::point{-70, 147, 22};
    EXPECT_EQ(3, node.IndexOf(point));
    point = raytrace::point{55, -42, -99};
    EXPECT_EQ(4, node.IndexOf(point));
    point = raytrace::point{55, -42, 22};
    EXPECT_EQ(5, node.IndexOf(point));
    point = raytrace::point{55, 147, -99};
    EXPECT_EQ(6, node.IndexOf(point));
    point = raytrace::point{55, 147, 22};
    EXPECT_EQ(7, node.IndexOf(point));
}

TEST(TreeTest, SplitBounds) {
    Bounds bounds{raytrace::point{-70, -42, -99}, raytrace::point{55, 147, 22}};
    tree::Node node{bounds};
    objects::sphere s0{raytrace::point{5, 4, 9}, 3};
    EXPECT_TRUE(node.bounds().intersects(s0.get_world_bounds()));
    node.add_object(&s0);
    EXPECT_TRUE(node.has(&s0));
}

TEST(TreeTest, AddObjects) {
    Bounds bounds{raytrace::point{0, 0, 0}, raytrace::point{2, 2, 2}};
    tree::Node node{bounds};
    objects::sphere s0{raytrace::point{0.5_p, 0.5_p, 0.5_p}, 0.4_p};
    objects::sphere s1{raytrace::point{0.5_p, 0.5_p, 1.5_p}, 0.4_p};
    objects::sphere s2{raytrace::point{0.5_p, 1.5_p, 0.5_p}, 0.4_p};
    objects::sphere s3{raytrace::point{0.5_p, 1.5_p, 1.5_p}, 0.4_p};
    objects::sphere s4{raytrace::point{1.5_p, 0.5_p, 0.5_p}, 0.4_p};
    objects::sphere s5{raytrace::point{1.5_p, 0.5_p, 1.5_p}, 0.4_p};
    objects::sphere s6{raytrace::point{1.5_p, 1.5_p, 0.5_p}, 0.4_p};
    objects::sphere s7{raytrace::point{1.5_p, 1.5_p, 1.5_p}, 0.4_p};
    EXPECT_TRUE(node.bounds().intersects(s0.get_world_bounds()));
    EXPECT_TRUE(node.bounds().intersects(s1.get_world_bounds()));
    EXPECT_TRUE(node.bounds().intersects(s2.get_world_bounds()));
    EXPECT_TRUE(node.bounds().intersects(s3.get_world_bounds()));
    EXPECT_TRUE(node.bounds().intersects(s4.get_world_bounds()));
    EXPECT_TRUE(node.bounds().intersects(s5.get_world_bounds()));
    EXPECT_TRUE(node.bounds().intersects(s6.get_world_bounds()));
    EXPECT_TRUE(node.bounds().intersects(s7.get_world_bounds()));
    EXPECT_TRUE(node.add_object(&s0));
    EXPECT_TRUE(node.has(&s0));
    EXPECT_TRUE(node.contains(&s0));
    EXPECT_EQ(1U, node.direct_object_count());
    EXPECT_EQ(0U, node.node_count());
    EXPECT_TRUE(node.add_object(&s1));
    EXPECT_TRUE(node.has(&s1));
    EXPECT_TRUE(node.contains(&s1));
    EXPECT_EQ(2U, node.direct_object_count());
    EXPECT_EQ(0U, node.node_count());
    EXPECT_TRUE(node.add_object(&s2));
    EXPECT_TRUE(node.has(&s2));
    EXPECT_TRUE(node.contains(&s2));
    EXPECT_EQ(3U, node.direct_object_count());
    EXPECT_EQ(0U, node.node_count());
    EXPECT_TRUE(node.add_object(&s3));
    EXPECT_TRUE(node.has(&s3));
    EXPECT_TRUE(node.contains(&s3));
    EXPECT_EQ(4U, node.direct_object_count());
    EXPECT_EQ(0U, node.node_count());
    EXPECT_TRUE(node.add_object(&s4));
    EXPECT_TRUE(node.has(&s4));
    EXPECT_TRUE(node.contains(&s4));
    EXPECT_EQ(5U, node.direct_object_count());
    EXPECT_EQ(0U, node.node_count());
    EXPECT_TRUE(node.add_object(&s5));
    EXPECT_TRUE(node.has(&s5));
    EXPECT_TRUE(node.contains(&s5));
    EXPECT_EQ(6U, node.direct_object_count());
    EXPECT_EQ(0U, node.node_count());
    EXPECT_TRUE(node.add_object(&s6));
    EXPECT_TRUE(node.has(&s6));
    EXPECT_TRUE(node.contains(&s6));
    EXPECT_EQ(7U, node.direct_object_count());
    EXPECT_EQ(0U, node.node_count());
    EXPECT_TRUE(node.add_object(&s7));
    EXPECT_TRUE(node.has(&s7));
    EXPECT_TRUE(node.contains(&s7));
    EXPECT_EQ(8U, node.direct_object_count());
    EXPECT_EQ(0U, node.node_count());
    EXPECT_EQ(8U, node.all_object_count());  // all objects are in this node
    // no subnodes yet...
    objects::sphere s8{raytrace::point{1.0_p, 1.0_p, 1.0_p}, 0.5_p};  // should be in all subnodes
    EXPECT_TRUE(node.bounds().intersects(s8.get_world_bounds()));
    EXPECT_TRUE(node.add_object(&s8));
    EXPECT_TRUE(node.has(&s8));
    EXPECT_EQ(1U, node.direct_object_count());
    EXPECT_EQ(8U, node.node_count());
    EXPECT_EQ(9U, node.all_object_count());  // all objects are in this node
    EXPECT_FALSE(node.has(&s0));
    EXPECT_FALSE(node.has(&s1));
    EXPECT_FALSE(node.has(&s2));
    EXPECT_FALSE(node.has(&s3));
    EXPECT_FALSE(node.has(&s4));
    EXPECT_FALSE(node.has(&s5));
    EXPECT_FALSE(node.has(&s6));
    EXPECT_FALSE(node.has(&s7));
    EXPECT_TRUE(node.under(&s0));
    EXPECT_TRUE(node.under(&s1));
    EXPECT_TRUE(node.under(&s2));
    EXPECT_TRUE(node.under(&s3));
    EXPECT_TRUE(node.under(&s4));
    EXPECT_TRUE(node.under(&s5));
    EXPECT_TRUE(node.under(&s6));
    EXPECT_TRUE(node.under(&s7));
    EXPECT_TRUE(node.contains(&s0));
    EXPECT_TRUE(node.contains(&s1));
    EXPECT_TRUE(node.contains(&s2));
    EXPECT_TRUE(node.contains(&s3));
    EXPECT_TRUE(node.contains(&s4));
    EXPECT_TRUE(node.contains(&s5));
    EXPECT_TRUE(node.contains(&s6));
    EXPECT_TRUE(node.contains(&s7));

    raytrace::ray r{raytrace::point{0.5_p, -1.0_p, 0.5_p}, raytrace::vector{0.0_p, 1.0_p, 0.0_p}};
    EXPECT_TRUE(node.bounds().intersects(r));
    EXPECT_EQ(8U, node.node_count());
    EXPECT_TRUE(node.any_of([&](tree::Node const& subnode) -> bool { return subnode.bounds().intersects(r); }));
    auto hits = node.intersects(r);
    EXPECT_EQ(2U, hits.size());  // should hit s0 and s2
    EXPECT_EQ(&s0, hits[0].object);
    EXPECT_EQ(&s2, hits[1].object);

    EXPECT_EQ(1U, node.count_of(&s0));  // should be in one subnode
    EXPECT_EQ(1U, node.count_of(&s1));  // should be in one subnode
    EXPECT_EQ(1U, node.count_of(&s2));  // should be in one subnode
    EXPECT_EQ(1U, node.count_of(&s3));  // should be in one subnode
    EXPECT_EQ(1U, node.count_of(&s4));  // should be in one subnode
    EXPECT_EQ(1U, node.count_of(&s5));  // should be in one subnode
    EXPECT_EQ(1U, node.count_of(&s6));  // should be in one subnode
    EXPECT_EQ(1U, node.count_of(&s7));  // should be in one subnode
}

static void subspheres(std::vector<raytrace::objects::sphere*>& spheres, raytrace::point const& center, precision R,
                       size_t depth) {
    if (depth > 0) {
        precision radius = R / 6.0_p;
        for (precision z = -R; z <= R; z += R) {
            for (precision y = -R; y <= R; y += R) {
                for (precision x = -R; x <= R; x += R) {
                    // if all are zero, continue
                    if (basal::equivalent(x, 0.0_p) and basal::equivalent(y, 0.0_p) and basal::equivalent(z, 0.0_p)) {
                        continue;
                    }
                    spheres.push_back(new raytrace::objects::sphere(center + R3::vector{{x, y, z}}, radius));
                    subspheres(spheres, spheres.back()->position(), R / 3.0_p, depth - 1);
                }
            }
        }
    }
}

TEST(TreeTest, AddSubSpheres) {
    using namespace geometry::operators;
    size_t depth = 2;                                          // how many levels of spheres to create
    size_t count = 1 + 26U + basal::exponentiate(26U, depth);  // 1 root + 26^depth sub-spheres
    precision radius = 10.0_p;                                 // radius of the root sphere
    Bounds bounds{raytrace::point{-1, -1, -1}, raytrace::point{1, 1, 1}};

    std::vector<raytrace::objects::sphere*> spheres;
    spheres.push_back(new raytrace::objects::sphere(R3::origin, radius));  // add the root sphere
    subspheres(spheres, R3::origin, radius, depth);                        // create a bunch of spheres
    // assert that each sphere is in a unqiue location
    std::set<raytrace::point> unique_positions;
    size_t index = 0U;
    for (auto const* obj : spheres) {
        auto pos = obj->position();
        ASSERT_TRUE(unique_positions.insert(pos).second)
            << "duplicate sphere at position: " << pos << " which is index " << index;
        index++;
    }
    ASSERT_EQ(spheres.size(), count);  // check we have the right number of spheres
    for (auto const* obj : spheres) {
        auto object_bounds = obj->get_world_bounds();
        bounds.grow(object_bounds);  // grow the bounds to include all the spheres
        EXPECT_TRUE(bounds.intersects(object_bounds));
    }
    tree::Node node{bounds};
    for (auto const* obj : spheres) {
        EXPECT_TRUE(node.add_object(obj));
        EXPECT_TRUE(node.contains(obj)) << "object: " << obj->get_world_bounds()
                                        << " not under node: " << node.bounds();
        EXPECT_LE(1U, node.count_of(obj)) << "object: " << obj->get_world_bounds() << " should be in one subnode";
    }
    std::cout << "There are " << node.all_object_count() << " objects in the node" << std::endl;
    std::cout << "The root bounds are " << node.bounds() << std::endl;
    std::cout << "There are " << spheres.size() << " spheres in the vector" << std::endl;

    // lets shoot some rays at the node and see what we hit
    {
        raytrace::ray r{raytrace::point{0.0_p, -14.0_p, 0.0_p}, raytrace::vector{0.0_p, 1.0_p, 0.0_p}};
        EXPECT_TRUE(node.bounds().intersects(r));  // should intersect with the root bound
        auto hits = node.intersects(r);
        EXPECT_EQ(7U, hits.size());  // should hit each sphere across the border areas and the root sphere.
        EXPECT_EQ(spheres[0], hits[0].object);
        // all the objects should be unique in the hits
        for (auto const& hit : hits) {
            size_t unique_count = 0;
            for (auto const& other_hit : hits) {
                if (hit.object == other_hit.object) {
                    unique_count++;
                }
            }
            ASSERT_EQ(1U, unique_count) << "hit object: " << hit.object->get_world_bounds()
                                        << " is not unique in the hits";
        }
    }
    {
        raytrace::ray r{raytrace::point{0.0_p, 14.0_p, 0.0_p}, raytrace::vector{0.0_p, -1.0_p, 0.0_p}};
        EXPECT_TRUE(node.bounds().intersects(r));  // should intersect with the root bound
        auto hits = node.intersects(r);
        EXPECT_EQ(7U, hits.size());  // should hit each sphere across the border areas and the root sphere.
        EXPECT_EQ(spheres[0], hits[0].object);
        // all the objects should be unique in the hits
        for (auto const& hit : hits) {
            size_t unique_count = 0;
            for (auto const& other_hit : hits) {
                if (hit.object == other_hit.object) {
                    unique_count++;
                }
            }
            ASSERT_EQ(1U, unique_count) << "hit object: " << hit.object->get_world_bounds()
                                        << " is not unique in the hits";
        }
    }
}
