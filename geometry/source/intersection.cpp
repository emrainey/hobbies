
#include "geometry/intersection.hpp"

#include <basal/basal.hpp>
#include <cassert>
#include <cmath>

#include "geometry/line.hpp"
#include "geometry/point.hpp"

namespace geometry {

using namespace geometry::operators;
using namespace linalg;
using namespace linalg::operators;

const char *get_type_as_str(IntersectionType obj) {
    static const char *names[] = {"NONE", "POINT", "SET OF POINTS", "LINE", "PLANE"};
    return names[basal::to_underlying(obj)];
}

IntersectionType get_type(const intersection &var) {
    if (std::holds_alternative<geometry::R3::point>(var)) {
        return IntersectionType::Point;
    } else if (std::holds_alternative<set_of_points>(var)) {
        return IntersectionType::Points;
    } else if (std::holds_alternative<geometry::R3::line>(var)) {
        return IntersectionType::Line;
    } else if (std::holds_alternative<geometry::plane>(var)) {
        return IntersectionType::Plane;
    } else {
        return IntersectionType::None;
    }
}

std::ostream &operator<<(std::ostream &os, const intersection &intersector) {
    IntersectionType type = get_type(intersector);
    os << " Intersection Type: " << get_type_as_str(type) << std::endl;
    os << "\tIntersection => ";
    if (type == IntersectionType::Point) {
        os << as_point(intersector);
    } else if (type == IntersectionType::Points) {
        for (auto &pnt : as_points(intersector)) {
            os << " " << pnt;
        }
    } else if (type == IntersectionType::Line) {
        os << as_line(intersector);
    } else if (type == IntersectionType::Plane) {
        os << as_plane(intersector);
    }
    return os;
}

bool intersects(const point &pt, const R3::line &lin) {
    if (pt == lin.position()) {  // is it the point already on the line?
        return true;
    } else {
        // find the t which it does intersect or it will return false if it does not
        double t = 0.0;
        return lin.solve(pt, t);
    }
}

intersection intersects(const R3::line &L0, const R3::line &L1) {
    // print_this(L0);
    // print_this(L1);
    if (L0 == L1) {  // if same line
        return intersection(L1);
    } else if (L0.position() == L1.position()) {  // happen to have the same point
        return intersection(L1.position());
    } else if (L0 || L1) {  // happen to be parallel
        return intersection();
    } else if (skew(L0, L1)) {  // skew lines
        return intersection();
    } else {
        // from http://mathworld.wolfram.com/Line-LineIntersection.html
        R3::vector v = L1.position() - L0.position();
        R3::vector a = R3::cross(v, L1.direction());
        R3::vector b = R3::cross(L0.direction(), L1.direction());
        double s = dot(a, b) / b.quadrance();  // L0 variable
        return intersection(L0.solve(s));
    }
}

intersection intersects(const R3::line &l, const plane &P) {
    /// \see http://mathworld.wolfram.com/Line-PlaneIntersection.html
    if (P.contains(l)) {  // line is in the plane
        return intersection(l);
    } else if (orthogonal(P.normal, l.direction())) {  // parallel to plane
        return intersection();
    } else {
        R3::point a = l.solve(0.0);
        R3::point b = l.solve(1.0);
        double da = P.distance(a);
        double db = P.distance(b);
        double dd = (da > db ? da - db : db - da);  // distance between the points
        double tn = 0.0;
        // the plane can be below, above or between the points.
        // sometimes these are very close to zero but not zero
        if (basal::equals_zero(da)) {
            return intersection(a);
        }
        if (basal::equals_zero(db)) {
            return intersection(b);
        }
        if ((da > 0 && db < 0) || (da < 0 && db > 0)) {
            if (da > db) {
                tn = da / dd + 0.0;
            } else if (db > da) {
                tn = 1.0 - db / dd;
            }
        } else if (da > db) {
            // e.g. a = 27, b = 14
            tn = (db / dd) + 1.0;
        } else if (da < db) {
            // e.g. a = 14, b = 27
            tn = 0.0 - (da / dd);
        }
        point c = l.solve(tn);
        // basal::exception::throw_unless(P.contains(c), __FILE__, __LINE__);
        return intersection(c);
    }
}

intersection intersects(const plane &P1, const plane &P2) {
    if (P1 == P2) {  // same planes with opposite normals perhaps
        return intersection(P1);
    } else if (P1 || P2) {  // parallel
        return intersection();
    } else {
        R3::vector v = R3::cross(P1.unormal(), P2.unormal());
        // print_this(v);
        //  this is the [n1n2]^T
        matrix mt{{{P1.coefficient().a, P1.coefficient().b, P1.coefficient().c},
                   {P2.coefficient().a, P2.coefficient().b, P2.coefficient().c}}};
        matrix b{{{-P1.coefficient().d}, {-P2.coefficient().d}}};
        matrix mtb = rowjoin(mt, b);
        matrix nmtb = mtb.nullspace();
        R3::point pt{-nmtb[0][1], -nmtb[1][1], -nmtb[2][1]};  // pick value from nullspace
        R3::line iline(v, pt);
        return intersection(iline);
    }
}

intersection intersects(const R3::sphere &S, const R3::line &l) noexcept(false) {
    constexpr bool use_quadratic_roots = true;

    // get the point closest to the center (which is in world_coordinates?)
    R3::point P = l.closest(R3::origin);
    // get the vector from the p to the center.
    R3::vector pc = P - R3::origin;
    // get the distance of the vector
    double d = pc.magnitude();
    double r = S.radius;

    if (d > r) {
        // no intersection
        return intersection();
    } else if (basal::equals(d, r)) {
        // P is on the surface, single point of Intersection!
        return intersection(P);
    } else {
        if constexpr (use_quadratic_roots) {
            R3::vector v = l.direction();
            R3::point p = l.position();
            // x^2 + y^2 + z^2 = r^2
            // a = (vx^2 + vy^2 + vz^2)
            // b = (2vxpx + 2vypy + 2vzpz)
            // c = (px^2 + py^2 + pz^2) - r^2
            element_type a = 0;
            element_type b = 0;
            element_type c = 0;
            for (size_t i = 0; i < p.dimensions; i++) {
                a += v[i] * v[i];
                b += 2 * p[i] * v[i];
                c += p[i] * p[i];
            }
            c -= r * r;
            auto roots = quadratic_roots(a, b, c);
            element_type t0 = std::get<0>(roots);
            element_type t1 = std::get<1>(roots);
            if (not std::isnan(t0) and not std::isnan(t1)) {
                R3::point R = l.solve(t0);
                R3::point Q = l.solve(t1);
                set_of_points sop = {R, Q};
                return intersection(sop);
            } else {
                // single points don't concern us, so drop them.
                return intersection();
            }
        } else {  // deduce geometrically
            // there IS an intersection but there's two points!
            // more shortcuts
            R3::point R{3};
            R3::point Q{3};
            R3::point C = R3::origin;
            R3::point Z0 = l.position();

            // find the internal t to P to determine if the vector is
            // going towards or away from P
            element_type t = 0.0;
            assert(l.solve(P, t));
            // if t == 0 then P == Z0
            // if t < 0 then V is facing away from P
            // if t > 0 then V is facing towards P
            element_type u = (t < 0 ? -1.0 : 1.0);

            // line passes throught the center
            if (basal::equals_zero(d)) {
                // C == P
                // The closet point to the center is the center (passes through the middle)
                // get the distance to the center from the zero point
                R3::vector cz = C - Z0;
                double k = cz.magnitude();
                // we'll add a sign to k to indicate direction
                k *= u;
                R = l.distance_along(k - r);
                Q = l.distance_along(k + r);
                set_of_points sop = {R, Q};
                assert(S.surface(R));
                assert(S.surface(Q));
                return intersection(sop);
            } else {
                // C != P and d < r
                // Solve for R and Q (the two sphere point intersections)
                // distance from R/Q to P (c^2 - b^2 = a^2) where c is r and b is d
                double m = sqrt(r * r - d * d);
                // vector from P to the zero point
                R3::vector pz = Z0 - P;
                double k = pz.norm();

                // is the zero point of the line in the sphere?
                if (basal::equals_zero(k)) {
                    // zero point is at P
                    R = l.distance_along(-m);
                    Q = l.distance_along(+m);
                } else if (k < m) {
                    // the zero point is in the sphere but not P, R, or Q
                    R = l.distance_along(-u * (m - k));
                    Q = l.distance_along(u * (k + m));
                } else if (basal::equals(k, m)) {
                    // we'll call the one we're on R and the other Q
                    R = Z0;
                    Q = l.distance_along(2 * m * u);
                    assert(S.surface(Q));
                } else {  // k > m
                    // zero point is outside the sphere.
                    k *= u;
                    R = l.distance_along(k - m);
                    Q = l.distance_along(k + m);
                }
                set_of_points sop = {R, Q};
                assert(S.surface(R));
                assert(S.surface(Q));
                return intersection(sop);
            }
        }
    }
}

}  // namespace geometry
