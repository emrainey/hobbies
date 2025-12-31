
#include "raytrace/objects/quadratic.hpp"

namespace raytrace {
namespace objects {

quadratic::quadratic(point const& center)
    : object{center, 2, Type::Quadratic, true}  // 2 collisions, closed surface for now, subclasses must correctly set!
    , m_coefficients{4, 4}                      // start zeroed
{
    basal::exception::throw_unless(m_coefficients.rows == 4 and m_coefficients.cols == 4, __FILE__, __LINE__,
                                   "Must be a 4x4");
}

quadratic::quadratic(point const& center, raytrace::matrix& C)
    : object{center, 2, Type::None, true}  // 2 collisions, closed surface for now, subclasses must correctly set!
    , m_coefficients{C} {
    basal::exception::throw_unless(m_coefficients.rows == 4 and m_coefficients.cols == 4, __FILE__, __LINE__,
                                   "Must be a 4x4");
}

vector quadratic::normal_(point const& object_surface_point) const {
    raytrace::matrix const& Q = m_coefficients;  // ease of reference
    precision x = object_surface_point.x;
    precision y = object_surface_point.y;
    precision z = object_surface_point.z;
    precision Nx = 2.0_p * (Q.at(1, 1) * x + Q.at(1, 2) * y + Q.at(1, 3) * z + Q.at(1, 4));
    precision Ny = 2.0_p * (Q.at(2, 1) * x + Q.at(2, 2) * y + Q.at(2, 3) * z + Q.at(2, 4));
    precision Nz = 2.0_p * (Q.at(3, 1) * x + Q.at(3, 2) * y + Q.at(3, 3) * z + Q.at(3, 4));
    vector object_normal{{Nx, Ny, Nz}};
    object_normal.normalize();
    return forward_transform(object_normal);
}

hits quadratic::collisions_along(ray const& object_ray) const {
    hits ts;
    precision x = object_ray.location().x;
    precision y = object_ray.location().y;
    precision z = object_ray.location().z;
    precision i = object_ray.direction()[0];
    precision j = object_ray.direction()[1];
    precision k = object_ray.direction()[2];
    raytrace::matrix const& Q = m_coefficients;  // ease of reference
    precision Q1221 = Q.at(1, 2) + Q.at(2, 1);
    precision Q2332 = Q.at(2, 3) + Q.at(3, 2);
    precision Q1331 = Q.at(1, 3) + Q.at(3, 1);
    precision Q1441 = Q.at(1, 4) + Q.at(4, 1);
    precision Q2442 = Q.at(2, 4) + Q.at(4, 2);
    precision Q3443 = Q.at(3, 4) + Q.at(4, 3);
    precision Q11 = Q.at(1, 1);
    precision Q22 = Q.at(2, 2);
    precision Q33 = Q.at(3, 3);
    precision Q44 = Q.at(4, 4);
    precision a = i * (i * Q11 + j * (Q1221)) + j * (j * Q22 + k * (Q2332)) + k * (k * Q33 + i * (Q1331));
    precision b = i * ((z * (Q1331) + y * (Q1221) + x * (2.0_p * Q11)) + (Q1441))
                  + j * ((z * (Q2332) + x * (Q1221) + y * (2.0_p * Q22)) + (Q2442))
                  + k * ((y * (Q2332) + x * (Q1331) + z * (2.0_p * Q33)) + (Q3443));
    precision c = x * (x * Q11 + y * (Q1221) + z * (Q1331) + (Q1441)) + y * (y * Q22 + z * (Q2332) + (Q2442))
                  + z * (z * Q33 + (Q3443)) + Q44;
    auto roots = quadratic_roots(a, b, c);
    precision t0 = std::get<0>(roots);
    precision t1 = std::get<1>(roots);
    if (not basal::is_nan(t0)) {
        point R = object_ray.distance_along(t0);
        ts.emplace_back(intersection{R}, t0, normal_(R), this);
    }
    if (not basal::is_nan(t1)) {
        point R = object_ray.distance_along(t1);
        ts.emplace_back(intersection{R}, t1, normal_(R), this);
    }
    return ts;
}

bool quadratic::is_surface_point(point const& world_point) const {
    point object_point = reverse_transform(world_point);
    precision x = object_point.x;
    precision y = object_point.y;
    precision z = object_point.z;
    raytrace::matrix const& Q = m_coefficients;  // ease of reference
    precision a = Q(1, 1), b = Q(1, 2), c = Q(1, 3), d = Q(1, 4);
    precision e = Q(2, 2), f = Q(2, 3), g = Q(2, 4), h = Q(3, 3);
    precision i = Q(3, 4), j = Q(4, 4);

    // ax2 + 2bxy + 2cxz + 2dx + ey2 +2fyz + 2gy + hz2 + 2iz +j=0,
    return basal::nearly_zero((a * x * x) + (2 * b * x * y) + (2 * c * x * z) + (2 * d * x) + (e * y * y)
                              + (2 * f * y * z) + (2 * g * y) + (h * z * z) + (2 * i * z) + j);
}

image::point quadratic::map(point const& object_surface_point __attribute__((unused))) const {
    // map some range of object 3D points to some 2D u,v pair
    // isolate which axis this is on and return the forward_transformed normal
    precision u = 0.0_p, v = 0.0_p;
    // TODO implement mapping for quadratic surface... how??
    return image::point(u, v);
}

void quadratic::print(std::ostream& os, char const name[]) const {
    os << " Quadratic " << this << " " << name << " " << position() << ", Coefficients {" << m_coefficients << "}"
       << std::endl;
}

precision quadratic::get_object_extent(void) const {
    // if it's an sphere, ellipsoid, the extent can be computed,
    // if it's an paraboloid, hyperboloid, the extent cannot be computed,
    return basal::pos_inf;
}

}  // namespace objects
}  // namespace raytrace
