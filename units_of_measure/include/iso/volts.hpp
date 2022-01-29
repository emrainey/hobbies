
#pragma once

#include "iso/measurement.hpp"

DEFINE_MEASUREMENT_CLASS(volts, double, SI::prefix, V)

// Specialized Quote Operators
iso::volts operator""_mV(long double a);
iso::volts operator""_mV(const char a[]);
