#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <raytrace/raytrace.hpp>
#include <raytrace/objparser.hpp>
#include <raytrace/objects/model.hpp>

#include "raytrace/gtest_helper.hpp"

using namespace raytrace;

class MockObserver : public obj::Observer {
public:
    MOCK_METHOD3(addVertex, void(float, float, float));
    MOCK_METHOD3(addNormal, void(float, float, float));
    MOCK_METHOD2(addTexture, void(float, float));
    MOCK_METHOD3(addFace, void(uint32_t, uint32_t, uint32_t));
    MOCK_METHOD6(addFace, void(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t));
    MOCK_METHOD9(addFace,
                 void(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t));
};

TEST(ObjParser, Comment) {
    char const* const literal = "# Comment";
    MockObserver mock;
    obj::Parser parser{mock};
    parser.Parse(literal);
    // No expected calls
    ASSERT_EQ(obj::Parser::State::Comment, parser.GetState());
    ASSERT_EQ(obj::Parser::SubState::String, parser.GetSubState());
    ASSERT_EQ(1u, parser.GetNumberOfLines());
    parser.Parse('\n');
    ASSERT_EQ(obj::Parser::State::Unknown, parser.GetState());
    ASSERT_EQ(obj::Parser::SubState::None, parser.GetSubState());
    ASSERT_EQ(2u, parser.GetNumberOfLines());
}

TEST(ObjParser, Object) {
    char const* const literal = "o Something";
    MockObserver mock;
    obj::Parser parser{mock};
    parser.Parse(literal);
    // No expected calls
    ASSERT_EQ(obj::Parser::State::Object, parser.GetState());
    ASSERT_EQ(obj::Parser::SubState::String, parser.GetSubState());
    ASSERT_EQ(1u, parser.GetNumberOfLines());
    parser.Parse('\n');
    ASSERT_EQ(obj::Parser::State::Unknown, parser.GetState());
    ASSERT_EQ(obj::Parser::SubState::None, parser.GetSubState());
    ASSERT_EQ(2u, parser.GetNumberOfLines());
}

TEST(ObjParser, Vertices2) {
    char const* const literal = "v 1.2 3.4";
    MockObserver mock;
    obj::Parser parser{mock};
    parser.Parse(literal);
    EXPECT_CALL(mock, addVertex(1.2_p, 3.4_p, 5.6_p)).Times(0);
    ASSERT_EQ(obj::Parser::State::Vertices, parser.GetState());
    ASSERT_EQ(obj::Parser::SubState::FloatingPoint, parser.GetSubState());
    ASSERT_EQ(1u, parser.GetNumberOfLines());
    parser.Parse('\n');
    ASSERT_EQ(obj::Parser::State::Unknown, parser.GetState());
    ASSERT_EQ(obj::Parser::SubState::None, parser.GetSubState());
    ASSERT_EQ(2u, parser.GetNumberOfLines());
}

TEST(ObjParser, Vertices3) {
    char const* const literal = "v 1.2 3.4 5.6";
    MockObserver mock;
    obj::Parser parser{mock};
    parser.Parse(literal);
    EXPECT_CALL(mock, addVertex(1.2_p, 3.4_p, 5.6_p));
    ASSERT_EQ(obj::Parser::State::Vertices, parser.GetState());
    ASSERT_EQ(obj::Parser::SubState::FloatingPoint, parser.GetSubState());
    ASSERT_EQ(1u, parser.GetNumberOfLines());
    parser.Parse('\n');
    ASSERT_EQ(obj::Parser::State::Unknown, parser.GetState());
    ASSERT_EQ(obj::Parser::SubState::None, parser.GetSubState());
    ASSERT_EQ(2u, parser.GetNumberOfLines());
}

