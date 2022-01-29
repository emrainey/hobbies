#pragma once
#include <cstdint>
#include <iostream>
#include <cmath>

#include "iec/prefix.hpp"

#include "iso/amperes.hpp"
#include "iso/volts.hpp"
#include "iso/ohms.hpp"
#include "iso/watts.hpp"

#include "iso/meters.hpp"
#include "iso/feet.hpp"

#include "iso/grams.hpp"

#include "iso/joules.hpp"
#include "iso/newtons.hpp"
#include "iso/pascals.hpp"

#include "iso/radians.hpp"
#include "iso/turns.hpp"
#include "iso/degrees.hpp"

#include "iso/seconds.hpp"
#include "iso/hertz.hpp"

#include "iso/rate.hpp"
#include "iso/compound.hpp"

namespace iso {
    // Following volts = amperes * ohms, watts = volts * amperes
    // Equational operators
    volts operator/(const watts &P, const amperes &I);
    volts operator*(const amperes &I, const ohms &R);

    watts operator*(const volts &E, const amperes &I);

    ohms operator/(const volts &E, const amperes &I);

    amperes operator/(const watts &P, const volts &E);
    amperes operator/(const volts &E, const ohms &R);

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
    void convert(feet& f, const meters& m);
    void convert(meters& m, const feet& f);
    void convert(radians& r, const turns& t);
    void convert(turns& t, const radians& r);
    void convert(degrees& d, const radians& r);
    void convert(radians& r, const degrees& d);
    void convert(hertz& hz, const seconds &sec);
    void convert(seconds& sec, const hertz &hz);

    // additional conversions for those in the know about the relationship

    hertz operator/(const double num, const seconds& denom);
    seconds operator/(const double num, const hertz& denom);

    // Once your system has a basis of units you can assign these concepts

    /** Define the standard distance as meters */
    using distance = meters;
    /** Define the standard time as seconds */
    using time = seconds;

    /** Define speed as a rate of distance in time */
    using speed = rate<distance,time>;
    /** Define acceleration as a rate of speed in time */
    using acceleration = rate<speed,time>;
    /** Define jerk as an acceleration in time */
    using jerk = rate<acceleration,time>;
    /** Define snap as a rate of jerk in time */
    using snap = rate<jerk,time>;
    /** Define crackle as a rate of snap in time */
    using crackle = rate<snap,time>;
    /** Define a pop as a rate of crackle in time */
    using pop = rate<crackle,time>;

    /** Creates a speed from a distance over time */
    speed operator/(const distance &num, const time &denom);

    /** Creates a speed from a distance over time */
    speed operator/(distance &&num, time &&denom);

    /** Creates a speed from a distance over time */
    speed operator/(const distance &num, time &&denom);

    /** Creates a speed from a distance over time */
    speed operator/(distance &&num, const time &denom);

    /** Alternative constructor */
    rate<feet,seconds> operator/(const feet &num, const seconds &denom);

    /** A quote operator for making speeds */
    speed operator""_m_per_sec(long double a);

    /** Creates an acceleration from speed per time */
    acceleration operator/(const speed &num, const time &denom);

    /** Creates an acceleration from speed per time */
    acceleration operator/(speed &&num, time &&denom);

    /** Creates an acceleration from speed per time */
    acceleration operator/(const speed &num, time &&denom);

    /** Creates an acceleration from speed per time */
    acceleration operator/(speed &&num, const time &denom);

    /** Constructs an acceleration from a double */
    acceleration operator""_G(long double g);

    /** A torque is a compound of newtons-meters */
    using torque = compound<newtons,meters>;

    /** Compounds a newtons-meters together into a torque */
    torque operator*(const newtons &N, const meters &m);

    /** Compounds a newtons-meters together into a torque */
    torque operator*(newtons &&N, meters &&m);

    /** Compounds a newtons-meters together into a torque */
    torque operator*(const newtons &N, meters &&m);

    /** Compounds a newtons-meters together into a torque */
    torque operator*(newtons &&N, const meters &m);

    // This is to override specific relations where things ARE equal

    /** Compares Joules to Torque */
    bool operator==(const joules &J, const torque &T);
}
