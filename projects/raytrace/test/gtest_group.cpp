#include "gtest/gtest.h"

#include "geometry/gtest_helper.hpp"
#include "raytrace/objects/group.hpp"
#include "raytrace/objects/plane.hpp"
#include "raytrace/objects/sphere.hpp"

#include <memory>

namespace raytrace {
namespace objects {
namespace test {

class GroupTest : public ::testing::Test {
protected:
    void SetUp() override {
        origin = point{0.0_p, 0.0_p, 0.0_p};
        sphere1 = std::make_unique<sphere>(point{1.0_p, 0.0_p, 0.0_p}, 0.5_p);
        sphere2 = std::make_unique<sphere>(point{-1.0_p, 0.0_p, 0.0_p}, 0.5_p);
        // Create plane with identity matrix
        auto identity_matrix = geometry::matrix::identity(3, 3);
        plane1 = std::make_unique<plane>(point{0.0_p, -2.0_p, 0.0_p}, identity_matrix);
    }

    point origin;
    std::unique_ptr<sphere> sphere1;
    std::unique_ptr<sphere> sphere2;
    std::unique_ptr<plane> plane1;
};

TEST_F(GroupTest, Constructor) {
    point group_position{5.0_p, 3.0_p, -2.0_p};
    group test_group(group_position);

    EXPECT_POINT_EQ(test_group.position(), group_position);
    EXPECT_TRUE(test_group.get_objects().empty());
}

TEST_F(GroupTest, AddObject) {
    group test_group(origin);

    // Test adding objects
    test_group.add_object(sphere1.get());
    test_group.add_object(sphere2.get());
    test_group.add_object(plane1.get());

    auto const& objects = test_group.get_objects();
    EXPECT_EQ(objects.size(), 3);
    EXPECT_EQ(objects[0], sphere1.get());
    EXPECT_EQ(objects[1], sphere2.get());
    EXPECT_EQ(objects[2], plane1.get());

    // Test adding null pointer
    test_group.add_object(nullptr);
    EXPECT_EQ(objects.size(), 3);  // Should not change
}

TEST_F(GroupTest, AddObjectPreservesPosition) {
    group test_group(origin);
    point original_sphere_pos = sphere1->position();

    test_group.add_object(sphere1.get());

    // Object position should remain unchanged after adding to group
    EXPECT_POINT_EQ(sphere1->position(), original_sphere_pos);
}

TEST_F(GroupTest, MoveBy) {
    group test_group(origin);
    test_group.add_object(sphere1.get());
    test_group.add_object(sphere2.get());

    vector offset{2.0_p, 3.0_p, -1.0_p};
    point expected_group_pos = (origin + offset);
    point expected_sphere1_pos = (point{1.0_p, 0.0_p, 0.0_p} + offset);
    point expected_sphere2_pos = (point{-1.0_p, 0.0_p, 0.0_p} + offset);

    test_group.move_by(offset);

    EXPECT_POINT_EQ(test_group.position(), expected_group_pos);
    EXPECT_POINT_EQ(sphere1->position(), expected_sphere1_pos);
    EXPECT_POINT_EQ(sphere2->position(), expected_sphere2_pos);
}

TEST_F(GroupTest, Position) {
    group test_group(origin);
    test_group.add_object(sphere1.get());
    test_group.add_object(sphere2.get());

    point original_sphere1_pos = sphere1->position();
    point original_sphere2_pos = sphere2->position();

    point new_group_position{5.0_p, -2.0_p, 3.0_p};
    vector expected_offset = new_group_position - origin;

    test_group.position(new_group_position);

    EXPECT_POINT_EQ(test_group.position(), new_group_position);
    EXPECT_POINT_EQ(sphere1->position(), (original_sphere1_pos + expected_offset));
    EXPECT_POINT_EQ(sphere2->position(), (original_sphere2_pos + expected_offset));
}

TEST_F(GroupTest, Scale) {
    group test_group(origin);
    test_group.add_object(sphere1.get());
    test_group.add_object(sphere2.get());

    precision scale_x = 2.0_p;
    precision scale_y = 1.5_p;
    precision scale_z = 0.5_p;

    // Expected positions after scaling from origin
    point expected_sphere1_pos{2.0_p, 0.0_p, 0.0_p};   // (1,0,0) * (2,1.5,0.5)
    point expected_sphere2_pos{-2.0_p, 0.0_p, 0.0_p};  // (-1,0,0) * (2,1.5,0.5)

    test_group.scale(scale_x, scale_y, scale_z);

    EXPECT_POINT_EQ(sphere1->position(), expected_sphere1_pos);
    EXPECT_POINT_EQ(sphere2->position(), expected_sphere2_pos);

    // Group position should remain at origin
    EXPECT_POINT_EQ(test_group.position(), origin);
}

TEST_F(GroupTest, ScaleWithNonZeroGroupOrigin) {
    point group_position{1.0_p, 1.0_p, 1.0_p};
    group test_group(group_position);
    test_group.add_object(sphere1.get());

    precision scale_factor = 2.0_p;

    // Sphere1 is at (1,0,0), group is at (1,1,1)
    // Offset from group: (0,-1,-1)
    // Scaled offset: (0,-2,-2)
    // New position: (1,1,1) + (0,-2,-2) = (1,-1,-1)
    point expected_sphere_pos{1.0_p, -1.0_p, -1.0_p};

    test_group.scale(scale_factor, scale_factor, scale_factor);

    EXPECT_POINT_EQ(sphere1->position(), expected_sphere_pos);
    EXPECT_POINT_EQ(test_group.position(), group_position);
}

TEST_F(GroupTest, Rotation) {
    group test_group(origin);
    test_group.add_object(sphere1.get());
    test_group.add_object(sphere2.get());

    // Rotate 90 degrees around Z-axis
    iso::radians angle_x{0.0};
    iso::radians angle_y{0.0};
    iso::radians angle_z{iso::pi / 2.0};

    test_group.rotation(angle_x, angle_y, angle_z);

    // After 90-degree Z rotation around origin:
    // sphere1 (1,0,0) -> (0,1,0)
    // sphere2 (-1,0,0) -> (0,-1,0)
    point expected_sphere1_pos{0.0_p, 1.0_p, 0.0_p};
    point expected_sphere2_pos{0.0_p, -1.0_p, 0.0_p};

    EXPECT_POINT_EQ(sphere1->position(), expected_sphere1_pos);
    EXPECT_POINT_EQ(sphere2->position(), expected_sphere2_pos);
}

TEST_F(GroupTest, RotationWithNonZeroGroupOrigin) {
    point group_position{2.0_p, 2.0_p, 0.0_p};
    group test_group(group_position);
    test_group.add_object(sphere1.get());

    // Rotate 90 degrees around Z-axis
    iso::radians angle_x{0.0};
    iso::radians angle_y{0.0};
    iso::radians angle_z{iso::pi / 2.0};

    test_group.rotation(angle_x, angle_y, angle_z);

    // sphere1 at (1,0,0), group at (2,2,0)
    // Offset from group: (1,0,0) - (2,2,0) = (-1,-2,0)
    // After 90° Z rotation: (-1,-2,0) -> (2,-1,0)
    // New position: (2,2,0) + (2,-1,0) = (4,1,0)
    point expected_sphere_pos{4.0_p, 1.0_p, 0.0_p};

    EXPECT_POINT_EQ(sphere1->position(), expected_sphere_pos);
}

TEST_F(GroupTest, EmptyGroup) {
    group empty_group(origin);

    // Test operations on empty group
    vector offset{1.0_p, 2.0_p, 3.0_p};
    empty_group.move_by(offset);
    EXPECT_POINT_EQ(empty_group.position(), (origin + offset));

    empty_group.scale(2.0_p, 2.0_p, 2.0_p);
    EXPECT_POINT_EQ(empty_group.position(), (origin + offset));

    iso::radians angle{iso::pi / 4.0};
    empty_group.rotation(angle, angle, angle);
    // Empty group should handle rotation without crashing
}

TEST_F(GroupTest, CombinedTransformations) {
    group test_group(origin);
    test_group.add_object(sphere1.get());

    point const original_pos = sphere1->position();

    // Apply multiple transformations in sequence
    test_group.move_by(vector{1.0_p, 0.0_p, 0.0_p});
    test_group.scale(2.0_p, 1.0_p, 1.0_p);
    test_group.rotation(iso::radians{0.0}, iso::radians{0.0}, iso::radians{iso::pi / 2.0});

    // Verify transformation occurred
    point const final_pos = sphere1->position();
    bool position_changed =
        (final_pos.x != original_pos.x) || (final_pos.y != original_pos.y) || (final_pos.z != original_pos.z);
    EXPECT_TRUE(position_changed) << "Expected sphere to move from original position";
}

TEST_F(GroupTest, GetObjectsConst) {
    group test_group(origin);
    test_group.add_object(sphere1.get());
    test_group.add_object(sphere2.get());

    auto const& const_group = test_group;
    auto const& objects = const_group.get_objects();

    EXPECT_EQ(objects.size(), 2);
    EXPECT_EQ(objects[0], sphere1.get());
    EXPECT_EQ(objects[1], sphere2.get());
}

}  // namespace test
}  // namespace objects
}  // namespace raytrace