#include "raytrace/scene.hpp"

#include <cassert>
#include <tuple>

namespace raytrace {

scene::scene(double art)
    : adaptive_reflection_threshold{art}
    , m_objects{}
    , m_lights{}
    , m_background{[](raytrace::ray const&) { return colors::black; }}
    , m_media{&mediums::vacuum}  // default to a vacuum
{
}

scene::~scene() {
    m_objects.clear();
    m_lights.clear();
}

void scene::add_object(objects::object const* obj) {
    m_objects.push_back(obj);
    // compute the bounding box for the scene
    auto bounds = obj->get_world_bounds();
    if constexpr (debug::scene) {
        obj->print("Object for Bounds");
        std::cout << " Min: " << bounds.min << " Max: " << bounds.max << std::endl;
    }
    if (bounds.is_infinite()) {
        m_infinite_objects.push_back(obj);
    } else {
        m_bounds.grow(bounds);
        if constexpr (debug::scene) {
            std::cout << "Growing Bounds to: " << m_bounds.min << " Max: " << m_bounds.max << std::endl;
        }
    }
}

void scene::add_light(lights::light const* lit) {
    m_lights.push_back(lit);
}

void scene::add_media(mediums::medium const* media) {
    if (media) {
        m_media = media;
    }
}

void scene::clear() {
    m_objects.clear();
    m_lights.clear();
}

size_t scene::number_of_objects(void) const {
    return m_objects.size();
}

size_t scene::number_of_lights(void) const {
    return m_lights.size();
}

objects::hits scene::find_intersections(ray const& world_ray, object_list const& objects) {
    // find collisions (objects determine number of collisions) with the environment
    objects::hits hits;
    /// FIXME this is not very efficient as we could use a some structure to determine where the ray is intersecting
    for (auto objptr : objects) {
        basal::exception::throw_if(objptr == nullptr, __FILE__, __LINE__, "Object can't be nullptr");
        auto collision = objptr->intersect(world_ray);
        if (get_type(collision.intersect) != IntersectionType::None) {
            statistics::get().intersections_with_objects++;
        } else {
            statistics::get().missed_rays++;
        }
        if (get_type(collision.intersect) == IntersectionType::Point) {
            statistics::get().intersections_with_point++;
        } else if (get_type(collision.intersect) == IntersectionType::Points) {
            statistics::get().intersections_with_points++;
        } else if (get_type(collision.intersect) == IntersectionType::Line) {
            statistics::get().intersections_with_line++;
        }
        hits.push_back(collision);
    }
    return hits;
}

objects::hit scene::nearest_object(ray const& world_ray, objects::hits const& hits, object_list const& objects) {
    basal::exception::throw_unless(hits.size() == objects.size(), __FILE__, __LINE__, "Lists must match!");
    precision closest_distance2 = std::numeric_limits<precision>::max();
    objects::hit closest_hit;
    for (auto const& this_hit : hits) {
        if constexpr (debug::hit) {
            std::cout << this_hit << std::endl;
        }
        if (get_type(this_hit.intersect) == IntersectionType::Point) {
            // find the distance
            vector D = as_point(this_hit.intersect) - world_ray.location();
            precision distance2 = D.quadrance();
            // distance can't be negative but can be zero
            // which means the world_ray is already touching so we don't need to care about that?
            if (basal::epsilon < distance2 and distance2 < closest_distance2) {
                closest_distance2 = distance2;
                closest_hit = this_hit;
            }
        } else if (get_type(this_hit.intersect) == IntersectionType::Points) {
            // returns two points, check each one.
            auto sop = as_points(this_hit.intersect);
            for (auto const& pnt : sop) {
                vector D = pnt - world_ray.location();
                precision distance2 = D.quadrance();
                if (basal::epsilon < distance2 and distance2 < closest_distance2) {
                    closest_distance2 = distance2;
                    closest_hit = this_hit;
                }
            }
        }
    }
    return closest_hit;
}

color scene::trace(ray const& world_ray, mediums::medium const& media, size_t reflection_depth,
                   precision recursive_contribution) {
    using namespace operators;

    // this will store the final traced value for this call.
    color traced_color;

    // finds all the intersections with the objects
    objects::hits hits = find_intersections(world_ray, m_objects);

    // find the closest intersection object
    objects::hit nearest = nearest_object(world_ray, hits, m_objects);

    // if it was a point...
    if (get_type(nearest.intersect) == IntersectionType::Point) {
        assert(nearest.object != nullptr);
        // temporary for not using pointer.
        objects::object const& obj = *nearest.object;
        // temporary for the object's medium
        mediums::medium const& medium = obj.material();
        // the intersection point in world space
        raytrace::point world_surface_point = as_point(nearest.intersect);
        // find produce the object surface point
        raytrace::point object_surface_point = obj.reverse_transform(world_surface_point);
        // grab the normal on the surface at that point, ensure it's normalized
        vector world_surface_normal = nearest.normal.normalized();
        if constexpr (enforce_contracts) {
            basal::exception::throw_unless(basal::nearly_equals(world_surface_normal.quadrance(), 1.0_p), __FILE__,
                                           __LINE__, "Must be normalized");
        }
        // if this is true, we've collided with something from the inside or the "backside"
        bool inside_out = (dot(world_surface_normal, world_ray.direction()) > 0);
        raytrace::statistics::get().inside_out_intersections += (inside_out ? 1 : 0);

        // compute the reflection vector
        ray world_reflection = obj.reflection(world_ray, world_surface_normal, world_surface_point);
        // compute the refracted vector
        ray world_refraction = obj.refraction(world_ray, world_surface_normal, world_surface_point,
                                              media.refractive_index(object_surface_point),
                                              medium.refractive_index(object_surface_point));
        if constexpr (enforce_contracts) {
            basal::exception::throw_if(dot(world_ray.direction(), world_refraction.direction()) < 0, __FILE__, __LINE__,
                                       "Refracted ray should not be opposites");
        }
        // today we get the surface's reflectivity at this texture point
        // (which means the surface could be like a film of mirror with bits flaking off)
        precision reflectivity = 0.0_p;
        precision transparency = 0.0_p;
        precision emissivity = 0.0_p;

        // several different types of color in this area to contend with
        color surface_color, emitted_color, reflected_color, transmitted_color;

        // compute the incident angle of the reflection vector
        iso::radians incident_angle
            = geometry::angle((inside_out ? -world_surface_normal : world_surface_normal), world_ray.direction());

        // compute the transmitted angle of the refraction vector
        iso::radians transmitted_angle = geometry::angle((inside_out ? -world_surface_normal : world_surface_normal),
                                                         world_refraction.direction());

        ///*******************************************************************/

        // get the light components, emission, reflection and refraction (diffraction, phosphorescence and
        // fluorescence not computed)
        medium.radiosity(object_surface_point, media.refractive_index(object_surface_point), incident_angle,
                         transmitted_angle,
                         // outputs
                         emissivity, reflectivity, transparency);

        // the medium is emitting light
        if (emissivity > 0.0_p) {
            // light sources which emit light
            emitted_color = medium.emissive(object_surface_point) * emissivity;
        }
        // on the very last depth call we still have to return the surface color, just no more casts
        if (reflectivity > 0.0_p) {
            // find the *reflected* medium color from all lights (without blocked paths)
            // The set of colors from each light source (including ambient)
            std::vector<color> surface_colors;
            // the medium starts at the ambient light
            surface_colors.push_back(medium.ambient(object_surface_point));
            // for each light in the scene... check the SHADOW rays!
            statistics::get().shadow_rays++;
            for (auto& light_ : m_lights) {
                // convenience variable
                lights::light const& scene_light = *light_;
                // the samples for this light
                std::vector<color> surface_color_samples(
                    scene_light.number_of_samples());  // should initialize to all black
                // for each sample, get the color
                for (size_t sample_index = 0; sample_index < scene_light.number_of_samples(); sample_index++) {
                    statistics::get().sampled_rays++;
                    // get the ray to the light source (full magnitude)
                    ray ray_to_light = scene_light.incident(world_surface_point, sample_index);
                    // just the vector to the light from P
                    vector const& light_direction = ray_to_light.direction();
                    // normalized light vector
                    vector normalized_light_direction = light_direction.normalized();
                    // construct a world ray from that point and normalized vector
                    ray world_ray(world_surface_point, normalized_light_direction);
                    // is the light blocked by anything?
                    objects::hits blockers = find_intersections(world_ray, m_objects);
                    // find the nearest object in the light path, which may include P (at/near zero)
                    // if the current object is blocking the light.
                    objects::hit blocker = nearest_object(world_ray, blockers, m_objects);
                    // is this point in a shadow of this light?
                    // either there's no intersection to the light, or
                    // there is one but it's farther away than the light itself.
                    bool no_intersection = (get_type(blocker.intersect) == IntersectionType::None);
                    bool point_farther_than_light = false;
                    bool object_is_transparent = false;
                    bool object_is_emissive = false;
                    if (get_type(blocker.intersect) == IntersectionType::Point) {
                        point_farther_than_light = (blocker.distance > light_direction.norm());
                        if (blocker.object != nullptr) {
                            auto other_world_point = as_point(blocker.intersect);
                            // FIXME is a refractive object so it must be transparent?
                            // object_is_transparent =
                            // (nearest.objptr->material().refractive_index(other_world_point) > 0.0_p);
                            // object_is_emissive = (nearest.objptr->material().emissive(other_world_point) > 0.0_p);
                        }
                    }
                    bool not_in_shadow
                        = (no_intersection or point_farther_than_light or object_is_transparent or object_is_emissive);
                    if (not_in_shadow) {
                        statistics::get().color_sampled_rays++;
                        // get the light color at this distance
                        color raw_light_color = scene_light.color_at(world_surface_point);
                        // the scaling at this point due to this light source's
                        // relationship with the normal of the medium this will be
                        // some fractional component from -1.0_p to 1.0_p since both input vectors are normalized.
                        precision incident_scaling = dot(normalized_light_direction, world_surface_normal);
                        if constexpr (enforce_contracts) {
                            basal::exception::throw_unless(within_inclusive(-1.0_p, incident_scaling, 1.0_p), __FILE__,
                                                           __LINE__, "Must be within bounds");
                        }
                        color incident_light
                            = (incident_scaling > 0.0_p) ? incident_scaling * raw_light_color : colors::black;
                        color diffuse_light = medium.diffuse(object_surface_point);
                        precision specular_scaling = dot(normalized_light_direction, world_reflection.direction());
                        if constexpr (enforce_contracts) {
                            basal::exception::throw_unless(within_inclusive(-1.0_p, specular_scaling, 1.0_p), __FILE__,
                                                           __LINE__, "Must be within bounds");
                        }
                        color specular_light = medium.specular(object_surface_point, specular_scaling, raw_light_color);
                        // blend the light color and the surface color together
                        surface_color_samples[sample_index] = (diffuse_light * incident_light);
                        // don't use color + color as that "blends", use accumulate for specular light.
                        surface_color_samples[sample_index] += specular_light;
                        // now add the transmitted light if the object was transparent
                        // FIXME this is incorrect as we're only considering if the object is inline with the light,
                        // not if it's out of line!
                        if (object_is_transparent) {
                            // convenience reference
                            raytrace::objects::object const& obj = *nearest.object;
                            // convenience reference
                            raytrace::mediums::medium const& mat = obj.material();
                            // trace another ray through the object
                            surface_color_samples[sample_index]
                                += trace(world_ray, mat, reflection_depth - 1, recursive_contribution);
                        }
                    } else {
                        statistics::get().point_in_shadow++;
                    }
                }
                // blend or accumulate? all the surface samples from this light (could be all black)
                // color surface_color = color::accumulate_samples(surface_color_samples);
                color surface_color = color::blend_samples(surface_color_samples);
                surface_colors.push_back(surface_color);
            }
            // now accumulate all the light sources together (including ambient)
            color surface_properties_color = color::accumulate_samples(surface_colors);

            if (reflection_depth > 0) {
                // mix how much local surface color versus reflected surface there should be
                precision smoothness = medium.smoothness(object_surface_point);
                if (smoothness > 0.0_p) {
                    // should we continue bouncing given the contribution?
                    if (recursive_contribution < adaptive_reflection_threshold) {
                        // count this as a save bounce (plus the rest we won't do)
                        statistics::get().saved_ray_traces += reflection_depth;
                        // just use the surface color
                        reflected_color = surface_properties_color;
                    } else {  // only cast the ray if it's more than zero
                        // this ray was bounced off an object
                        statistics::get().bounced_rays++;

                        // find out what the reflection adds to this
                        color bounced_color = medium.bounced(
                            world_surface_point,
                            trace(world_reflection, media, reflection_depth - 1, recursive_contribution * smoothness));

                        // somehow interpolate the two based on how much of a smooth mirror this medium is.
                        reflected_color = interpolate(bounced_color, surface_properties_color, smoothness);
                    }
                } else {
                    // perfectly diffuse so it's just surface properties
                    reflected_color = surface_properties_color;
                }
            } else {
                // no more reflections so it's only surface properties!
                // FIXME for metals this seems wrong as the color should be altering the light.
                reflected_color = surface_properties_color;
            }
        }
        if (reflection_depth > 0 and transparency > 0.0_p and not world_refraction.direction().is_zero()) {
            // this ray was transmitted through the new medium
            statistics::get().transmitted_rays++;
            // get the colors from the transmitted light
            transmitted_color = trace(world_refraction, medium, reflection_depth - 1, recursive_contribution);
            // FIXME figure out how to diminish the recursive contribution by the dropoff in the media? maybe check
            // it above?
        }
        // blend that reflected color with the transmitted color
        surface_color = interpolate(transmitted_color, reflected_color, transparency);
        // now add emitted color
        surface_color += emitted_color;  // this will clamp internally
        // the media will absorb some of the light from that surface.
        traced_color = media.absorbance(nearest.distance, surface_color);
    } else {  // if (get_type(closest_intersection) == geometry::intersectionType::None) {
        // return the background as the ray didn't hit anything.
        traced_color = media.absorbance(std::numeric_limits<precision>::infinity(), m_background(world_ray));
    }
    return traced_color;
}

void scene::render(camera& view, std::string filename, size_t number_of_samples, size_t reflection_depth,
                   std::optional<image::rendered_line> row_notifier, uint8_t aaa_mask_threshold, bool filter_capture) {
    bool adaptive_antialiasing = aaa_mask_threshold != raytrace::image::AAA_MASK_DISABLED;
    if constexpr (debug::camera) {
        view.print("Camera Info:\n");
    }

    // FIXME insert the code which creates the octo-tree here as it can't be done in the add_object method correctly

    auto tracer = [&](image::point const& pnt) -> color {
        // create the ray at each point in the image along the vector
        // from the image plane along the camera ray.
        ray world_ray = view.cast(pnt);

        // trace the ray out to the world, starting from a vacuum
        return trace(world_ray, *m_media, reflection_depth);
    };
    // if we're doing adaptive anti-aliasing we only shoot 1 ray at first and then compute a contrast mask
    // later
    view.capture.generate_each(tracer, adaptive_antialiasing ? 1 : number_of_samples, row_notifier, &view.mask,
                               image::AAA_MASK_DISABLED);
    // if the threshold is not disabled, then compute the extra pixels based on the mask
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
    // if we want to filter the image before viewing or saving, do that here.
    if (filter_capture) {
        // copy the image into a duplicate
        fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8> capture_copy(view.capture);
        int16_t kernel[3]{1, 2, 1};
        fourcc::filter(view.capture, capture_copy, kernel);
    }
    view.capture.save(filename);
}

void scene::print(char const str[]) const {
    std::cout << str << std::endl;
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

}  // namespace raytrace