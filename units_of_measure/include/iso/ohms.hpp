
#pragma once

#include "iso/measurement.hpp"

DEFINE_MEASUREMENT_CLASS(ohms, double, SI::prefix, Ohm) // can't use omega mark ASCII 234

iso::ohms operator""_mOhm(long double a);
iso::ohms operator""_mOhm(const char a[]);

iso::ohms operator""_KOhm(long double a);
iso::ohms operator""_KOhm(const char a[]);
