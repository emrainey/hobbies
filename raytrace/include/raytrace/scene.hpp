#pragma once

#include "raytrace/color.hpp"
#include "raytrace/image.hpp"
#include "raytrace/camera.hpp"
#include "raytrace/light.hpp"
#include "raytrace/object.hpp"
#include "raytrace/mediums/transparent.hpp"
#include <basal/printable.hpp>

namespace raytrace {

/** A functor type that can return a color given the world ray */
using background_mapper = std::function<color(const ray&)>;

/** A Class to hold all the components needed to render a scene */
class scene : public basal::printable {
public:
    using object_list = std::vector<const object*>;
    using light_list = std::vector<const light*>;
    using intersect_list = std::vector<geometry::intersection>;

    /**
     * A complete set of information about the intersection information relative to an
     * unmentioned point.
     */
    struct intersect_set {
        /** Simple Constructor */
        explicit intersect_set(element_type d, const geometry::intersection& i, const object *o);
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

        /** The distance to the object */
        element_type distance;
        /** The intersection information */
        geometry::intersection intersector;
        /** The pointer to the const object of *closest* intersection */
        const object* objptr;
    };

    /** Constructor */
    scene() = delete;

    /** Configures the camera and image projection */
    scene(size_t image_height, size_t image_width, iso::degrees field_of_view);

    /** Destructor */
    virtual ~scene();

    /**
     * Finds the intersection with each item in the objects list.
     * The list of intersections is the same length as the object list.
     * @param world_ray [in] The ray to intersect with.
     * @param objects [in] The list of objects in the scene.
     * @return
     */
    static intersect_list find_intersections(const ray& world_ray, const object_list& objects);

    /**
     * Given a set of intersections, finds the closest intersection to a point.
     * @param [in] world_ray The point of the ray is the point to judge the distance from.
     * @param [in] intersections The list of intersections with the objects list.
     * @param [in] objects The list of objects in the scene.
     * @return Returns the pointer to the nearest object to the point.
     */
    static intersect_set nearest_object(const ray& world_ray, const intersect_list& intersections, const object_list&);

    /**
     * Takes an object list and a light list and finds the blended color for that point from the
     * lights.
     * @param obj The reference to the object on which to find the color.
     * @param world_surface_point the point to consider in world space
     * @param world_surface_normal The normal on the object at that point in world space
     * @param object_surface_point The point to consider in object space
     * @param volumetric_point The volumetric point on the surface of the object.
     * @param reflection The vector at surface_point where the world_ray will bounce off to
     * @param objects The list of objects in the scene.
     * @param lights The list of lights in the scene.
     */
    static color find_surface_color(const object& obj,
                                    const point& world_surface_point,
                                    const vector& world_surface_normal,
                                    const point& object_surface_point,
                                    const raytrace::point& volumetric_point,
                                    const vector& reflection,
                                    const object_list& objects,
                                    const light_list& lights);

    /**
     * Traces the path of a world ray within the scene and returns the color.
     * @param world_ray The ray in world coordinates to trace.
     * @param refractive_index The current medium's refractive index. Most of time this will be air (~1.0)
     * @param reflection_depth The current recursive depth of reflections.
     */
    color trace(const ray& world_ray, const medium& media, size_t depth = 1);

    /**
     * Computes the scene and saves the camera image to a file
     * @param filename The name of the file to write
     * @param number_of_samples The number of subsamples per pixel.
     * @param reflection_depth The depth of recursion for a traced ray. 1 equals no reflections.
     * @param func     The optional callback per pixel
     * @param show_bar Enables showing the progress bar
     */
    void render(std::string filename,
                size_t number_of_samples = 1,
                size_t reflection_depth = 1,
                std::optional<image::rendered_line> func = std::nullopt,
                uint8_t mask_threshold = raytrace::image::AAA_MASK_DISABLED);

    /** The camera of the scene */
    camera view;

    /** Allows the user to set a functor whioh returns the background color */
    void set_background_mapper(background_mapper bgm);

    /** Adds an object to the scene */
    void add_object(const object *obj);

    /** Adds a light to the scene */
    void add_light(const light *lit);

    /** Removes all objects and lights from a scene */
    void clear();

    /** @copydoc basal::printable::print */
    void print(const char str[]) const override;

protected:
    /** The list of objects in the scene. */
    object_list m_objects;

    /** The list of lights in the scene */
    light_list m_lights;

    /** The Background mapper */
    background_mapper m_background;
};

}