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
void convert(feet &f, const meters &m);
void convert(meters &m, const feet &f);

// Theres many different combinations of turns, radians, degrees
void convert(radians &r, const turns &t);
void convert(turns &t, const radians &r);
void convert(degrees &d, const radians &r);
void convert(radians &r, const degrees &d);

// Another conversion seconds <-> hertz
void convert(hertz &hz, const seconds &sec);
void convert(seconds &sec, const hertz &hz);

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
volts operator/(const watts &P, const amperes &I);
/// volts = amperes * ohms
volts operator*(const amperes &I, const ohms &R);
/// watts = volts * amperes
watts operator*(const volts &E, const amperes &I);
/// ohms = volts / amperes
ohms operator/(const volts &E, const amperes &I);
/// amperes = watts / volts
amperes operator/(const watts &P, const volts &E);
/// amperes = volts / ohms
amperes operator/(const volts &E, const ohms &R);
/// hertz = _num_ / seconds
hertz operator/(precision const num, const seconds &denom);
/// seconds = _num_ / hertz
seconds operator/(precision const num, const hertz &denom);

/// Creates a speed from a distance over time
speed operator/(const distance &num, const time &denom);

/// Creates a speed from a distance over time
speed operator/(distance &&num, time &&denom);

/// Creates a speed from a distance over time
speed operator/(const distance &num, time &&denom);

/// Creates a speed from a distance over time
speed operator/(distance &&num, const time &denom);

/// Alternative constructor
rate<feet, seconds> operator/(const feet &num, const seconds &denom);

/// Creates an acceleration from speed per time
acceleration operator/(const speed &num, const time &denom);

/// Creates an acceleration from speed per time
acceleration operator/(speed &&num, time &&denom);

/// Creates an acceleration from speed per time
acceleration operator/(const speed &num, time &&denom);

/// Creates an acceleration from speed per time
acceleration operator/(speed &&num, const time &denom);

/// Compounds a newtons-meters together into a torque
torque operator*(const newtons &N, const meters &m);

/// Compounds a newtons-meters together into a torque
torque operator*(newtons &&N, meters &&m);

/// Compounds a newtons-meters together into a torque
torque operator*(const newtons &N, meters &&m);

/// Compounds a newtons-meters together into a torque
torque operator*(newtons &&N, const meters &m);

// This is to override specific relations where things ARE equal

/// Compares Joules to Torque
bool operator==(const joules &J, const torque &T);
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

/// PI accurate to 200 digits
constexpr precision pi = static_cast<precision>(compute_pi(200));
/// 2*PI
constexpr precision tau = static_cast<precision>(2.0) * pi;

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
}  // namespace iso
