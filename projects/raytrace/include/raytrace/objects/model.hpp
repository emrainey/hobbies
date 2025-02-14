#pragma once

#include <raytrace/objects/object.hpp>
#include <raytrace/objects/face.hpp>
#include <raytrace/objparser.hpp>

namespace raytrace {
namespace objects {

class Model : public object, protected obj::Observer {
public:
    Model() : points_{}, normals_{}, texels_{}, faces_{}, parser_{*this}, loaded_{false} {}
    virtual ~Model() = default;

    void print(char const name[]) const override {
        printf("Model %s has %zu points, %zu normals, %zu triangles\n", name, points_.size(), normals_.size(), faces_.size());
    }

    std::ostream& operator<<(std::ostream& os) const {
        return os << "Model has " << points_.size() << " points, " << normals_.size() << " normals, " << texels_.size() << " texels, " << faces_.size() << " faces";
    }

    bool is_surface_point(raytrace::point const& world_point) const override {
        for (auto const& f : faces_) {
            if (f.is_surface_point(world_point)) {
                return true;
            }
        }
        return false;
    }

    hits collisions_along(ray const& object_ray) const override {
        hits hits;
        for (auto const& f : faces_) {
            auto subhits = f.collisions_along(object_ray);
            hits.insert(hits.end(), subhits.begin(), subhits.end());
        }
        return hits;
    }

    image::point map(point const& object_surface_point) const override {
        return image::point{0, 0};
    }

    precision get_object_extent(void) const override {
        // find the point that is the furthest from the center
        precision max_extent = 0.0_p;
        for (auto const& point : points_) {
            auto vec = point - R3::origin;
            precision extent = vec.quadrance(); // r^2 is ok for this
            if (extent > max_extent) {
                max_extent = extent;
            }
        }
        return max_extent;
    }

    void addVertex(float x, float y, float z) override {
        if constexpr (debug) {
            printf("Adding vertex %f %f %f\n", x, y, z);
        }
        points_.emplace_back(x, y, z);
    }
    void addNormal(float dx, float dy, float dz) override {
        if constexpr (debug) {
            printf("Adding vector %f %f %f\n", dx, dy, dz);
        }
        raytrace::vector normal{dx, dy, dz};
        normals_.emplace_back(normal);
    }
    void addTexture(float u, float v) override {
        if constexpr (debug) {
            printf("Adding texture %f %f\n", u, v);
        }
        texels_.emplace_back(u, v);
    }
    void addFace(uint32_t a, uint32_t b, uint32_t c) override {
        uint32_t ia = a - 1;
        uint32_t ib = b - 1;
        uint32_t ic = c - 1;
        bool points_ok = ia < points_.size() and ib < points_.size() and ic < points_.size();
        if (ia < points_.size() and ib < points_.size() and ic < points_.size()) {
            if constexpr (debug) {
                printf("Adding triangle %u %u %u\n", a, b, c);
            }
            faces_.emplace_back(points_[ia], points_[ib], points_[ic]);
        } else {
            if constexpr (debug) {
                printf("Index out of bounds! %" PRIu32 " %" PRIu32 " %" PRIu32 "\n", a, b, c);
            }
        }
    }

    void addFace(uint32_t a, uint32_t ta, uint32_t b, uint32_t tb, uint32_t c, uint32_t tc) override {
        uint32_t ia = a - 1;
        uint32_t ib = b - 1;
        uint32_t ic = c - 1;
        bool points_ok = ia < points_.size() and ib < points_.size() and ic < points_.size();
        uint32_t ita = ta - 1;
        uint32_t itb = tb - 1;
        uint32_t itc = tc - 1;
        bool texels_ok = ita < texels_.size() and itb < texels_.size() and itc < texels_.size();
        if (points_ok and texels_ok) {
            if constexpr (debug) {
                printf("Adding triangle (%u %u %u), (%u %u %u)\n", a, b, c, ta, tb, tc);
            }
            faces_.emplace_back(points_[ia], points_[ib], points_[ic], texels_[ita], texels_[itb], texels_[itc]);
        } else {
            if constexpr (debug) {
                printf("Index out of bounds! %" PRIu32 " %" PRIu32 " %" PRIu32 "\n", a, b, c);
            }
        }
    }

    void addFace(uint32_t v1, uint32_t t1, uint32_t n1, uint32_t v2, uint32_t t2, uint32_t n2, uint32_t v3, uint32_t t3, uint32_t n3) override {
        uint32_t iv1 = v1 - 1;
        uint32_t iv2 = v2 - 1;
        uint32_t iv3 = v3 - 1;
        bool points_ok = iv1 < points_.size() and iv2 < points_.size() and iv3 < points_.size();
        uint32_t it1 = t1 - 1;
        uint32_t it2 = t2 - 1;
        uint32_t it3 = t3 - 1;
        bool texels_ok = it1 < texels_.size() and it2 < texels_.size() and it3 < texels_.size();
        uint32_t in1 = n1 - 1;
        uint32_t in2 = n2 - 1;
        uint32_t in3 = n3 - 1;
        bool normals_ok = in1 < normals_.size() and in2 < normals_.size() and in3 < normals_.size();
        if (points_ok and normals_ok and texels_ok) {
            if constexpr (debug) {
                printf("Adding triangle (%u %u %u), (%u, %u, %u), (%u, %u, %u)\n", v1, v2, v3, t1, t2, t3, n1, n2, n3);
            }
            faces_.emplace_back(points_[iv1], points_[iv2], points_[iv3], texels_[it1], texels_[it2], texels_[it3], normals_[in1], normals_[in2], normals_[in3]);
        } else {
            if constexpr (debug) {
                printf("Index out of bounds! %" PRIu32 " %" PRIu32 " %" PRIu32 "\n", v1, v2, v3);
            }
        }
    }

    size_t GetNumberOfFaces(void) const {
        return faces_.size();
    }

    void LoadFromFile(char const * const filename) {
        if (not loaded_) {
            FILE * file = fopen(filename, "r");
            throw_exception_unless(file != nullptr, "File %s was not found!", filename);
            char buffer[1024];
            while (fgets(buffer, sizeof(buffer), file)) {
                parser_.Parse(buffer);
            }
            fclose(file);
            loaded_ = true;
            // the center of the model is the average of all the points or is it just the origin in model space?
        }
    }

    void LoadFromString(char const * const literal) {
        if (not loaded_) {
            parser_.Parse(literal);
            loaded_ = true;
        }
    }

    obj::Parser::Statistics const& GetStatistics() const {
        return parser_.GetStatistics();
    }

protected:

    vector normal_(point const& object_surface_point) const override {
        vector object_surface_normal{0.0_p, 0.0_p, 0.0_p};
        // find the triangle this point is on the hard way
        for (auto const& f : faces_) {
            if (f.is_contained(object_surface_point)) {
                // the triangle's normal is in the composite object space, not world space, so it needs another transform
                object_surface_normal = f.normal(object_surface_point);
            }
        }
        return entity::forward_transform(object_surface_normal);
    }
    static constexpr bool debug = false;
    std::vector<raytrace::point> points_;
    std::vector<raytrace::vector> normals_;
    std::vector<image::point> texels_;
    std::vector<raytrace::objects::face> faces_;
    obj::Parser parser_;
    bool loaded_;
};

}   // namespace objects
}   // namespace raytrace
