
#include <gtest/gtest.h>
#include "iso/iso.hpp"

TEST(IsoTest, PITest) {
    double pi = std::atan(1.0) * 4.0;
    auto scos = [](double a) -> double {
        return std::sin(a - (iso::pi/2));
    };
    EXPECT_DOUBLE_EQ(iso::pi, pi);
    EXPECT_DOUBLE_EQ(0.0, scos(pi / 2.0));
    EXPECT_DOUBLE_EQ(1.0, std::sin(pi / 2.0));
    EXPECT_DOUBLE_EQ(0.0, scos(iso::pi / 2.0));
    EXPECT_DOUBLE_EQ(1.0, std::sin(iso::pi / 2.0));
    EXPECT_DOUBLE_EQ(0.0, std::sin(iso::pi/2.0 - iso::pi/2));
}

TEST(IsoTest, DegreesToRadians) {
    iso::degrees dtheta(90);
    iso::radians rtheta;
    iso::convert(rtheta, dtheta);
    ASSERT_DOUBLE_EQ(iso::pi / 2.0, rtheta.value);
}

TEST(IsoTest, RadiansToDegrees) {
    iso::radians rtheta(iso::pi / 2);
    iso::degrees dtheta;
    iso::convert(dtheta, rtheta);
    ASSERT_DOUBLE_EQ(90.0, dtheta.value);
}

TEST(IsoTest, FeetMeters) {
    using namespace iso;
    meters A;
    A = 7.42_m; // operator"" into copy assign
    meters X(8.42_m); // operator"" into copy constructor
    feet B = convert(A);
    ASSERT_DOUBLE_EQ(7.42, A.value);
    ASSERT_DOUBLE_EQ(8.42, X.value);
    ASSERT_NEAR(24.34383, B.value, 1E-3); // within a milli-foot
    ASSERT_DOUBLE_EQ(390, B.sixteenths);
    B = 12.0_ft;
    A = convert(B);
    EXPECT_FLOAT_EQ(3.6576, A.value);
}

TEST(IsoTest, Scaling) {
    using namespace iso;
    meters A = 7.42_m;  // operator""_m
    ASSERT_DOUBLE_EQ(0.007'420, A.at_scale(SI::prefix::kilo));
    ASSERT_DOUBLE_EQ(7'420'000'000, A.at_scale(SI::prefix::nano));

    meters B(93.0); // valued constructor
    meters C(93.0 * SI::prefix::mega); // operator* then specific constructor
    ASSERT_DOUBLE_EQ(93'000'000, C.value);
    ASSERT_DOUBLE_EQ(93'000'000, B.at_scale(SI::prefix::micro));

    double k = 9.0 * SI::prefix::nano; // operator* again
    ASSERT_DOUBLE_EQ(0.000'000'009, k);
}

TEST(IsoTest, HertzSeconds) {
    using namespace iso;

    hertz framerate = 10.0_hz; // copy assign? or implicit conversion to copy constructor?
    framerate = 11.0_hz;
    seconds framewindow = convert(framerate);
    ASSERT_DOUBLE_EQ(1.0/11.0, framewindow.value);

    framewindow = 20.0_sec;
    framerate = convert(framewindow);
    ASSERT_DOUBLE_EQ(1/20.0, framerate.value);

    framerate = hertz(30.0); // wrap new assignment into a copy assign
    framewindow = 1.0 /framerate; // could use any double, which would modify the relationship
    ASSERT_DOUBLE_EQ(1/30.0, framewindow.value);
}

TEST(IsoTest, Quantities) {
    using namespace iso;
    distance d1 = 9.807_m;
    iso::time t1 = 1.0_sec;
    acceleration earth = 9.807_G;

    // distance d2 = 12_ft; // ERROR (no direct conversion from feet to meters)

    // different types of divisor constructors
    speed v1 = d1 / 1.0_sec;
    speed v2 = 9.807_m / t1;
    speed v3 = d1 / t1;
    speed v4 = 9.807_m / 1.0_sec;

    ASSERT_DOUBLE_EQ(9.807, v1.reduce());
    ASSERT_DOUBLE_EQ(9.807, v2.reduce());
    ASSERT_DOUBLE_EQ(9.807, v3.reduce());
    ASSERT_DOUBLE_EQ(9.807, v4.reduce());

    acceleration g1 = 9.807_m_per_sec / 1.0_sec;
    acceleration g2 = v1 / 1.0_sec;
    acceleration g3 = 9.807_m_per_sec / t1;
    acceleration g4 = v1 / t1;

    ASSERT_TRUE(earth == g1) << "Must be equivalent!";
    ASSERT_TRUE(earth == g2) << "Must be equivalent!";
    ASSERT_TRUE(earth == g3) << "Must be equivalent!";
    ASSERT_TRUE(earth == g4) << "Must be equivalent!";
}

TEST(IsoTest, Comparisons) {
    using namespace iso;
    acceleration earth = 9.807_G;
    acceleration mars = 3.711_G;
    acceleration venus = 8.87_G;
    ASSERT_TRUE(mars < venus && earth > venus) << "Mars < Venus < Earth";
    ASSERT_TRUE(mars != earth);
}

TEST(IsoTest, IECObjects) {
    uint64_t kib = 24_kibibytes;
    uint64_t kil = 24_kilobytes;
    ASSERT_TRUE(1_kibibytes == 1024) << "Must be power of two valued";
    ASSERT_TRUE(1_kilobytes == 1000) << "Must be power of ten valued";
    ASSERT_TRUE(kib != kil) << "Must not be equal!";
}

TEST(IsoTest, VoltageAmperesOhms) {
    using namespace iso;
    volts V = 24_mV;
    ASSERT_DOUBLE_EQ(V.value, 0.024) << "Must be equal";
    amperes I = 3_mA;
    ohms R = V / I; // operator/
    ASSERT_TRUE(R == 8_Ohm) << "Must be equal resistance!";
}

TEST(IsoTest, JoulesAndCompound) {
    using namespace iso;
    joules E = 87_J;
    torque F = 87_N * 1.0_m;
    ASSERT_DOUBLE_EQ(E.value, F.value) << "By raw value they are the same!";
    ASSERT_TRUE(10_J == 5_N*2.0_m) << "Not the same values but the same force!";
}

TEST(IsoTest, GramsAndOverloads) {
    using namespace iso;
    grams w1 = 118841_g;
    grams w2 = 10_g;
    ASSERT_TRUE((w1 + w2) == 118851_g) << "Must be equal";
}
