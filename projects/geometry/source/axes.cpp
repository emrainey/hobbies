
#include "geometry/axes.hpp"

#include <cassert>
#include <cmath>

namespace geometry {

namespace R2 {

axes::axes(R2::point const& origin, R2::vector const& abscissa, R2::vector const& ordinate)
    : origin_{origin}, abscissa_{abscissa}, ordinate_{ordinate} {
    basal::exception::throw_if(abscissa == R2::null, __FILE__, __LINE__, "Abscissa can't be null");
    basal::exception::throw_if(ordinate == R2::null, __FILE__, __LINE__, "Ordinate can't be null");
    basal::exception::throw_unless(is_basis(), __FILE__, __LINE__,
                                   "Abscissa and ordinate must be mutually perpendicular");
}
R2::point const& axes::origin() const {
    return origin_;
}
R2::vector const& axes::abscissa() const {
    return abscissa_;
}
R2::vector const& axes::ordinate() const {
    return ordinate_;
}
R2::point const& axes::origin(R2::point const& origin) {
    origin_ = origin;
    return origin_;
}

R2::vector const& axes::abscissa(R2::vector const& abscissa) {
    abscissa_ = abscissa;
    return abscissa_;
}

R2::vector const& axes::ordinate(R2::vector const& ordinate) {
    ordinate_ = ordinate;
    return ordinate_;
}

bool axes::is_basis() const {
    return forward().determinant() > 0.0_p;  // check if the determinant is positive
}

matrix axes::forward() const {
    // create a 2x2 transform, without translation
    return matrix{{{abscissa()[0], ordinate()[0]}, {abscissa()[1], ordinate()[1]}}};
}

matrix axes::backwards() const {
    return forward().inverse();
}

}  // namespace R2

namespace R3 {
axes::axes(R3::point const& origin, R3::vector const& abscissa, R3::vector const& ordinate, R3::vector const& applicate)
    : origin_{origin}, abscissa_{abscissa}, ordinate_{ordinate}, applicate_{applicate} {
    basal::exception::throw_if(abscissa == R3::null, __FILE__, __LINE__, "Abscissa can't be null");
    basal::exception::throw_if(ordinate == R3::null, __FILE__, __LINE__, "Ordinate can't be null");
    basal::exception::throw_if(applicate == R3::null, __FILE__, __LINE__, "Applicate can't be null");
    basal::exception::throw_unless(is_basis(), __FILE__, __LINE__,
                                   "Abscissa, ordinate and applicate must be mutually perpendicular");
}

R3::point const& axes::origin() const {
    return origin_;
}
R3::vector const& axes::abscissa() const {
    return abscissa_;
}
R3::vector const& axes::ordinate() const {
    return ordinate_;
}
R3::vector const& axes::applicate() const {
    return applicate_;
}

R3::point const& axes::origin(R3::point const& origin) {
    origin_ = origin;
    return origin_;
}
R3::vector const& axes::abscissa(R3::vector const& abscissa) {
    abscissa_ = abscissa;
    return abscissa_;
}
R3::vector const& axes::ordinate(R3::vector const& ordinate) {
    ordinate_ = ordinate;
    return ordinate_;
}
R3::vector const& axes::applicate(R3::vector const& applicate) {
    applicate_ = applicate;
    return applicate_;
}

bool axes::is_basis() const {
    return forward().determinant() > 0.0_p;  // check if the determinant is positive
}

matrix axes::forward() const {
    matrix m{3, 3};
    // create a 3x3 transform, without translation
    return matrix{{{abscissa()[0], ordinate()[0], applicate()[0]},
                   {abscissa()[1], ordinate()[1], applicate()[1]},
                   {abscissa()[2], ordinate()[2], applicate()[2]}}};
}

matrix axes::backwards() const {
    return forward().inverse();
}

}  // namespace R3

}  // namespace geometry
