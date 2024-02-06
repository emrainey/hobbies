#include "linalg/solvers.hpp"

#include <complex>
#include <iostream>

namespace linalg {

constexpr static bool root_debug = false;

/// in order to save some typing just a shorter definition of complex numbers
using complex_ = std::complex<precision>;

/// pull in basal's precision literals
using namespace basal::literals;

std::tuple<precision, precision> quadratic_roots(precision a, precision b, precision c) {
    statistics::get().quadratic_roots++;
    if constexpr (root_debug) {
        std::cout << "Quadratic Coefficients a=" << a << ", b=" << b << ", c=" << c << std::endl;
    }
    if (basal::nearly_zero(a)) {
        return std::make_tuple(basal::nan, basal::nan);
    }
    auto i = (b * b) - (4.0_p * a * c);
    if (i >= 0.0) {
        auto p = (-b + sqrt(i)) / (2.0_p * a);
        auto q = (-b - sqrt(i)) / (2.0_p * a);
        return std::make_tuple(p, q);
    } else {
        // Requires Complex Numbers
        return std::make_tuple(basal::nan, basal::nan);
    }
}

std::tuple<precision, precision, precision> cubic_roots(precision a, precision b, precision c,
                                                                 precision d) {
    statistics::get().cubic_roots++;
    if (basal::nearly_zero(a)) {
        // not a valid case
        return std::make_tuple(basal::nan, basal::nan, basal::nan);
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
    precision shift = -b / 3.0_p;
    // now that a = 1 these are much simpler
    complex_ Q = ((3.0_p * c) - (b * b)) / 9.0_p;
    complex_ R = ((b * ((9.0_p * c) - (2.0_p * b * b))) - (27.0_p * d)) / 54.0_p;
    // this is the discriminant which tells us how may solutions there are
    complex_ D = (Q * Q * Q) + (R * R);
    if constexpr (root_debug) {
        std::cout << "Q: " << Q << std::endl;
        std::cout << "R: " << R << std::endl;
        std::cout << "D: " << D << std::endl;
    }
    precision x1 = basal::nan;
    precision x2 = basal::nan;
    precision x3 = basal::nan;
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
        x1 = (not basal::nearly_zero(X1.imag()) ? basal::nan : (X1.real() + shift));
        x2 = (not basal::nearly_zero(X2.imag()) ? basal::nan : (X2.real() + shift));
        x3 = (not basal::nearly_zero(X3.imag()) ? basal::nan : (X3.real() + shift));
    } else if (basal::nearly_zero(D.real())) {
        auto R3 = cbrt(R);
        if constexpr (root_debug) {
            std::cout << "Either 2 real or at the zero inflection" << std::endl;
            std::cout << "R0: " << std::get<0>(R3) << std::endl;
            std::cout << "R1: " << std::get<1>(R3) << std::endl;
            std::cout << "R2: " << std::get<2>(R3) << std::endl;
        }
        X1 = 2.0_p * std::get<0>(R3);
        X2 = -std::get<1>(R3);
        X3 = -std::get<2>(R3);
        x1 = X1.real() + shift;
        x2 = X2.real() + shift;
        x3 = X3.real() + shift;
    } else if (D.real() > 0) {
        precision S_ = R.real() + sqrt(D.real());
        precision T_ = R.real() - sqrt(D.real());
        precision S = std::cbrt(S_);
        precision T = std::cbrt(T_);
        if constexpr (root_debug) {
            std::cout << "one real and two complex (which does not cross y=0)" << std::endl;
            std::cout << "S: " << S << std::endl;
            std::cout << "T: " << T << std::endl;
        }
        // the easiest but incomplete solution is to ignore the imaginary parts all together.
        X1 = S + T;
        x1 = X1.real() + shift;
    }
    if constexpr (root_debug) {
        std::cout << "Cubic Complex Roots (no shift): X1=" << X1 << ", X2=" << X2 << ", X3=" << X3 << std::endl;
        std::cout << "Cubic Real Roots: x1=" << x1 << ", x2=" << x2 << ", x3=" << x3 << std::endl;
    }
    basal::exception::throw_if(basal::is_nan(x1) and basal::is_nan(x2) and basal::is_nan(x3), __FILE__, __LINE__,
                               "Cubics always have at least 1 solution");
    return std::make_tuple(x1, x2, x3);
}

#if defined(USE_PRECISION_AS_FLOAT)
#warning "Using float precision for quartic_roots, most solutions will not work"
#endif

std::tuple<precision, precision, precision, precision> quartic_roots(precision a,
                                                                     precision b,
                                                                     precision c,
                                                                     precision d,
                                                                     precision e) {
    using namespace std::literals::complex_literals;
    statistics::get().quartic_roots++;
    if constexpr (root_debug) {
        std::cout << "Quartic Coefficients: a=" << a << ", b=" << b << ", c=" << c << ", d=" << d << ", e=" << e
                  << std::endl;
    }
    if (basal::nearly_zero(a)) {
        return std::make_tuple(basal::nan, basal::nan, basal::nan, basal::nan);
    }
    a /= a;
    b /= a;
    c /= a;
    d /= a;
    e /= a;
    precision x1, x2, x3, x4;
    x1 = x2 = x3 = x4 = basal::nan;

    /// @internal A method based on Herbert E. Salzer "A Note on the Solution of Quartic Equations"
    /// (Am. Math Society Proceedings, 1959).
    ///

    // solve the cubic to find a good value of z. this is the "resolvent cubic equation"
    precision c1 = (b * d ) - (4.0_p * e);
    precision d1 = (e * ((4.0_p * c) - (b * b))) - (d * d);
    auto cr = cubic_roots(1.0_p, -c, c1, d1);
    // how to pick which root? any root? we'll pick left first then eliminate if NaN.
    precision z = std::get<0>(cr);
    z = basal::is_nan(z) ? std::get<1>(cr) : z;
    z = basal::is_nan(z) ? std::get<2>(cr) : z;
    complex_ mm = (0.25_p * b * b) - c + z;
    complex_ m, n;
    if constexpr (root_debug) {
        std::cout << "z=" << z << ", m^2=" << mm << std::endl;
    }
    m = std::sqrt(mm);
    if (real(mm) > 0) {
        n = (0.25_p * ((b * z) - (2.0_p * d))) / m;
    } else if (basal::nearly_zero(real(mm))) {
        complex_ none{0.0, 0.0};
        n = ((0.25_p * z * z) - e) + none;
        n = std::sqrt(n);
    } else {
        // all imaginary roots
        if constexpr (root_debug) {
            std::cout << "Quartic roots are all imaginary. Returning NaNs" << std::endl;
        }
        return std::make_tuple(basal::nan, basal::nan, basal::nan, basal::nan);
    }
    if constexpr (root_debug) {
        std::cout << "m=" << m << ", n=" << n << std::endl;
    }
    complex_ alpha = (0.5_p * b * b) - z - c;
    complex_ beta = (4.0_p * n) - (b * m);
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
    complex_ X1 = ((-b * 0.5_p) + m + gamma) * 0.5_p;
    complex_ X2 = ((-b * 0.5_p) - m + delta) * 0.5_p;
    complex_ X3 = ((-b * 0.5_p) + m - gamma) * 0.5_p;
    complex_ X4 = ((-b * 0.5_p) - m - delta) * 0.5_p;

    if (alpha_plus_beta.real() >= 0) {
        x1 = std::isinf(X1.real()) ? basal::nan : X1.real();
        x3 = std::isinf(X3.real()) ? basal::nan : X3.real();
    }
    if (alpha_minus_beta.real() >= 0) {
        x2 = std::isinf(X2.real()) ? basal::nan : X2.real();
        x4 = std::isinf(X4.real()) ? basal::nan : X4.real();
    }
    if constexpr (root_debug) {
        std::cout << "Quartic Complex Roots: X1=" << X1 << ", X2=" << X2 << ", X3=" << X3 << ", X4=" << X4 << std::endl;
        std::cout << "Quartic Real Roots: x1=" << x1 << ", x2=" << x2 << ", x3=" << x3 << ", x4=" << x4 << std::endl;
    }
    return std::make_tuple(x1, x2, x3, x4);
}

}  // namespace linalg