TEST(ObjParser, Normals) {
    char const* const literal = "vn 1.2 3.4 5.6";
    MockObserver mock;
    obj::Parser parser{mock};
    parser.Parse(literal);
    EXPECT_CALL(mock, addNormal(1.2_p, 3.4_p, 5.6_p));
    ASSERT_EQ(obj::Parser::State::Normals, parser.GetState());
    ASSERT_EQ(obj::Parser::SubState::FloatingPoint, parser.GetSubState());
    ASSERT_EQ(1u, parser.GetNumberOfLines());
    parser.Parse('\n');
    ASSERT_EQ(obj::Parser::State::Unknown, parser.GetState());
    ASSERT_EQ(obj::Parser::SubState::None, parser.GetSubState());
    ASSERT_EQ(2u, parser.GetNumberOfLines());
}

TEST(ObjParser, Faces) {
    char const* const literal = "f 1 2 3";
    MockObserver mock;
    obj::Parser parser{mock};
    parser.Parse(literal);
    EXPECT_CALL(mock, addFace(1, 2, 3));
    ASSERT_EQ(obj::Parser::State::Faces, parser.GetState());
    ASSERT_EQ(obj::Parser::SubState::Integer, parser.GetSubState());
    ASSERT_EQ(1u, parser.GetNumberOfLines());
    parser.Parse('\n');
    ASSERT_EQ(obj::Parser::State::Unknown, parser.GetState());
    ASSERT_EQ(obj::Parser::SubState::None, parser.GetSubState());
    ASSERT_EQ(2u, parser.GetNumberOfLines());
}

char const* const literal
    = "# comment\n"
      "o Object\n"
      "v 3.0 2.0 1.0\n"
      "v 5.0 4.0 6.0\n"
      "v 7.0 8.0 9.0\n"
      "vt 0.0 0.0\n"
      "vn 0.0 0.0 1.0\n"
      "f 1 2 3\n"
      "";

TEST(ObjParser, SingleTriangle) {
    MockObserver mock;
    obj::Parser parser{mock};
    EXPECT_CALL(mock, addVertex(3.0_p, 2.0_p, 1.0_p)).Times(1);
    EXPECT_CALL(mock, addVertex(5.0_p, 4.0_p, 6.0_p)).Times(1);
    EXPECT_CALL(mock, addVertex(7.0_p, 8.0_p, 9.0_p)).Times(1);
    EXPECT_CALL(mock, addNormal(0.0_p, 0.0_p, 1.0_p)).Times(1);
    // TODO add texture at some point
    EXPECT_CALL(mock, addFace(1, 2, 3)).Times(1);
    parser.Parse(literal);
    ASSERT_EQ(9u, parser.GetNumberOfLines());
    ASSERT_EQ(3u, parser.GetStatistics().vertices);
    ASSERT_EQ(1u, parser.GetStatistics().textures);
    ASSERT_EQ(1u, parser.GetStatistics().normals);
    ASSERT_EQ(1u, parser.GetStatistics().faces);
}

TEST(ObjParser, SingleTriangleModel) {
    objects::Model model;
    model.LoadFromString(literal);
    ASSERT_EQ(3u, model.GetStatistics().vertices);
    ASSERT_EQ(1u, model.GetStatistics().textures);
    ASSERT_EQ(1u, model.GetStatistics().normals);
    ASSERT_EQ(1u, model.GetStatistics().faces);
    ASSERT_EQ(1u, model.GetNumberOfFaces());
}

