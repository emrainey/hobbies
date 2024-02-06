#include "raytrace/objects/torus.hpp"

#include <iostream>

namespace raytrace {
namespace objects {
using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

torus::torus(const point& C, precision ring_radius, precision tube_radius)
    : object{C, 4, true}  // up to 4 collisions, closed surface
    , m_ring_radius{ring_radius}
    , m_tube_radius{tube_radius} {
    basal::exception::throw_if(tube_radius > ring_radius, __FILE__, __LINE__,
                               "Self-intersecting torus, tube must be smaller than ring radius");
}

vector torus::normal(const point& world_surface_point) const {
    point object_surface_point = reverse_transform(world_surface_point);
    // project along the XY plane
    vector ring_vector{{object_surface_point.x, object_surface_point.y, 0}};
    // normalize so we can then...
    ring_vector.normalize();
    // scale it to the ring
    ring_vector *= m_ring_radius;
    // make a ring point
    raytrace::point ring_point = R3::origin + ring_vector;
    // create a vector from the ring to the pnt
    raytrace::vector N = object_surface_point - ring_point;
    N.normalize();
    raytrace::vector world_N = forward_transform(N);
    return vector(world_N);  // copy constructor output
}

hits torus::collisions_along(const ray& object_ray) const {
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
        ts.push_back(r0);
        ts.push_back(r1);
        ts.push_back(r2);
        ts.push_back(r3);
    }
    return ts;
}

bool torus::is_surface_point(const point& world_point) const {
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

image::point torus::map(const point& object_surface_point __attribute__((unused))) const {
    // FIXME (Torus) texture mapping a torus is hard but not impossible. define the mapping as a set of 2 angles,
    // one around the Z axis and another around the edge of the ring at that position.
    precision u = 0.0;
    precision v = 0.0;
    return image::point(u, v);
}

void torus::print(const char str[]) const {
    std::cout << str << " torus @" << this << " " << position() << " Inner Radius" << m_tube_radius
              << " Ring Radius:" << m_ring_radius << std::endl;
}

precision torus::get_object_extant(void) const {
    return m_ring_radius + m_tube_radius;
}

}  // namespace objects
}  // namespace raytrace
