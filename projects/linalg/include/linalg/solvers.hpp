#pragma once
/// @file
/// This header is to declare cross object math operations
/// @copyright Copyright 2019 (C) Erik Rainey.

#include <cmath>
#include <complex>
#include <iso/iso.hpp>  // for PI
#include <tuple>

#include "linalg/linalg.hpp"

namespace linalg {
/// Solves for the quadratic roots via the quadratic formula.
/// Given: a*x^2+b*x+c=0, solves for x. A NaN is a complex solution.
std::tuple<precision, precision> quadratic_roots(precision a, precision b, precision c);

///
/// @todo Solves for cubic roots of a cubic formula:
/// Given: a*x^3+b*x^2+c*x+d=0, solves for x. A NaN in the tuple is a complex solution.
/// @see https://www.youtube.com/watch?v=N-KXStupwsc
/// @return Returns real solutions only. Solutions which would be complex are left as NaN. Solutions are left-packed.
///
std::tuple<precision, precision, precision> cubic_roots(precision a, precision b, precision c,
                                                                 precision d);

/// @todo Solves for quartic roots of a quartic formula:
/// Given: a*x^4+b*x^3+c*x^2+d*x^1+e=0, solves for x. A NaN in the tuple is a complex solution.
std::tuple<precision, precision, precision, precision> quartic_roots(precision a, precision b,
                                                                                 precision c, precision d,
                                                                                 precision e);

/// Returns true when m is within the range of low to hi (but not when equal)
constexpr bool within(precision low, precision m, precision hi) {
    return (low < m and m < hi);
}

/// Returns true when m is within the range of low to hi (but not when equal)
constexpr bool within_inclusive(precision low, precision m, precision hi) {
    return (low <= m and m <= hi);
}

///
/// Computes the 3 roots of a non-zero complex number and returns as a 3-tuple using
/// de'Moive's Theorem of integer roots of complex numbers.
/// @see https://socratic.org/questions/how-do-i-find-the-cube-root-of-a-complex-number
/// @return Returns in the order of k = 0, 1, 2 (see link)
///
template <typename T>
std::tuple<std::complex<T>, std::complex<T>, std::complex<T>> cbrt(std::complex<T>& v) {
    constexpr int n = 3;
    // convert to a polar notation and use the cbrt procedure there
    if constexpr (false) {
        std::cout << "r=" << abs(v) << ", phi=" << arg(v) << std::endl;
    }
    T r = std::cbrt(std::abs(v));
    T a1 = arg(v) / n;
    T a2 = a1 + 2 * iso::pi / n;
    T a3 = a1 + 4 * iso::pi / n;
    return std::make_tuple(std::polar(r, a1), std::polar(r, a2), std::polar(r, a3));
}
}  // namespace linalg
