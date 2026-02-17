#pragma once

/// @file
/// The Raytrace library scene header

#include <basal/printable.hpp>
#include <vector>

#include "raytrace/tree.hpp"
#include "raytrace/camera.hpp"
#include "raytrace/color.hpp"
#include "raytrace/image.hpp"
#include "raytrace/lights/light.hpp"
#include "raytrace/mediums/transparent.hpp"
#include "raytrace/objects/object.hpp"
#include "raytrace/objects/group.hpp"

namespace raytrace {

/// A functor type that can return a color given the world ray
using background_mapper = std::function<color(ray const&)>;

/// A Class to hold all the components needed to render a scene
class scene : public basal::printable {
public:
    using object_list = std::vector<objects::object const*>;
    using light_list = std::vector<lights::light const*>;

    /// Constructor
    /// @param adaptive_threshold
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
    objects::hits find_intersections(ray const& world_ray);

    ///
    /// Given a set of hits, finds the closest intersection to a point.
    /// @param [in] world_ray The point of the ray is the point to judge the distance from.
    /// @param [in] hits The list of hits with the objects list.
    /// @param [in] objects The list of objects in the scene.
    /// @return Returns the pointer to the nearest object to the point.
    ///
    objects::hit nearest_object(ray const& world_ray, objects::hits const& hits);

    ///
    /// Traces the path of a world ray within the scene and returns the color.
    /// @param world_ray The ray in world coordinates to trace.
    /// @param refractive_index The current medium's refractive index. Most of time this will be air (~1.0_p)
    /// @param reflection_depth The current recursive depth of reflections.
    /// @param recursive_contribution The amount of contribution from this level of recursion to the top level color.
    ///                               When it falls below a global limit, the reflection will not be considered.
    ///                               @see adaptive_reflection_threshold
    ///
    color trace(ray const& world_ray, mediums::medium const& media, size_t depth = 1,
                precision recursive_contribution = 1.0_p);

    /// The direct light from a single source of a single sample on a world point
    /// @param scene_light The light source in the scene
    /// @param medium The medium being lit
    /// @param world_surface_point The point on the surface in world coordinates
    /// @param object_surface_point The point on the object's surface in object coordinates
    /// @param world_surface_normal The normal vector on the surface at the point in world coordinates
    /// @param world_reflection The reflection ray in world coordinates
    /// @param sample_index The index of the sample for the light source
    /// @param reflection_depth The current recursive depth of reflections.
    /// @param recursive_contribution The amount of contribution from this level of recursion to the top level color.
    /// @return The color resulting from the direct light at the point
    color direct_light(lights::light const& scene_light, mediums::medium const& medium,
                       point const& world_surface_point, point const& object_surface_point,
                       vector const& world_surface_normal, ray const& world_reflection, size_t sample_index,
                       size_t reflection_depth, precision recursive_contribution);

    /// Computes the color from the emissive light at the point in the scene
    /// @param emissivity The emissivity of the medium at the point (0.0_p = non-emissive, 1.0_p = fully emissive)
    /// @param medium The medium being emitted from
    /// @param object_surface_point The point on the object's surface in world coordinates
    /// @return The color resulting from the emissive light at the point
    color emissive_light(precision emissivity, mediums::medium const& medium,
                         raytrace::point const& object_surface_point) const;

    /// Computes the reflected light color from the medium at the point in the scene
    /// @param reflectivity The reflectivity of the medium at the point (0.0_p = non-reflective, 1.0_p = fully
    /// reflective)
    /// @param medium The medium being reflected from
    /// @param media The current medium the ray is in
    /// @param world_surface_point The point on the object's surface in world coordinates
    /// @param object_surface_point The point on the object's surface in object coordinates
    /// @param world_surface_normal The normal vector on the surface at the point in world coordinates
    /// @param world_reflection The reflection ray in world coordinates
    /// @param reflection_depth The current recursive depth of reflections.
    /// @param recursive_contribution The amount of contribution from this level of recursion to the top level color.
    ///                               When it falls below a global limit, the reflection will not be considered.
    ///                               @see adaptive_reflection_threshold
    color reflected_light(precision reflectivity, mediums::medium const& medium, mediums::medium const& media,
                          point const& world_surface_point, point const& object_surface_point,
                          vector const& world_surface_normal, ray const& world_reflection, size_t reflection_depth,
                          precision recursive_contribution = 1.0_p);

    /// Computes the transmitted light color through the medium at the point in the scene
    /// @param transparency The transparency of the medium at the point (0.0_p = opaque, 1.0_p = fully transparent)
    /// @param medium The medium being transmitted through
    /// @param world_refraction The refraction ray in world coordinates
    /// @param reflection_depth The current recursive depth of reflections.
    /// @param recursive_contribution The amount of contribution from this level of recursion to the top level color.
    ///                               When it falls below a global limit, the transmission will not be considered.
    ///                               @see adaptive_reflection_threshold
    /// @return The color resulting from the transmitted light through the medium
    color transmitted_light(precision transparency, mediums::medium const& medium, ray const& world_refraction,
                            size_t reflection_depth, precision recursive_contribution = 1.0_p);

    ///
    /// Computes the scene and saves the camera image to a file
    /// @param views The camera view to render to from.
    /// @param filename The name of the file to write
    /// @param number_of_samples The number of subsamples per pixel.
    /// @param reflection_depth The depth of recursion for a traced ray. 1 equals no reflections.
    /// @param func     The optional callback per pixel (used for updating UIs)
    /// @param aaa_mask_threshold The threshold for the adaptive anti-aliasing mask. If the mask pixel is above the threshold value, then it
    ///                           will attempt to recompute. If 255, anti-aliasing is disabled. If 1, anti-aliasing is enabled for all pixels.
    /// @param filter_capture Whether to do a post-process filter on the capture before saving. This is separate from tone mapping as it is just a convolution filter, not a color mapping.
    /// @param tone_mapper Whether to apply tone mapping to the final image.
    void render(camera& view,
                std::string filename,
                size_t number_of_samples = 1,
                size_t reflection_depth = 1,
                std::optional<image::rendered_line> func = std::nullopt,
                uint8_t mask_threshold = raytrace::image::AAA_MASK_DISABLED,
                bool filter_capture = false,
                bool tone_mapper = false);

    /// The limit for reflective contributions to the top level trace.
    precision adaptive_reflection_threshold;

    /// Allows the user to set a functor which returns the background color
    void set_background_mapper(background_mapper bgm);

    /// Adds all the objects from a group to the scene
    void add_group(objects::group const* grp);

    /// Adds an object to the scene
    void add_object(objects::object const* obj);

    /// Adds a light to the scene
    void add_light(lights::light const* lit);

    /// Adds a media which the entire scene is set into
    void add_media(mediums::medium const* media);

    /// Removes all objects and lights from a scene
    void clear();

    // basal::printable
    void print(std::ostream& os, char const str[]) const override;

    friend std::ostream& operator<<(std::ostream& os, scene const& sc);

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
    mediums::medium const* m_media;

    /// The set of the finite bounds of the scene
    Bounds m_bounds;

    /// The infinite bounds object list
    object_list m_infinite_objects;

    /// The finite bounds object list sorted into nodes
    std::vector<tree::Node> m_nodes;
};

}  // namespace raytrace