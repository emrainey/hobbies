#include <tuple>
#include "raytrace/scene.hpp"

namespace raytrace {

scene::intersect_set::intersect_set(element_type d, const geometry::intersection& i, const object *o)
    : distance(d)
    , intersector(i)
    , objptr(o) {}

static constexpr bool debug = false;
static constexpr bool enforce_ranges = true;
/** uses fixed color scheme for shadows, light, near-zero values */
static constexpr bool debug_shadows_and_light = false;
static constexpr bool use_incident_scaling = true;
static constexpr bool use_specular_scaling = true;

// displays the distance to a light in shadow as a grayscale value (black is still shadow, white is very close)
static constexpr bool use_grayscale_distance = false;

scene::scene(size_t image_height, size_t image_width, iso::degrees field_of_view, double art)
    : view(image_height, image_width, field_of_view)
    , adaptive_reflection_threshhold(art)
    , m_objects()
    , m_lights()
    , m_background([](const raytrace::ray&) { return colors::black; })
    {}

scene::~scene() {
    m_objects.clear();
    m_lights.clear();
}

void scene::add_object(const object *obj) {
    m_objects.push_back(obj);
}

void scene::add_light(const light *lit) {
    m_lights.push_back(lit);
}

void scene::clear() {
    m_objects.clear();
    m_lights.clear();
}

scene::intersect_list scene::find_intersections(const ray& world_ray, const object_list& objects) {
    // find collisions (objects determine number of collisions) with the environment
    intersect_list intersections;
    for (auto objptr : objects) {
        basal::exception::throw_if(objptr == nullptr, __FILE__, __LINE__, "Object can't be nullptr");
        intersections.push_back(objptr->intersect(world_ray));
    }
    return intersections;
}

scene::intersect_set scene::nearest_object(const ray& world_ray,
                                           const intersect_list& intersections,
                                           const object_list& objects) {
    basal::exception::throw_unless(intersections.size() == objects.size(), __FILE__, __LINE__, "Lists must match!");
    const object* closest_object = nullptr;
    element_type closest_distance2 = std::numeric_limits<element_type>::max();
    geometry::intersection closest_intersection;
    for (size_t i = 0; i < intersections.size(); i++) {
        const geometry::intersection& inter = intersections[i];
        if (get_type(inter) == IntersectionType::Point) {
            // find the distance
            vector D = as_point(inter) - world_ray.location();
            element_type distance2 = D.quadrance();
            // distance can't be negative but can be zero
            // which means the world_ray is already touching so we don't need to care about that?
            if (basal::epsilon < distance2 and distance2 < closest_distance2) {
                // copy assign
                closest_distance2 = distance2;
                closest_intersection = inter;
                closest_object = objects[i];
            }
        } else if (get_type(inter) == IntersectionType::Points) {
            // returns two points, check each one.
            set_of_points sop = as_points(inter);
            std::cout << inter << std::endl;
            for (size_t j = 0; j < sop.size(); j++) {
                vector D = sop[j] - world_ray.location();
                element_type distance2 = D.quadrance();
                if (basal::epsilon < distance2 and distance2 < closest_distance2) {
                    // copy assign
                    closest_distance2 = distance2;
                    closest_intersection = intersection(sop[j]);
                    closest_object = objects[i];
                    // FIXME (Scene) what if the normal is incorrect? why would it be incorrect?
                }
            }
        }
    }
    return intersect_set(std::sqrt(closest_distance2), closest_intersection, closest_object);
}

color scene::find_surface_color(const object& obj,
                                const point& world_surface_point,
                                const vector& world_surface_normal,
                                const point& object_surface_point __attribute__((unused)),
                                const raytrace::point& volumetric_point,
                                const vector& reflection,
                                const object_list& objects,
                                const light_list& lights) {
    using namespace operators;

    // get the reference to the surface
    const medium& medium = obj.material();

    // The set of colors from each light source (including ambient)
    std::vector<color> surface_colors;

    // the medium starts at the ambient light
    surface_colors.push_back(medium.ambient(volumetric_point));

    // for each light in the scene... check the SHADOW rays!
    for (auto& light_ : lights) {
        // convenience variable
        const light& scene_light = *light_;
        // the samples for this light
        std::vector<color> surface_color_samples(scene_light.number_of_samples()); // should initialize to all black
        // for each sample, get the color
        for (size_t sample_index = 0; sample_index < scene_light.number_of_samples(); sample_index++) {
            // get the ray to the light source (full magnitude)
            ray ray_to_light = scene_light.incident(world_surface_point, sample_index);
            // just the vector to the light from P
            const vector& light_direction = ray_to_light.direction();
            // normalized light vector
            vector normalized_light_direction = light_direction.normalized();
            // construct a world ray from that point and normalized vector
            ray world_ray(world_surface_point, normalized_light_direction);
            // is the light blocked by anything?
            intersect_list intersections = find_intersections(world_ray, objects);
            // find the nearest object in the light path, which may include P (at/near zero)
            // if the current object is blocking the light.
            scene::intersect_set nearest = nearest_object(world_ray, intersections, objects);
            // is this point in a shadow of this light?
            // either there's no intersection to the light, or
            // there is one but it's farther away than the light itself.

            // debug the distance to the light source with a greyscale value.
            // ONLY THE FIRST LIGHT IS CONSIDERED
            if constexpr (use_grayscale_distance) {
                return color::greyscale(nearest.distance, 0.0, 100.0);
            }

            bool not_in_shadow = (   get_type(nearest.intersector) == IntersectionType::None
                                    or (    get_type(nearest.intersector) == IntersectionType::Point
                                        and nearest.distance > light_direction.norm()) );
            if (not_in_shadow) {
                if constexpr (debug_shadows_and_light) {
                    surface_color_samples[sample_index] = colors::red;
                } else {

                    // get the light color at this distance
                    color raw_light_color = scene_light.color_at(world_surface_point);
                    color incident_light = colors::black;
                    color specular_light = colors::black;

                    if constexpr (use_incident_scaling) {
                        // the scaling at this point due to this light source's
                        // relationship with the normal of the medium this will be
                        // some fractional component under 1.0 since both input vectors
                        // are normalized.
                        element_type incident_scaling = dot(normalized_light_direction, world_surface_normal);
                        if constexpr (enforce_ranges) {
                            basal::exception::throw_unless(within_inclusive(-1.0, incident_scaling, 1.0), __FILE__, __LINE__, "Must be within bounds");
                        }
                        if (incident_scaling > 0) { // the light is reflected
                            incident_light = incident_scaling * raw_light_color;
                        }
                        surface_color_samples[sample_index] = (medium.diffuse(volumetric_point) * incident_light);
                    } else {
                        surface_color_samples[sample_index] = raw_light_color;
                    }
                    if constexpr (use_specular_scaling) {
                        element_type specular_scaling = dot(normalized_light_direction, reflection);
                        if constexpr (enforce_ranges) {
                            basal::exception::throw_unless(within_inclusive(-1.0, specular_scaling, 1.0), __FILE__, __LINE__, "Must be within bounds");
                        }
                        if (specular_scaling > 0) {
                            // accumulate the specular into the existing color
                            surface_color_samples[sample_index] += medium.specular(volumetric_point, specular_scaling, raw_light_color);
                        }
                    }
                }
            } else {
                // don't add anything due to each light when you're in shadow.
                // for debug, you can use magenta in shadow
                if constexpr (debug_shadows_and_light) {
                    if (basal::equals_zero(nearest.distance)) {
                        surface_color_samples[sample_index] += colors::blue; // this means there's no light at all on this spot
                    } else if (nearest.distance < 0.0625) {
                        // if we're suffering from a near zero error
                        // this will illuminate creases or tight corners too
                        surface_color_samples[sample_index] += colors::yellow;
                    } else {
                        if (nearest.distance <= light_direction.norm()) {
                            // actually in shadow
                            //std::cout << "Nearest Intersection " << nearest.intersector << " @ " << nearest.distance << std::endl;
                            //std::cout << "Light " << scene_light << std::endl;
                            //std::cout << "Light Distance " << light_direction << " @ " << light_direction.norm() << std::endl;
                            surface_color_samples[sample_index] += colors::magenta;
                        } else {
                            // logic error, inverted distance or something?
                            surface_color_samples[sample_index] += colors::cyan;
                        }
                    }
                }
            }
        }
        // blend all the surface samples from this light (could be all black)
        //color surface_color = color::accumulate_samples(surface_color_samples);
        color surface_color = color::blend_samples(surface_color_samples);
        surface_colors.push_back(surface_color);
    }
    // now accumulate all the light sources together (including ambient)
    return color::accumulate_samples(surface_colors);
}

color scene::trace(const ray& world_ray, const medium& media, size_t reflection_depth, double recursive_contribution) {
    using namespace operators;

    // finds all the intersections with the objects
    intersect_list intersections = find_intersections(world_ray, m_objects);

    // find the closest intersection object
    intersect_set nearest = nearest_object(world_ray, intersections, m_objects);

    // if it was a point...
    if (get_type(nearest.intersector) == IntersectionType::Point) {
        assert(nearest.objptr != nullptr);
        // temporary for not using pointer.
        const object& obj = *nearest.objptr;
        // temporary for the object's medium
        const medium& medium = obj.material();
        // the intersection point in world space
        raytrace::point world_surface_point = as_point(nearest.intersector);
        // find produce the object surface point
        raytrace::point object_surface_point = obj.reverse_transform(world_surface_point);

        // find the normal on the surface at that point
        vector world_surface_normal = obj.normal(world_surface_point);
        if constexpr (enforce_ranges) {
            basal::exception::throw_unless(basal::equals(world_surface_normal.magnitude(), 1.0), __FILE__, __LINE__, "Must be normalized");
        }

        // compute the reflection vector
        vector world_reflection = obj.reflection(world_ray.direction(), world_surface_point);
        // compute the refracted vector
        ray world_refraction = obj.refraction(world_ray, world_surface_point, media.refractive_index(object_surface_point), medium.refractive_index(object_surface_point));

        // today we get the surface's reflectivity at this texture point
        // (which means the surface could be like a film of mirror with bits flaking off)
        element_type reflectivity = 0;
        element_type transparency = 0;
        element_type emissivity = 0;

        // several different types of color in this area to contend with
        color surface_color, emitted_color, reflected_color, transmitted_color;

        // compute the incident angle of the reflection vector
        iso::radians incident_angle = geometry::angle(world_surface_normal, world_reflection);

        // compute the transmitted angle of the refraction vector
        iso::radians transmitted_angle = geometry::angle(world_surface_normal, world_refraction.direction());

        /*********************************************************************/

        // get the light components, emission, reflection and refraction (diffraction, phosporescense and fluorescence not computed)
        medium.radiosity(object_surface_point,
                         media.refractive_index(object_surface_point),
                         incident_angle, transmitted_angle,
                         // outputs
                         emissivity, reflectivity, transparency);

        if (emissivity > 0) { // the medium is emitting light
            // TODO light sources which emit light
            emitted_color = colors::black;
            // surface_color += ???
        }
        if (reflectivity > 0.0) { // on the very last depth call we still have to return the surface color, just no more casts
            // find the *reflected* medium color from all lights (without blocked paths)
            // The set of colors from each light source (including ambient)
            std::vector<color> surface_colors;
            // the medium starts at the ambient light
            surface_colors.push_back(medium.ambient(object_surface_point));
            // for each light in the scene... check the SHADOW rays!
            for (auto& light_ : m_lights) {
                // convenience variable
                const light& scene_light = *light_;
                // the samples for this light
                std::vector<color> surface_color_samples(scene_light.number_of_samples()); // should initialize to all black
                // for each sample, get the color
                for (size_t sample_index = 0; sample_index < scene_light.number_of_samples(); sample_index++) {
                    // get the ray to the light source (full magnitude)
                    ray ray_to_light = scene_light.incident(world_surface_point, sample_index);
                    // just the vector to the light from P
                    const vector& light_direction = ray_to_light.direction();
                    // normalized light vector
                    vector normalized_light_direction = light_direction.normalized();
                    // construct a world ray from that point and normalized vector
                    ray world_ray(world_surface_point, normalized_light_direction);
                    // is the light blocked by anything?
                    intersect_list intersections = find_intersections(world_ray, m_objects);
                    // find the nearest object in the light path, which may include P (at/near zero)
                    // if the current object is blocking the light.
                    scene::intersect_set nearest = nearest_object(world_ray, intersections, m_objects);
                    // is this point in a shadow of this light?
                    // either there's no intersection to the light, or
                    // there is one but it's farther away than the light itself.
                    bool not_in_shadow = (   get_type(nearest.intersector) == IntersectionType::None
                                          or (    get_type(nearest.intersector) == IntersectionType::Point
                                              and nearest.distance > light_direction.norm()));
                    if (not_in_shadow) {
                        // get the light color at this distance
                        color raw_light_color = scene_light.color_at(world_surface_point);
                        // the scaling at this point due to this light source's
                        // relationship with the normal of the medium this will be
                        // some fractional component from -1.0 to 1.0 since both input vectors are normalized.
                        element_type incident_scaling = dot(normalized_light_direction, world_surface_normal);
                        //basal::exception::throw_unless(within_inclusive(-1.0, incident_scaling, 1.0), __FILE__, __LINE__, "Must be within bounds");
                        color incident_light = (incident_scaling > 0.0) ? incident_scaling * raw_light_color : colors::black;
                        color diffuse_light = medium.diffuse(object_surface_point);
                        element_type specular_scaling = dot(normalized_light_direction, world_reflection);
                        //basal::exception::throw_unless(within_inclusive(-1.0, specular_scaling, 1.0), __FILE__, __LINE__, "Must be within bounds");
                        color specular_light = (specular_scaling > 0) ? medium.specular(object_surface_point, specular_scaling, raw_light_color) : colors::black;
                        // blend the light color and the surface color together
                        surface_color_samples[sample_index] = (diffuse_light * incident_light);
                        // don't use color + color as that "blends", use accumulate for specular light.
                        surface_color_samples[sample_index] += specular_light;
                    }
                }
                // blend or accumulate? all the surface samples from this light (could be all black)
                //color surface_color = color::accumulate_samples(surface_color_samples);
                color surface_color = color::blend_samples(surface_color_samples);
                surface_colors.push_back(surface_color);
            }
            // now accumulate all the light sources together (including ambient)
            color surface_properties_color = color::accumulate_samples(surface_colors);

            if (reflection_depth > 0) {
                // mix how much local surface color versus reflected surface there should be
                element_type smoothness = medium.smoothness(object_surface_point);
                if (smoothness > 0.0) {
                    // should we continue bouncing given the contribution?
                    if (recursive_contribution < adaptive_reflection_threshhold) {
                        // count this as a save bounce
                        statistics::get().saved_ray_traces++;
                        // just use the surface color
                        reflected_color = surface_properties_color;
                    } else { // only cast the ray if it's more than zero
                        // find the reflected vector at world_surface_point
                        ray new_ray(world_surface_point, world_reflection);

                        // find out what the reflection adds to this
                        color bounced_color = trace(new_ray, media, reflection_depth - 1, recursive_contribution * smoothness);
                        statistics::get().bounced_rays++;

                        // somehow interpolate the two based on how much of a smooth mirror this medium is.
                        reflected_color = interpolate(bounced_color, surface_properties_color, smoothness);
                    }
                } else {
                    // perfectly diffuse so it's just surface properties
                    reflected_color = surface_properties_color;
                }
            } else {
                // no more reflections so it's only surface properties!
                reflected_color = surface_properties_color;
            }
        }
        if (reflection_depth > 0 and transparency > 0.0 and not world_refraction.direction().is_zero()) {
            // find the dropoff of the medium we're *in* given the distance
            element_type dropoff = medium.absorbance(nearest.distance);
            element_type transmitted_scaling = 1.0 - dropoff;
            if (transmitted_scaling > 0.0) {
                if (recursive_contribution < adaptive_reflection_threshhold) {
                    // due to the low contribution to the final color, we can disregard this.
                    statistics::get().saved_ray_traces++;
                    // just return black?
                    transmitted_color = colors::black;
                } else {
                    // get the colors from the transmitted light
                    transmitted_color = trace(world_refraction, medium, reflection_depth - 1, recursive_contribution * transmitted_scaling);
                    // now scale that transmitted_color via the dropoff
                    transmitted_color.scale(transmitted_scaling);
                    // this ray was transmitted through the new medium
                    statistics::get().transmitted_rays++;
                }
            } else {
                // not a transmissible medium, set that to black;
                transmitted_color = colors::black;
            }
        }
        // blend that reflected color with the transmitted color
        surface_color = interpolate(transmitted_color, reflected_color, transparency);
        // TODO now add emitted color
        //surface_color += emitted_color;
        return surface_color;
    } else { // if (get_type(closest_intersection) == geometry::intersectionType::None) {
        // return the background as the ray didn't hit anything.
        return m_background(world_ray);
    }
}

void scene::render(std::string filename,
                   size_t number_of_samples,
                   size_t reflection_depth,
                   std::optional<image::rendered_line> row_notifier,
                   uint8_t aaa_mask_threshold) {
    bool adaptive_antialiasing = aaa_mask_threshold != raytrace::image::AAA_MASK_DISABLED;
    if constexpr (debug) {
        view.print("Camera Info:\n");
    }
    auto tracer = [&](const image::point& pnt) -> color {
        // create the ray at each point in the image along the vector
        // from the image plane along the camera ray.
        ray world_ray = view.cast(pnt);

        // trace the ray out to the world, starting from a vaccum
        return trace(world_ray, mediums::vaccum, reflection_depth);
    };
    // if we're doing adaptive anti-aliasing we only shoot 1 ray at first and then compute a constrast mask later
    view.capture.generate_each(tracer, adaptive_antialiasing ? 1 : number_of_samples, row_notifier, &view.mask, image::AAA_MASK_DISABLED);
    //
    if (aaa_mask_threshold < image::AAA_MASK_DISABLED) {
        // reset all rendered lines
        if (row_notifier != std::nullopt) {
            image::rendered_line func = row_notifier.value();
            for (size_t y = 0; y < view.capture.height; y++) {
                func(y, false);
            }
        }
        // compute the mask
        fourcc::sobel_mask(view.capture, view.mask);
        // update the image based on the mask
        view.capture.generate_each(tracer, number_of_samples, row_notifier, &view.mask, aaa_mask_threshold);
    }
    view.capture.save(filename);
}

void scene::print(const char str[]) const {
    std::cout << str << std::endl;
    view.print(str);
    for (auto obj : m_objects) {
        obj->print(str);
    }
    for (auto lite : m_lights) {
        lite->print(str);
    }
}

void scene::set_background_mapper(background_mapper mapper) {
    if (mapper) {
        m_background = mapper;
    }
}

}