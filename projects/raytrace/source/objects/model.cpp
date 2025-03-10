#include <raytrace/objects/model.hpp>

namespace raytrace {
namespace objects {

Model::Model()
    : object{R3::origin, SIZE_T_MAX, false}
    , points_{}
    , normals_{}
    , texels_{}
    , faces_{}
    , parser_{*this}
    , loaded_{false} {
}

void Model::print(char const name[]) const {
    printf("Model %s has %zu points, %zu normals, %zu triangles\n", name, points_.size(), normals_.size(),
           faces_.size());
}

std::ostream& Model::operator<<(std::ostream& os) const {
    return os << "Model has " << points_.size() << " points, " << normals_.size() << " normals, " << texels_.size()
              << " texels, " << faces_.size() << " faces";
}

bool Model::is_surface_point(raytrace::point const& world_point) const {
    raytrace::point object_point = reverse_transform(world_point);
    for (auto const& f : faces_) {
        if (f.is_surface_point(object_point)) {
            return true;
        }
    }
    return false;
}

Model::hits Model::collisions_along(ray const& object_ray) const {
    hits hits;
    for (auto const& face : faces_) {
        // transform to each face's position
        auto face_ray = face.reverse_transform(object_ray);
        auto subhits = face.collisions_along(face_ray);
        for (auto& h : subhits) {
            auto face_point = as_point(h.intersect);
            h.intersect = face.forward_transform(face_point);
            h.normal = face.forward_transform(h.normal);
        }
        hits.insert(hits.end(), subhits.begin(), subhits.end());
    }
    return hits;
}

image::point Model::map(point const&) const {
    // object_surface_point -> image_point through the UV map
    return image::point{0, 0};
}

precision Model::get_object_extent(void) const {
    precision max_extent = 0.0_p;
    for (auto const& point : points_) {
        auto vec = point - R3::origin;
        precision extent = vec.magnitude();
        if (extent > max_extent) {
            max_extent = extent;
        }
    }
    if constexpr (debug) {
        printf("Model: Returning %lf for max extent\r\n", max_extent);
    }
    return max_extent;
}

void Model::addVertex(float x, float y, float z) {
    if constexpr (debug) {
        printf("Model: Adding vertex %f %f %f\n", x, y, z);
    }
    points_.emplace_back(x, y, z);
}

void Model::addNormal(float dx, float dy, float dz) {
    if constexpr (debug) {
        printf("Model: Adding vector %f %f %f\n", dx, dy, dz);
    }
    raytrace::vector normal{dx, dy, dz};
    normals_.emplace_back(normal);
}

void Model::addTexture(float u, float v) {
    if constexpr (debug) {
        printf("Model: Adding texture %f %f\n", u, v);
    }
    texels_.emplace_back(u, v);
}

void Model::addFace(uint32_t a, uint32_t b, uint32_t c) {
    uint32_t ia = a - 1;
    uint32_t ib = b - 1;
    uint32_t ic = c - 1;
    bool points_ok = ia < points_.size() and ib < points_.size() and ic < points_.size();
    if (points_ok) {
        if constexpr (debug) {
            printf("Model: Adding triangle %u %u %u\n", a, b, c);
        }
        // swap the order of points so that the normal is facing the right way
        faces_.emplace_back(points_[ic], points_[ib], points_[ia]);
    } else {
        if constexpr (debug) {
            printf("Model: Index out of bounds! %" PRIu32 " %" PRIu32 " %" PRIu32 "\n", a, b, c);
        }
    }
}

void Model::addFace(uint32_t a, uint32_t ta, uint32_t b, uint32_t tb, uint32_t c, uint32_t tc) {
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
            printf("Model: Adding triangle (%u %u %u), (%u %u %u)\n", a, b, c, ta, tb, tc);
        }
        // swap the order of points so that the normal is facing the right way
        faces_.emplace_back(points_[ic], points_[ib], points_[ia], texels_[ita], texels_[itb], texels_[itc]);
    } else {
        if constexpr (debug) {
            printf("Model: Index out of bounds! %" PRIu32 " %" PRIu32 " %" PRIu32 "\n", a, b, c);
        }
    }
}

void Model::addFace(uint32_t v1, uint32_t t1, uint32_t n1, uint32_t v2, uint32_t t2, uint32_t n2, uint32_t v3,
                    uint32_t t3, uint32_t n3) {
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
            printf("Model: Adding triangle (%u, %u, %u), (%u, %u, %u), (%u, %u, %u)\n", v1, v2, v3, t1, t2, t3, n1, n2,
                   n3);
        }
        // swap the order of points so that the normal is facing the right way
        faces_.emplace_back(points_[iv3], points_[iv2], points_[iv1], texels_[it1], texels_[it2], texels_[it3],
                            normals_[in1], normals_[in2], normals_[in3]);
    } else {
        if constexpr (debug) {
            printf("Model: Index out of bounds! %" PRIu32 " %" PRIu32 " %" PRIu32 "\n", v1, v2, v3);
        }
    }
}

size_t Model::GetNumberOfFaces(void) const {
    return faces_.size();
}

void Model::LoadFromFile(char const* const filename) {
    if (not loaded_) {
        FILE* file = fopen(filename, "r");
        throw_exception_unless(file != nullptr, "File %s was not found!", filename);
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), file)) {
            parser_.Parse(buffer);
        }
        fclose(file);
        loaded_ = true;
        // compute the center of the model
        vector computed_centroid;
        for (auto& pnt : points_) {
            vector v = pnt - R3::origin;
            if constexpr (debug) {
                std::cout << "Point Offset " << v << std::endl;
            }
            computed_centroid += v;
        }
        computed_centroid /= static_cast<precision>(points_.size());
        if constexpr (debug) {
            std::cout << "Computed Centroid of Model: " << computed_centroid << std::endl;
        }
        // adjust each face position now to be relative to the center
        for (auto& face : faces_) {
            face.print("Face");
            auto _old = face.position();
            auto _new = (_old - computed_centroid);
            if constexpr (debug) {
                std::cout << "Old Position " << _old << " New Position " << _new << std::endl;
            }
            face.position(_new);
        }
        position(R3::origin + computed_centroid);  // this is the center of the model, not each face
    }
}

void Model::LoadFromString(char const* const literal) {
    if (not loaded_) {
        parser_.Parse(literal);
        loaded_ = true;
    }
}

obj::Parser::Statistics const& Model::GetStatistics() const {
    return parser_.GetStatistics();
}

vector Model::normal_(point const& object_surface_point) const {
    vector object_surface_normal{0.0_p, 0.0_p, 0.0_p};
    for (auto const& f : faces_) {
        point face_surface_point = f.reverse_transform(object_surface_point);
        if (f.is_contained(face_surface_point)) {
            object_surface_normal = f.normal(face_surface_point);
        }
    }
    return entity::forward_transform(object_surface_normal);
}

}  // namespace objects
}  // namespace raytrace
