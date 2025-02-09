/// @file
/// The ISO units of measure header
/// @copyright Copyright (c) 2022
#pragma once
#include <cmath>
#include <cstdint>
#include <iostream>

#include "iec/prefix.hpp"
#include "iso/amperes.hpp"
#include "iso/compound.hpp"
#include "iso/degrees.hpp"
#include "iso/feet.hpp"
#include "iso/grams.hpp"
#include "iso/hertz.hpp"
#include "iso/joules.hpp"
#include "iso/meters.hpp"
#include "iso/newtons.hpp"
#include "iso/ohms.hpp"
#include "iso/pascals.hpp"
#include "iso/radians.hpp"
#include "iso/rate.hpp"
#include "iso/seconds.hpp"
#include "iso/turns.hpp"
#include "iso/volts.hpp"
#include "iso/watts.hpp"

namespace iso {

// feet <=> meters
feet convert(meters &A);
meters convert(feet &B);
// radians <=> turns
radians convert(turns &A);
turns convert(radians &A);
// seconds <=> cycles
hertz convert(seconds &A);
seconds convert(hertz &B);

// slightly different conversions interface
void convert(feet &f, meters const& m);
void convert(meters &m, feet const& f);

// Theres many different combinations of turns, radians, degrees
void convert(radians &r, turns const& t);
void convert(turns &t, radians const& r);
void convert(degrees &d, radians const& r);
void convert(radians &r, degrees const& d);

// Another conversion seconds <-> hertz
void convert(hertz &hz, seconds const& sec);
void convert(seconds &sec, hertz const& hz);

/// Define the standard distance as meters
using distance = meters;
/// Define the standard time as seconds
using time = seconds;

// Once your system has a basis of units you can assign these concepts

/// Define speed as a rate of distance in time
using speed = rate<distance, time>;
/// Define acceleration as a rate of speed in time
using acceleration = rate<speed, time>;
/// Define jerk as an acceleration in time
using jerk = rate<acceleration, time>;
/// Define snap as a rate of jerk in time
using snap = rate<jerk, time>;
/// Define crackle as a rate of snap in time
using crackle = rate<snap, time>;
/// Define a pop as a rate of crackle in time
using pop = rate<crackle, time>;
/// A torque is a compound of newtons-meters
using torque = compound<newtons, meters>;

namespace operators {
/// volts = watts / amperes
volts operator/(watts const& P, amperes const& I);
/// volts = amperes * ohms
volts operator*(amperes const& I, ohms const& R);
/// watts = volts * amperes
watts operator*(volts const& E, amperes const& I);
/// ohms = volts / amperes
ohms operator/(volts const& E, amperes const& I);
/// amperes = watts / volts
amperes operator/(watts const& P, volts const& E);
/// amperes = volts / ohms
amperes operator/(volts const& E, ohms const& R);
/// hertz = _num_ / seconds
hertz operator/(precision const num, seconds const& denom);
/// seconds = _num_ / hertz
seconds operator/(precision const num, hertz const& denom);

/// Creates a speed from a distance over time
speed operator/(distance const& num, time const& denom);

/// Creates a speed from a distance over time
speed operator/(distance &&num, time &&denom);

/// Creates a speed from a distance over time
speed operator/(distance const& num, time &&denom);

/// Creates a speed from a distance over time
speed operator/(distance &&num, time const& denom);

/// Alternative constructor
rate<feet, seconds> operator/(feet const& num, seconds const& denom);

/// Creates an acceleration from speed per time
acceleration operator/(speed const& num, time const& denom);

/// Creates an acceleration from speed per time
acceleration operator/(speed &&num, time &&denom);

/// Creates an acceleration from speed per time
acceleration operator/(speed const& num, time &&denom);

/// Creates an acceleration from speed per time
acceleration operator/(speed &&num, time const& denom);

/// Compounds a newtons-meters together into a torque
torque operator*(newtons const& N, meters const& m);

/// Compounds a newtons-meters together into a torque
torque operator*(newtons &&N, meters &&m);

/// Compounds a newtons-meters together into a torque
torque operator*(newtons const& N, meters &&m);

/// Compounds a newtons-meters together into a torque
torque operator*(newtons &&N, meters const& m);

// This is to override specific relations where things ARE equal

/// Compares Joules to Torque
bool operator==(joules const& J, torque const& T);
}  // namespace operators

/// A method of computing PI at compile time
constexpr long double compute_pi(size_t digits) {
    using namespace basal::literals;
    long double pi = 0.0_p;
    for (long double i = 0.0_p; i < digits; i += 1.0_p) {
        long double denum = 1.0_p;
        for (long double j = i; j > 0; j -= 1.0_p) {
            denum *= 16.0_p;
        }
        pi += ((4.0_p / (8.0_p * i + 1.0_p)) - (2.0_p / (8.0_p * i + 4.0_p)) - (1.0_p / (8.0_p * i + 5.0_p)) - (1.0_p / (8.0_p * i + 6.0_p)))
              / denum;
    }
    return pi;
}

/// 1/2 a turn around a circle in radian, accurate to 200 digits.
constexpr precision pi = static_cast<precision>(compute_pi(200));
/// 1 turn around a circle in radians, also known as 2 * pi
constexpr precision tau = static_cast<precision>(2.0) * pi;
/// Natural Log e of 2
constexpr precision ln2 = 0.6931471805599453094;
/// Inverse of ln2
constexpr precision invln2 = 1.0 / ln2;

/// The namespace where the quote operators are stored. You must `using namespace literals;` to use them
namespace literals {
/// A quote operator for making speeds
speed operator""_m_per_sec(long double a);

/// Constructs an acceleration from a double
acceleration operator""_G(long double g);

/// Specialized Quote Operator
iso::amperes operator""_mA(long double a);
/// Specialized Quote Operator
iso::amperes operator""_mA(char const a[]);
/// Specialized Quote Operator
iso::ohms operator""_mOhm(long double a);
/// Specialized Quote Operator
iso::ohms operator""_mOhm(char const a[]);
/// Specialized Quote Operator
iso::ohms operator""_KOhm(long double a);
/// Specialized Quote Operator
iso::ohms operator""_KOhm(char const a[]);
/// Specialized Quote Operator
iso::volts operator""_mV(long double a);
/// Specialized Quote Operator
iso::volts operator""_mV(char const a[]);
/// Specialized Quote Operator
iso::watts operator""_mW(long double a);
/// Specialized Quote Operator
iso::watts operator""_mW(char const a[]);
}  // namespace literals

/// The speed of causality in a vacuum
const speed c{meters{299'792'458.0}, seconds{1.0}};

/// Converts a frequency of light into a wavelength in meters
inline meters wavelength(hertz const& f) {
    return meters{c.value / f.value};
}

/// Converts a wavelength in meters to a frequency of light.
inline hertz frequency(meters const& l) {
    return hertz{c.value / l.value};
}

}  // namespace iso
