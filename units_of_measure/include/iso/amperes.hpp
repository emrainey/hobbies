
#pragma once

#include "iso/measurement.hpp"

DEFINE_MEASUREMENT_CLASS(amperes, double, SI::prefix, A)

// Specialized Quote Operators
iso::amperes operator""_mA(long double a);
iso::amperes operator""_mA(const char a[]);
