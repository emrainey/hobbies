
#include "raytrace/objects/quadratic.hpp"

namespace raytrace {
namespace objects {

quadratic::quadratic(const point& center)
    : object(center, 2, true)  // 2 collisions, closed surface (CHECK)
    , m_coefficients(4, 4)     // start zeroed
{
    basal::exception::throw_unless(m_coefficients.rows == 4 and m_coefficients.cols == 4, __FILE__, __LINE__,
                                   "Must be a 4x4");
}

quadratic::quadratic(const point& center, raytrace::matrix& C)
    : object(center, 2, true)  // 2 collisions, closed surface (CHECK)
    , m_coefficients(C) {
    basal::exception::throw_unless(m_coefficients.rows == 4 and m_coefficients.cols == 4, __FILE__, __LINE__,
                                   "Must be a 4x4");
}

vector quadratic::normal(const point& world_surface_point) const {
    point object_surface_point = reverse_transform(world_surface_point);
    const raytrace::matrix& Q = m_coefficients;  // ease of reference
    element_type x = world_surface_point.x;
    element_type y = world_surface_point.y;
    element_type z = world_surface_point.z;
    element_type Nx = 2.0 * (Q.at(1, 1) * x + Q.at(1, 2) * y + Q.at(1, 3) * z + Q.at(1, 4));
    element_type Ny = 2.0 * (Q.at(2, 1) * x + Q.at(2, 2) * y + Q.at(2, 3) * z + Q.at(2, 4));
    element_type Nz = 2.0 * (Q.at(3, 1) * x + Q.at(3, 2) * y + Q.at(3, 3) * z + Q.at(3, 4));
    vector object_normal{{Nx, Ny, Nz}};
    object_normal.normalize();
    return forward_transform(object_normal);
}

hits quadratic::collisions_along(const ray& object_ray) const {
    hits ts;
    element_type x = object_ray.location().x;
    element_type y = object_ray.location().y;
    element_type z = object_ray.location().z;
    element_type i = object_ray.direction()[0];
    element_type j = object_ray.direction()[1];
    element_type k = object_ray.direction()[2];
    const raytrace::matrix& Q = m_coefficients;  // ease of reference
    element_type Q1221 = Q.at(1, 2) + Q.at(2, 1);
    element_type Q2332 = Q.at(2, 3) + Q.at(3, 2);
    element_type Q1331 = Q.at(1, 3) + Q.at(3, 1);
    element_type Q1441 = Q.at(1, 4) + Q.at(4, 1);
    element_type Q2442 = Q.at(2, 4) + Q.at(4, 2);
    element_type Q3443 = Q.at(3, 4) + Q.at(4, 3);
    element_type Q11 = Q.at(1, 1);
    element_type Q22 = Q.at(2, 2);
    element_type Q33 = Q.at(3, 3);
    element_type Q44 = Q.at(4, 4);
    element_type a = i * (i * Q11 + j * (Q1221)) + j * (j * Q22 + k * (Q2332)) + k * (k * Q33 + i * (Q1331));
    element_type b = i * ((z * (Q1331) + y * (Q1221) + x * (2.0 * Q11)) + (Q1441)) +
                     j * ((z * (Q2332) + x * (Q1221) + y * (2.0 * Q22)) + (Q2442)) +
                     k * ((y * (Q2332) + x * (Q1331) + z * (2.0 * Q33)) + (Q3443));
    element_type c = x * (x * Q11 + y * (Q1221) + z * (Q1331) + (Q1441)) + y * (y * Q22 + z * (Q2332) + (Q2442)) +
                     z * (z * Q33 + (Q3443)) + Q44;
    auto roots = quadratic_roots(a, b, c);
    element_type t0 = std::get<0>(roots);
    element_type t1 = std::get<1>(roots);
    if (not std::isnan(t0)) {
        ts.push_back(t0);
    }
    if (not std::isnan(t1)) {
        ts.push_back(t1);
    }
    return ts;
}

bool quadratic::is_surface_point(const point& world_point) const {
    point object_point = reverse_transform(world_point);
    element_type x = object_point.x;
    element_type y = object_point.y;
    element_type z = object_point.z;
    const raytrace::matrix& Q = m_coefficients;  // ease of reference
    element_type a = Q(1, 1), b = Q(1, 2), c = Q(1, 3), d = Q(1, 4);
    element_type e = Q(2, 2), f = Q(2, 3), g = Q(2, 4), h = Q(3, 3);
    element_type i = Q(3, 4), j = Q(4, 4);

    // ax2 +2bxy+2cxz+2dx+ey2 +2fyz+ 2gy + hz2 + 2iz +j=0,
    return basal::equals_zero((a * x * x) + (2 * b * x * y) + (2 * c * x) + (2 * d * x) + (e * y * y) +
                              (2 * f * y * z) + (2 * g * y) + (h * z * z) + (2 * i * z) + j);
}

image::point quadratic::map(const point& object_surface_point __attribute__((unused))) const {
    // map some range of object 3D points to some 2D u,v pair
    // isolate which axis this is on and return the forward_transformed normal
    element_type u = 0.0, v = 0.0;
    // TODO implement mapping for quadratic surface... how??
    return image::point(u, v);
}

void quadratic::print(const char name[]) const {
    std::cout << name << " " << *this << std::endl;
}

std::ostream& operator<<(std::ostream& os, const quadratic& q) {
    os << " Quadratic " << q.position() << ", Coefficients {" << q.m_coefficients << "}";
    return os;
}

}  // namespace objects
}  // namespace raytrace
