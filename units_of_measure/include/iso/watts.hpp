
#pragma once

#include "iso/measurement.hpp"

DEFINE_MEASUREMENT_CLASS(watts, double, SI::prefix, W)

inline iso::watts operator""_mW(long double a) { return iso::watts(a * SI::prefix::milli); }
inline iso::watts operator""_mW(const char a[]) { return iso::watts(atol(a) * SI::prefix::milli); }
