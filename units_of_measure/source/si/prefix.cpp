
#include <cmath>

#include "si/prefix.hpp"

double operator*(const double a, SI::prefix p) {
    return a * pow(10.0,static_cast<double>(p));
}
