#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <raytrace/raytrace.hpp>
#include <raytrace/objparser.hpp>

using namespace raytrace;


class MockObserver : public obj::Observer {
public:
    MOCK_METHOD3(addVertex, void(float, float, float));
    MOCK_METHOD3(addNormal, void(float, float, float));
    MOCK_METHOD3(addFace, void(uint32_t, uint32_t, uint32_t));
};

TEST(ObjParser, Comment) {
    char const * const literal = "# Comment";
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
    char const * const literal = "o Something";
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
    char const * const literal = "v 1.2 3.4";
    MockObserver mock;
    obj::Parser parser{mock};
    parser.Parse(literal);
    EXPECT_CALL(mock, addVertex(1.2f, 3.4f, 5.6f)).Times(0);
    ASSERT_EQ(obj::Parser::State::Vertices, parser.GetState());
    ASSERT_EQ(obj::Parser::SubState::FloatingPoint, parser.GetSubState());
    ASSERT_EQ(1u, parser.GetNumberOfLines());
    parser.Parse('\n');
    ASSERT_EQ(obj::Parser::State::Unknown, parser.GetState());
    ASSERT_EQ(obj::Parser::SubState::None, parser.GetSubState());
    ASSERT_EQ(2u, parser.GetNumberOfLines());
}

TEST(ObjParser, Vertices3) {
    char const * const literal = "v 1.2 3.4 5.6";
    MockObserver mock;
    obj::Parser parser{mock};
    parser.Parse(literal);
    EXPECT_CALL(mock, addVertex(1.2f, 3.4f, 5.6f));
    ASSERT_EQ(obj::Parser::State::Vertices, parser.GetState());
    ASSERT_EQ(obj::Parser::SubState::FloatingPoint, parser.GetSubState());
    ASSERT_EQ(1u, parser.GetNumberOfLines());
    parser.Parse('\n');
    ASSERT_EQ(obj::Parser::State::Unknown, parser.GetState());
    ASSERT_EQ(obj::Parser::SubState::None, parser.GetSubState());
    ASSERT_EQ(2u, parser.GetNumberOfLines());
}

TEST(ObjParser, Normals) {
    char const * const literal = "vn 1.2 3.4 5.6";
    MockObserver mock;
    obj::Parser parser{mock};
    parser.Parse(literal);
    EXPECT_CALL(mock, addNormal(1.2f, 3.4f, 5.6f));
    ASSERT_EQ(obj::Parser::State::Normals, parser.GetState());
    ASSERT_EQ(obj::Parser::SubState::FloatingPoint, parser.GetSubState());
    ASSERT_EQ(1u, parser.GetNumberOfLines());
    parser.Parse('\n');
    ASSERT_EQ(obj::Parser::State::Unknown, parser.GetState());
    ASSERT_EQ(obj::Parser::SubState::None, parser.GetSubState());
    ASSERT_EQ(2u, parser.GetNumberOfLines());
}

TEST(ObjParser, Faces) {
    char const * const literal = "f 1 2 3";
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

TEST(ObjParser, SingleTriangle) {
    char const * const literal =
        "# comment\n"
        "o Object\n"
        "v 1.0 2.0 3.0\n"
        "v 4.0 5.0 6.0\n"
        "v 7.0 8.0 9.0\n"
        "f 1 2 3\n";
    MockObserver mock;
    obj::Parser parser{mock};
    EXPECT_CALL(mock, addVertex(1.0, 2.0, 3.0)).Times(1);
    EXPECT_CALL(mock, addVertex(4.0, 5.0, 6.0)).Times(1);
    EXPECT_CALL(mock, addVertex(7.0, 8.0, 9.0)).Times(1);
    EXPECT_CALL(mock, addFace(1, 2, 3)).Times(1);
    parser.Parse(literal);
    ASSERT_EQ(7u, parser.GetNumberOfLines());
}

class TestObjParser : public ::testing::Test, public obj::Observer {
public:
    TestObjParser() : ::testing::Test(), parser{*this} {}

    void addVertex(float x, float y, float z) override {
        element_type a = x;
        element_type b = y;
        element_type c = z;
        raytrace::point p{a, b, c};
        points.emplace_back(p);
    }
    void addNormal(float dx, float dy, float dz) override {
        element_type a = dx;
        element_type b = dy;
        element_type c = dz;
        raytrace::vector v{a, b, c};
        normals.emplace_back(v);
    }
    void addFace(uint32_t a, uint32_t b, uint32_t c) override {
        if (a < points.size() and b < points.size() and c < points.size()) {
            triangles.emplace_back(points[a], points[b], points[c]);
        }
    }
    size_t GetNumberTriangles(void) const {
        return triangles.size();
    }
protected:
    std::vector<raytrace::point> points;
    std::vector<raytrace::vector> normals;
    std::vector<raytrace::objects::triangle> triangles;
    obj::Parser parser;
};

TEST_F(TestObjParser, SingleTriangle) {
    char const * const literal =
        "# comment\n"
        "o Object\n"
        "v 1.0 2.0 3.0\n"
        "v 4.0 5.0 6.0\n"
        "v 7.0 8.0 9.0\n"
        "f 1 2 3\n";
    parser.Parse(literal);
    ASSERT_EQ(7u, parser.GetNumberOfLines());
    ASSERT_EQ(1u, GetNumberTriangles());
}