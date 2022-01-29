
#pragma once

#include "iso/measurement.hpp"

DEFINE_MEASUREMENT_CLASS(turns, double, SI::prefix, Tau) // can't use Tau mark ASCII 231

namespace iso {

    constexpr long double compute_pi(size_t digits) {
        long double pi = 0.0;
        for (long double i = 0.0; i < digits; i+=1.0) {
            long double denum = 1.0;
            for (long double j = i; j > 0; j-=1.0) {
                denum *= 16.0;
            }
            pi += ((4.0/(8.0*i + 1.0)) -
                   (2.0/(8.0*i + 4.0)) -
                   (1.0/(8.0*i + 5.0)) -
                   (1.0/(8.0*i + 6.0))) /
                   denum;
        }
        return pi;
    }

    // constexpr double tau = 6.283185307179586'4769252867665590057683943387987502116419498891846156328125724179972560696506842341359;
                            //6.283185307179586'231995926937088370323181152343750
    // constexpr double pi  = tau / 2.0;
    constexpr double pi = compute_pi(200);
    constexpr double tau = 2.0 * pi;
}
