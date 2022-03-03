#include "linalg/solvers.hpp"

#include <complex>
#include <iostream>

namespace linalg {

constexpr static bool root_debug = false;

/** in order to save some typing just a shorter definition of complex numbers */
using complex_ = std::complex<element_type>;

std::tuple<element_type, element_type> quadratic_roots(element_type a, element_type b, element_type c) {
    statistics::get().quadratic_roots++;
    if constexpr (root_debug) {
        std::cout << "Quadratic Coefficients a=" << a << ", b=" << b << ", c=" << c << std::endl;
    }
    if (basal::equals_zero(a)) {
        return std::make_tuple(std::nan(""), std::nan(""));
    }
    auto i = (b * b) - (4 * a * c);
    if (i >= 0.0) {
        auto p = (-b + sqrt(i)) / (2 * a);
        auto q = (-b - sqrt(i)) / (2 * a);
        return std::make_tuple(p, q);
    } else {
        // Requires Complex Numbers
        return std::make_tuple(std::nan(""), std::nan(""));
    }
}

std::tuple<element_type, element_type, element_type> cubic_roots(element_type a, element_type b, element_type c,
                                                                 element_type d) {
    statistics::get().cubic_roots++;
    if (basal::equals_zero(a)) {
        // not a valid case
        return std::make_tuple(std::nan(""), std::nan(""), std::nan(""));
    }
    if constexpr (root_debug) {
        std::cout << "Cubic Coefficients a=" << a << ", b=" << b << ", c=" << c << ", d=" << d << std::endl;
    }
    // reduce b,c,d by a to do less math overall
    b /= a;
    c /= a;
    d /= a;
    // we'll apply this at the end again to move the roots back
    // to the right place. the equations assume that the cubic is shifted
    // such that the inflection is on x=0
    element_type shift = -b / 3;
    // now that a = 1 these are much simpler
    complex_ Q = (3.0 * c - b * b) / 9.0;
    complex_ R = (b * (9.0 * c - (2.0 * b * b)) - 27.0 * d) / 54.0;
    // this is the discriminant which tells us how may solutions there are
    complex_ D = Q * Q * Q + R * R;
    if constexpr (root_debug) {
        std::cout << "Q: " << Q << std::endl;
        std::cout << "R: " << R << std::endl;
        std::cout << "D: " << D << std::endl;
    }
    element_type x1 = std::nan("");
    element_type x2 = std::nan("");
    element_type x3 = std::nan("");
    complex_ X1, X2, X3;
    if (D.real() < 0) {
        complex_ S_ = R + sqrt(D);
        complex_ T_ = R - sqrt(D);
        auto S3 = cbrt(S_);  // returns 3 roots
        auto T3 = cbrt(T_);  // returns 3 roots
        if constexpr (root_debug) {
            std::cout << "one real and two complex (which does cross y=0)" << std::endl;
            std::cout << "S0: " << std::get<0>(S3) << std::endl;
            std::cout << "S1: " << std::get<1>(S3) << std::endl;
            std::cout << "S2: " << std::get<2>(S3) << std::endl;
            std::cout << "T0: " << std::get<0>(T3) << std::endl;
            std::cout << "T1: " << std::get<1>(T3) << std::endl;
            std::cout << "T2: " << std::get<2>(T3) << std::endl;
        }
        X1 = std::get<0>(S3) + std::get<0>(T3);  // real?
        X2 = std::get<2>(S3) + std::get<1>(T3);  // swap conj pairs
        X3 = std::get<1>(S3) + std::get<2>(T3);  // swap conj pairs
        x1 = (not basal::equals_zero(X1.imag()) ? std::nan("") : (X1.real() + shift));
        x2 = (not basal::equals_zero(X2.imag()) ? std::nan("") : (X2.real() + shift));
        x3 = (not basal::equals_zero(X3.imag()) ? std::nan("") : (X3.real() + shift));
    } else if (basal::equals_zero(D.real())) {
        auto R3 = cbrt(R);
        if constexpr (root_debug) {
            std::cout << "Either 2 real or at the zero inflection" << std::endl;
            std::cout << "R0: " << std::get<0>(R3) << std::endl;
            std::cout << "R1: " << std::get<1>(R3) << std::endl;
            std::cout << "R2: " << std::get<2>(R3) << std::endl;
        }
        X1 = 2.0 * std::get<0>(R3);
        X2 = -std::get<1>(R3);
        X3 = -std::get<2>(R3);
        x1 = X1.real() + shift;
        x2 = X2.real() + shift;
        x3 = X3.real() + shift;
    } else if (D.real() > 0) {
        element_type S_ = R.real() + sqrt(D.real());
        element_type T_ = R.real() - sqrt(D.real());
        element_type S = std::cbrt(S_);
        element_type T = std::cbrt(T_);
        if constexpr (root_debug) {
            std::cout << "one real and two complex (which does not cross y=0)" << std::endl;
            std::cout << "S: " << S << std::endl;
            std::cout << "T: " << T << std::endl;
        }
        // the easiest but incomplete solution is to ignore the imaginary parts alltogether.
        X1 = S + T;
        x1 = X1.real() + shift;
    }
    if constexpr (root_debug) {
        std::cout << "Cubic Complex Roots (no shift): X1=" << X1 << ", X2=" << X2 << ", X3=" << X3 << std::endl;
        std::cout << "Cubic Real Roots: x1=" << x1 << ", x2=" << x2 << ", x3=" << x3 << std::endl;
    }
    basal::exception::throw_if(std::isnan(x1) and std::isnan(x2) and std::isnan(x3), __FILE__, __LINE__,
                               "Cubics always have at least 1 solution");
    return std::make_tuple(x1, x2, x3);
}

std::tuple<element_type, element_type, element_type, element_type> quartic_roots(element_type a, element_type b,
                                                                                 element_type c, element_type d,
                                                                                 element_type e) {
    using namespace std::literals::complex_literals;
    statistics::get().quartic_roots++;
    if constexpr (root_debug) {
        std::cout << "Quartic Coefficients: a=" << a << ", b=" << b << ", c=" << c << ", d=" << d << ", e=" << e
                  << std::endl;
    }
    if (basal::equals_zero(a)) {
        return std::make_tuple(std::nan(""), std::nan(""), std::nan(""), std::nan(""));
    }
    a /= a;
    b /= a;
    c /= a;
    d /= a;
    e /= a;
    element_type x1, x2, x3, x4;
    x1 = x2 = x3 = x4 = std::nan("");

    /** @internal A method based on Herbert E. Salzer "A Note on the Solution of Quartic Equations"
     * (Am. Math Society Proceedings, 1959).
     */

    // solve the cubic to find a good value of z. this is the "resolvent cubic equation"
    element_type c1 = b * d - 4 * e;
    element_type d1 = e * (4 * c - b * b) - d * d;
    auto cr = cubic_roots(1.0, -c, c1, d1);
    // how to pick which root? any root? we'll pick left first then eliminate if NaN.
    element_type z = std::get<0>(cr);
    z = std::isnan(z) ? std::get<1>(cr) : z;
    z = std::isnan(z) ? std::get<2>(cr) : z;
    complex_ mm = 0.25 * b * b - c + z;
    complex_ m, n;
    if constexpr (root_debug) {
        std::cout << "z=" << z << ", m^2=" << mm << std::endl;
    }
    m = std::sqrt(mm);
    if (real(mm) > 0) {
        n = 0.25 * (b * z - 2 * d) / m;
    } else if (basal::equals_zero(real(mm))) {
        n = (0.25 * z * z - e) + 0.0i;
        n = std::sqrt(n);
    } else {
        // all imaginary roots
        if constexpr (root_debug) {
            std::cout << "Quartic roots are all imaginary. Returning NaNs" << std::endl;
        }
        return std::make_tuple(std::nan(""), std::nan(""), std::nan(""), std::nan(""));
    }
    if constexpr (root_debug) {
        std::cout << "m=" << m << ", n=" << n << std::endl;
    }
    complex_ alpha = 0.5 * b * b - z - c;
    complex_ beta = 4.0 * n - b * m;
    if constexpr (root_debug) {
        std::cout << "α=" << alpha << ", β=" << beta << std::endl;
    }
    complex_ alpha_plus_beta = alpha + beta;
    complex_ alpha_minus_beta = alpha - beta;
    complex_ gamma = std::sqrt(alpha_plus_beta);
    complex_ delta = std::sqrt(alpha_minus_beta);
    if constexpr (root_debug) {
        std::cout << "γ=" << gamma << ", δ=" << delta << std::endl;
    }
    complex_ X1 = (-b * 0.5 + m + gamma) * 0.5;
    complex_ X2 = (-b * 0.5 - m + delta) * 0.5;
    complex_ X3 = (-b * 0.5 + m - gamma) * 0.5;
    complex_ X4 = (-b * 0.5 - m - delta) * 0.5;

    if (alpha_plus_beta.real() >= 0) {
        x1 = std::isinf(X1.real()) ? std::nan("") : X1.real();
        x3 = std::isinf(X3.real()) ? std::nan("") : X3.real();
    }
    if (alpha_minus_beta.real() >= 0) {
        x2 = std::isinf(X2.real()) ? std::nan("") : X2.real();
        x4 = std::isinf(X4.real()) ? std::nan("") : X4.real();
    }
    if constexpr (root_debug) {
        std::cout << "Quartic Complex Roots: X1=" << X1 << ", X2=" << X2 << ", X3=" << X3 << ", X4=" << X4 << std::endl;
        std::cout << "Quartic Real Roots: x1=" << x1 << ", x2=" << x2 << ", x3=" << x3 << ", x4=" << x4 << std::endl;
    }
    return std::make_tuple(x1, x2, x3, x4);
}

}  // namespace linalg
