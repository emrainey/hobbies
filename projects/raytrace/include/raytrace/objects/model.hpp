#pragma once

#include <raytrace/objects/object.hpp>
#include <raytrace/objects/face.hpp>
#include <raytrace/objparser.hpp>

namespace raytrace {
namespace objects {

/// @class Model
/// @brief Represents a 3D model object in the ray tracing system.

class Model
    : public object
    , protected obj::Observer {
public:
    /// @brief Default constructor for the Model class.
    Model();

    /// @brief Prints the model information.
    /// @param name The name to be printed.
    void print(char const name[]) const override;

    /// @brief Overloaded stream insertion operator for the Model class.
    /// @param os The output stream.
    /// @return The output stream with the model information.
    std::ostream& operator<<(std::ostream& os) const;

    /// @brief Adds a vertex to the model.
    /// @param x The x-coordinate of the vertex.
    /// @param y The y-coordinate of the vertex.
    /// @param z The z-coordinate of the vertex.
    void addVertex(float x, float y, float z) override;

    /// @brief Adds a normal vector to the model.
    /// @param dx The x-component of the normal vector.
    /// @param dy The y-component of the normal vector.
    /// @param dz The z-component of the normal vector.
    void addNormal(float dx, float dy, float dz) override;

    /// @brief Adds a texture coordinate to the model.
    /// @param u The u-coordinate of the texture.
    /// @param v The v-coordinate of the texture.
    void addTexture(float u, float v) override;

    /// @brief Adds a face to the model using vertex indices.
    /// @param a The index of the first vertex.
    /// @param b The index of the second vertex.
    /// @param c The index of the third vertex.
    void addFace(uint32_t a, uint32_t b, uint32_t c) override;

    /// @brief Adds a face to the model using vertex and texture indices.
    /// @param a The index of the first vertex.
    /// @param ta The index of the first texture coordinate.
    /// @param b The index of the second vertex.
    /// @param tb The index of the second texture coordinate.
    /// @param c The index of the third vertex.
    /// @param tc The index of the third texture coordinate.
    void addFace(uint32_t a, uint32_t ta, uint32_t b, uint32_t tb, uint32_t c, uint32_t tc) override;

    /// @brief Adds a face to the model using vertex, texture, and normal indices.
    /// @param v1 The index of the first vertex.
    /// @param t1 The index of the first texture coordinate.
    /// @param n1 The index of the first normal vector.
    /// @param v2 The index of the second vertex.
    /// @param t2 The index of the second texture coordinate.
    /// @param n2 The index of the second normal vector.
    /// @param v3 The index of the third vertex.
    /// @param t3 The index of the third texture coordinate.
    /// @param n3 The index of the third normal vector.
    void addFace(uint32_t v1, uint32_t t1, uint32_t n1, uint32_t v2, uint32_t t2, uint32_t n2, uint32_t v3, uint32_t t3,
                 uint32_t n3) override;

    /// @brief Gets the number of faces in the model.
    /// @return The number of faces.
    size_t GetNumberOfFaces(void) const;

    /// @brief Loads the model from a file.
    /// @param filename The path to the file.
    void LoadFromFile(char const* const filename);

    /// @brief Loads the model from a string literal.
    /// @param literal The string containing the model data.
    void LoadFromString(char const* const literal);

    /// @brief Gets the statistics of the model parser.
    /// @return The statistics of the parser.
    obj::Parser::Statistics const& GetStatistics() const;

    bool is_surface_point(raytrace::point const& world_point) const override;
    hits collisions_along(ray const& object_ray) const override;
    image::point map(point const& object_surface_point) const override;
    precision get_object_extent(void) const override;

protected:
    vector normal_(point const& object_surface_point) const override;
    std::vector<raytrace::point> points_;
    std::vector<raytrace::vector> normals_;
    std::vector<image::point> texels_;
    std::vector<raytrace::objects::face> faces_;
    obj::Parser parser_;
    bool loaded_;
};
}  // namespace objects
}  // namespace raytrace
