#include "raytrace/objects/torus.hpp"

#include <iostream>

namespace raytrace {
namespace objects {
using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

torus::torus(point const& C, precision ring_radius, precision tube_radius)
    : object{C, 4, true}  // up to 4 collisions, closed surface
      ,
      m_ring_radius{ring_radius},
      m_tube_radius{tube_radius} {
    basal::exception::throw_if(tube_radius > ring_radius, __FILE__, __LINE__,
                               "Self-intersecting torus, tube must be smaller than ring radius");
}

vector torus::normal_(point const& object_surface_point) const {
    // project along the XY plane
    vector ring_vector{{object_surface_point.x, object_surface_point.y, 0}};
    // normalize so we can then...
    ring_vector.normalize();
    // scale it to the ring
    ring_vector *= m_ring_radius;
    // make a ring point
    raytrace::point ring_point = R3::origin + ring_vector;
    // create a vector from the ring to the pnt
    return (object_surface_point - ring_point).normalize();
}

hits torus::collisions_along(ray const& object_ray) const {
    hits ts;
    point A = object_ray.closest(R3::origin);
    vector D = A - R3::origin;
    precision rr2 = (m_ring_radius + m_tube_radius) * (m_ring_radius + m_tube_radius);
    if (D.quadrance() <= rr2) {
        // formulate the torus equation (sqrt(x*x + y*y) + R)^2 +z*z = r*r into a quartic equation of t
        // given each x y z is a line equation (x => x + ti)
        // ((x_ti)^2 + (y+tj)^2 + q^2 + (z+tk)^2 - r^2)^2 = 4q^2((x+ti)^2 + (y+tj)^2)
        // @see
        // https://www.wolframalpha.com/input/?i=FullSimplify+%28%28x%2Bti%29%5E2%2B%28y%2Btj%29%5E2+%2B+q%5E2+%2B+%28z%2Btk%29%5E2+-+r%5E2%29%5E2%3D4q%5E2%28%28x%2Bti%29%5E2+%2B+%28y%2Btj%29%5E2%29+
        // in order to solve this, we have to have a quartic root solver, which is quite complex.
        // first break down line components into x,y,z & i,j,k
        // also, I refined the equations from http://blog.marcinchwedczuk.pl/ray-tracing-torus (they had simplified
        // versions which I think were better than those I dervied)
        precision x = object_ray.location().x;
        precision y = object_ray.location().y;
        precision z = object_ray.location().z;
        precision i = object_ray.direction()[0];
        precision j = object_ray.direction()[1];
        precision k = object_ray.direction()[2];
        precision q = m_ring_radius;
        precision r = m_tube_radius;
        // preprocessing and common terms
        precision ii = i * i;
        precision jj = j * j;
        precision kk = k * k;
        precision ix = i * x;
        precision jy = j * y;
        precision kz = k * z;
        precision qq = q * q;
        precision rr = r * r;
        // precision qq_rr = qq - rr;
        precision rr_qq = rr + qq;
        precision xx = x * x;
        precision yy = y * y;
        precision zz = z * z;
        precision ii_jj_kk = ii + jj + kk;
        precision ix_jy_kz = ix + jy + kz;
        precision xx_yy_zz = xx + yy + zz;
        precision xx_yy_zz_rr_qq = xx_yy_zz - rr_qq;
        // t^4 terms: i^4 + j^4 + k^4 + 2(i^2j^2 + i^2+k^2 + j^2+k^2)
        // simplified: (ii + jj + kk)^2
        precision a = ii_jj_kk * ii_jj_kk;
        // t^3 terms: 4*t^3(xi^3 + yj^3 + zk^3 + i^2(j+k) + j^2(i+k) + k^2(j+i))
        // simplified: 4*ii_jj_kk*ix_jy_kz
        precision b = 4.0 * ii_jj_kk * ix_jy_kz;
        // t^2 terms: 2*(i^2(q^2-r^2) + j^2(q^2-r^2) + k^2(q^2-r^2)
        //               + x^2(3i^2+j^2+k^) + y^2(i^2+3j^2+k^2) + z^2(i^2+j^+3k^2)
        //               + 4(ijxy + ikxz + jkyz))
        // simplified: 2*ii_jj_kk*(xx_yy_zz+rr_qq) +
        //             4*ix_jy_kz*ix_jy_kz + 4*qq*kk
        precision c = 2.0 * ii_jj_kk * xx_yy_zz_rr_qq + 4.0 * ix_jy_kz * ix_jy_kz;
        // t^1 terms: 4*(ix(q^2-r^2)+jy(q^2-r^2)+kz(q^2-r^2) + ix^3 + jy^3 + kz^3
        //               + x^2(jy+kz) + y^2(ix+kz) + z^2(ix+jy))
        // simplified: 4*(xx_yy_zz - rr_qq)*ix_jy_kz + 8.0*qq*kz
        precision d = 4.0 * xx_yy_zz_rr_qq * ix_jy_kz;
        // t^0 terms; q^4 + r^4 + x^4 + y^4 + z^4
        //            + 2q^2(x^2+y^2+z^2) - 2r^2(q^2+x^2+y^2+z^2)
        //            + 2(x^2y^2 + x^2z^2 + y^2z^2)
        // simplified: (xx_yy_zz - rr_qq)(xx_yy_zz - rr_qq) - 4*qq*(rr - zz)
        precision e = xx_yy_zz_rr_qq * xx_yy_zz_rr_qq;
        // corrections from the other side of the squaring
        // c -= 4*qq*(ii + jj);
        c += 4.0 * qq * kk;
        // d -= 8*qq*(ix + jy);
        d += 8.0 * qq * kz;
        // e -= 4*qq*(xx + yy);
        e -= 4 * qq * (rr - zz);
        // now the terms are in the format needed (ax^4+bx^3+cx^2+dx+e = 0)
        auto roots = quartic_roots(a, b, c, d, e);
        precision r0 = std::get<0>(roots);
        precision r1 = std::get<1>(roots);
        precision r2 = std::get<2>(roots);
        precision r3 = std::get<3>(roots);
        point p0 = object_ray.distance_along(r0);
        point p1 = object_ray.distance_along(r1);
        point p2 = object_ray.distance_along(r2);
        point p3 = object_ray.distance_along(r3);
        ts.emplace_back(intersection{p0}, r0, normal_(p0), this);
        ts.emplace_back(intersection{p1}, r1, normal_(p1), this);
        ts.emplace_back(intersection{p2}, r2, normal_(p2), this);
        ts.emplace_back(intersection{p3}, r3, normal_(p3), this);
    }
    return ts;
}

bool torus::is_surface_point(point const& world_point) const {
    // (sqrt(x*x + y*y) + R)^2 +z*z = r*r
    point object_point = reverse_transform(world_point);
    precision x = object_point.x;
    precision y = object_point.y;
    precision z = object_point.z;
    precision R = m_tube_radius;
    precision r = m_ring_radius;
    precision sqrt_xx_yy = std::sqrt((x * x) + (y * y));
    precision sqrt_xx_yy_R_zz = ((sqrt_xx_yy + R) * (sqrt_xx_yy + R)) + (z * z);
    return basal::nearly_equals(r * r, sqrt_xx_yy_R_zz);
}

image::point torus::map(point const& object_surface_point) const {
    // (Torus) texture mapping a torus is hard but not impossible. define the mapping as a set of 2 angles,
    // one around the Z axis and another around the edge of the ring at that position.
    // The mapping has a "seem" along the inner edge of the torus, closest to the origin and at -X
    precision x = object_surface_point.x;
    precision y = object_surface_point.y;
    precision z = object_surface_point.z;

    // angle around Z "turns"
    precision t = std::atan2(y, x);
    // radius in the xy plane
    precision r_xy = std::sqrt((x * x) + (y * y));
    // imagine a plane formed by the line to the point from origin, cross the Z axis, this is the radial plane or RZ
    // distance from the point to the tube center circle (could be negative)
    precision d_rz = r_xy - m_ring_radius;
    // angle around the tube of the torus
    precision b = std::atan2(z, d_rz);
    // normalize the turns to 0 - 1 inclusive instead of -pi to +pi
    precision u = (t + iso::pi) / iso::tau;
    precision v = (b + iso::pi) / iso::tau;
    return image::point(u, v);
}

void torus::print(char const str[]) const {
    std::cout << str << " torus @" << this << " " << position() << " Inner Radius" << m_tube_radius
              << " Ring Radius:" << m_ring_radius << std::endl;
}

precision torus::get_object_extent(void) const {
    return m_ring_radius + m_tube_radius;
}

}  // namespace objects
}  // namespace raytrace
