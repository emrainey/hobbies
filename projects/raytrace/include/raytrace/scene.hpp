#pragma once

#include <basal/printable.hpp>
#include <vector>

#include "raytrace/camera.hpp"
#include "raytrace/color.hpp"
#include "raytrace/image.hpp"
#include "raytrace/lights/light.hpp"
#include "raytrace/mediums/transparent.hpp"
#include "raytrace/objects/object.hpp"

namespace raytrace {

/// A functor type that can return a color given the world ray
using background_mapper = std::function<color(const ray&)>;

/// A Class to hold all the components needed to render a scene
class scene : public basal::printable {
public:
    using object_list = std::vector<const objects::object*>;
    using light_list = std::vector<const lights::light*>;
    using intersect_list = std::vector<geometry::intersection>;

    /// A complete set of information about the intersection information relative to an
/// unmentioned point.
    struct intersect_set {
        /// Simple Constructor
        explicit intersect_set(precision d, const geometry::intersection& i, const objects::object* o);
        // Default Copy
        explicit intersect_set(const intersect_set&) = default;
        // No move construct
        intersect_set(intersect_set&&) = delete;
        // Default Copy Assign
        intersect_set& operator=(const intersect_set&) = default;
        // No move assign
        intersect_set& operator=(intersect_set&&) = delete;
        // Default destructor
        virtual ~intersect_set() = default;

        /// The distance to the object
        precision distance;
        /// The intersection information
        geometry::intersection intersector;
        /// The pointer to the const object of *closest* intersection
        const objects::object* objptr;
    };

    /// Constructor
    /// @param adaptive_threshold
    ///
    scene(precision adaptive_threshold = 1.0_p / 32.0_p);

    /// Destructor
    virtual ~scene();

    ///
    /// Finds the intersection with each item in the objects list.
    /// The list of intersections is the same length as the object list.
    /// @param world_ray [in] The ray to intersect with.
    /// @param objects [in] The list of objects in the scene.
    /// @return
    ///
    static intersect_list find_intersections(const ray& world_ray, const object_list& objects);

    ///
    /// Given a set of intersections, finds the closest intersection to a point.
    /// @param [in] world_ray The point of the ray is the point to judge the distance from.
    /// @param [in] intersections The list of intersections with the objects list.
    /// @param [in] objects The list of objects in the scene.
    /// @return Returns the pointer to the nearest object to the point.
    ///
    static intersect_set nearest_object(const ray& world_ray, const intersect_list& intersections, const object_list&);

    ///
    /// Traces the path of a world ray within the scene and returns the color.
    /// @param world_ray The ray in world coordinates to trace.
    /// @param refractive_index The current medium's refractive index. Most of time this will be air (~1.0_p)
    /// @param reflection_depth The current recursive depth of reflections.
    /// @param recursive_contribution The amount of contribution from this level of recursion to the top level color.
    ///                               When it falls below a global limit, the reflection will not be considered.
    ///                               @see adaptive_reflection_threshold
    ///
    color trace(const ray& world_ray, const mediums::medium& media, size_t depth = 1,
                precision recursive_contribution = 1.0_p);

    ///
    /// Computes the scene and saves the camera image to a file
    /// @param views The camera view to render to from.
    /// @param filename The name of the file to write
    /// @param number_of_samples The number of subsamples per pixel.
    /// @param reflection_depth The depth of recursion for a traced ray. 1 equals no reflections.
    /// @param func     The optional callback per pixel
    /// @param show_bar Enables showing the progress bar
    ///
    void render(camera& view, std::string filename, size_t number_of_samples = 1, size_t reflection_depth = 1,
                std::optional<image::rendered_line> func = std::nullopt,
                uint8_t mask_threshold = raytrace::image::AAA_MASK_DISABLED, bool filter_capture = false);

    /// The limit for reflective contributions to the top level trace.
    precision adaptive_reflection_threshold;

    /// Allows the user to set a functor whioh returns the background color
    void set_background_mapper(background_mapper bgm);

    /// Adds an object to the scene
    void add_object(const objects::object* obj);

    /// Adds a light to the scene
    void add_light(const lights::light* lit);

    /// Adds a media which the entire scene is set into
    void add_media(const mediums::medium* media);

    /// Removes all objects and lights from a scene
    void clear();

    /// @copydoc basal::printable::print
    void print(const char str[]) const override;

    /// Returns the number of objects in the scene
    size_t number_of_objects(void) const;

    /// Returns the number of lights in the scene
    size_t number_of_lights(void) const;

protected:
    /// The list of objects in the scene.
    object_list m_objects;

    /// The list of lights in the scene
    light_list m_lights;

    /// The Background mapper
    background_mapper m_background;

    /// The media which the scene starts in
    const mediums::medium* m_media;
};

}  // namespace raytrace