TEST(ObjParser, CubeModelFromString) {
    objects::Model model;
    char const* const literal
        = "o Cube\n"
          "v 1.000000 -1.000000 -1.000000\n"
          "v 1.000000 -1.000000 1.000000\n"
          "v -1.000000 -1.000000 1.000000\n"
          "v -1.000000 -1.000000 -1.000000\n"
          "v 1.000000 1.000000 -0.999999\n"
          "v 0.999999 1.000000 1.000001\n"
          "v -1.000000 1.000000 1.000000\n"
          "v -1.000000 1.000000 -1.000000\n"
          "vt 1.000000 0.333333\n"
          "vt 1.000000 0.666667\n"
          "vt 0.666667 0.666667\n"
          "vt 0.666667 0.333333\n"
          "vt 0.666667 0.000000\n"
          "vt 0.000000 0.333333\n"
          "vt 0.000000 0.000000\n"
          "vt 0.333333 0.000000\n"
          "vt 0.333333 1.000000\n"
          "vt 0.000000 1.000000\n"
          "vt 0.000000 0.666667\n"
          "vt 0.333333 0.333333\n"
          "vt 0.333333 0.666667\n"
          "vt 1.000000 0.000000\n"
          "vn 0.000000 -1.000000 0.000000\n"
          "vn 0.000000 1.000000 0.000000\n"
          "vn 1.000000 0.000000 0.000000\n"
          "vn -0.000000 0.000000 1.000000\n"
          "vn -1.000000 -0.000000 -0.000000\n"
          "vn 0.000000 0.000000 -1.000000\n"
          "f 2/1/1 3/2/1 4/3/1\n"
          "f 8/1/2 7/4/2 6/5/2\n"
          "f 5/6/3 6/7/3 2/8/3\n"
          "f 6/8/4 7/5/4 3/4/4\n"
          "f 3/9/5 7/10/5 8/11/5\n"
          "f 1/12/6 4/13/6 8/11/6\n"
          "f 1/4/1 2/1/1 4/3/1\n"
          "f 5/14/2 8/1/2 6/5/2\n"
          "f 1/12/3 5/6/3 2/8/3\n"
          "f 2/12/4 6/8/4 3/4/4\n"
          "f 4/13/5 3/9/5 8/11/5\n"
          "f 5/6/6 1/12/6 8/11/6\n"
          "";
    model.LoadFromString(literal);
    ASSERT_EQ(8u, model.GetStatistics().vertices);
    ASSERT_EQ(14u, model.GetStatistics().textures);
    ASSERT_EQ(6u, model.GetStatistics().normals);
    ASSERT_EQ(12u, model.GetStatistics().faces);
    ASSERT_EQ(12u, model.GetNumberOfFaces());
}

TEST(ObjParser, CubeModelFromFile) {
    objects::Model model;
    printf("The test has to run from the root ${workspaceFolder}/testing of hobbies\r\n");
    model.LoadFromFile("../projects/raytrace/test/cube.obj");
    ASSERT_EQ(8u, model.GetStatistics().vertices);
    ASSERT_EQ(14u, model.GetStatistics().textures);
    ASSERT_EQ(6u, model.GetStatistics().normals);
    ASSERT_EQ(12u, model.GetStatistics().faces);
    ASSERT_EQ(12u, model.GetNumberOfFaces());
}

TEST(ObjParser, DISABLED_CubeModelAsObject) {
    objects::Model model;
    printf("The test has to run from the root ${workspaceFolder}/testing of hobbies\r\n");
    model.LoadFromFile("../projects/raytrace/test/cube.obj");
    EXPECT_POINT_EQ(R3::origin, model.position());
    // move by enough to not be able to hit the original box
    model.move_by(raytrace::vector{10, 10, 10});

    EXPECT_POINT_EQ(raytrace::point(10, 10, 10), model.position());

    // show that some rays don't work
    {
        raytrace::ray r0{raytrace::point(0.0_p, -2.0_p, 0.0_p), R3::basis::Y};
        auto hit = model.intersect(r0);
        EXPECT_EQ(geometry::IntersectionType::None, get_type(hit.intersect));
    }

    // shoot some rays at it to see if the transform works
    {
        raytrace::ray r0{raytrace::point{10.5_p, 8.0_p, 10.5_p}, R3::basis::Y};
        auto hit = model.intersect(r0);
        // FIXME these are failing, probably due to assuming world or object space
        EXPECT_EQ(geometry::IntersectionType::Point, get_type(hit.intersect));
        EXPECT_POINT_EQ(raytrace::point(10.5_p, 9.0_p, 10.5_p), as_point(hit.intersect));
        EXPECT_VECTOR_EQ(raytrace::vector({0, -1, 0}), hit.normal);
    }
}