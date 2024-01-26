#include "basal/gtest_helper.hpp"
#include "iso/iso.hpp"

using namespace basal::literals;

TEST(IsoTest, PITest) {
    iso::precision pi = std::atan(1.0_p) * 4.0_p;
    auto scos = [](iso::precision a) -> iso::precision { return std::sin(a - (iso::pi / 2)); };
    EXPECT_PRECISION_EQ(iso::pi, pi);
    EXPECT_PRECISION_EQ(0.0_p, scos(pi / 2.0_p));
    EXPECT_PRECISION_EQ(1.0_p, std::sin(pi / 2.0_p));
    EXPECT_PRECISION_EQ(0.0_p, scos(iso::pi / 2.0_p));
    EXPECT_PRECISION_EQ(1.0_p, std::sin(iso::pi / 2.0_p));
    EXPECT_PRECISION_EQ(0.0_p, std::sin(iso::pi / 2.0_p - iso::pi / 2));
}

TEST(IsoTest, DegreesToRadians) {
    iso::degrees dtheta{90};
    iso::radians rtheta;
    iso::convert(rtheta, dtheta);
    constexpr iso::precision const C = 2.0_p;
    ASSERT_PRECISION_EQ(iso::pi / C, rtheta.value);
}

TEST(IsoTest, RadiansToDegrees) {
    iso::radians rtheta{iso::pi / 2};
    iso::degrees dtheta;
    iso::convert(dtheta, rtheta);
    constexpr iso::precision const C = 90.0_p;
    ASSERT_PRECISION_EQ(C, dtheta.value);
}

TEST(IsoTest, FeetMeters) {
    using namespace iso;
    using namespace iso::literals;
    using namespace iso::operators;

    meters A;
    A = 7.42_m;        // operator"" into copy assign
    meters X{8.42_m};  // operator"" into copy constructor
    feet B = convert(A);
    ASSERT_PRECISION_EQ(7.42_p, A.value);
    ASSERT_PRECISION_EQ(8.42_p, X.value);
    ASSERT_NEAR(24.34383_p, B.value, 1E-3);  // within a milli-foot
    ASSERT_PRECISION_EQ(390, B.sixteenths);
    B = 12.0_ft;
    A = convert(B);
    EXPECT_FLOAT_EQ(3.6576_p, A.value);
}

TEST(IsoTest, Scaling) {
    using namespace iso;
    using namespace iso::literals;
    using namespace iso::operators;
    using namespace SI::operators;

    meters A = 7.42_m;  // operator""_m
    ASSERT_PRECISION_EQ(0.007'420, A.at_scale(SI::prefix::kilo));
    ASSERT_PRECISION_EQ(7'420'000'000, A.at_scale(SI::prefix::nano));
    constexpr precision const D = 93.0_p;
    meters B{D};                     // valued constructor
    meters C{D * SI::prefix::mega};  // operator* then specific constructor
    ASSERT_PRECISION_EQ(D * 1E6, C.value);
    ASSERT_PRECISION_EQ(D * 1E6, B.at_scale(SI::prefix::micro));

    precision k = 9.0_p * SI::prefix::nano;  // operator* again
    ASSERT_PRECISION_EQ(0.000'000'009, k);
}

TEST(IsoTest, HertzSeconds) {
    using namespace iso;
    using namespace iso::literals;
    using namespace iso::operators;

    hertz framerate = 10.0_Hz;  // copy assign? or implicit conversion to copy constructor?
    framerate = 11.0_Hz;
    seconds framewindow = convert(framerate);
    ASSERT_PRECISION_EQ(1.0_p / 11.0_p, framewindow.value);

    framewindow = 20.0_sec;
    framerate = convert(framewindow);
    ASSERT_PRECISION_EQ(1.0_p / 20.0_p, framerate.value);

    framerate = hertz(30.0_p);        // wrap new assignment into a copy assign
    framewindow = 1.0_p / framerate;  // could use any precision, which would modify the relationship
    ASSERT_PRECISION_EQ(1.0_p / 30.0_p, framewindow.value);
}

TEST(IsoTest, Quantities) {
    using namespace iso;
    using namespace iso::literals;
    using namespace iso::operators;

    distance d1 = 9.807_m;
    iso::time t1 = 1.0_sec;
    acceleration earth = 9.807_G;

    // distance d2 = 12_ft; // ERROR (no direct conversion from feet to meters)

    // different types of divisor constructors
    speed v1 = d1 / 1.0_sec;
    speed v2 = 9.807_m / t1;
    speed v3 = d1 / t1;
    speed v4 = 9.807_m / 1.0_sec;

    ASSERT_PRECISION_EQ(9.807_p, v1.reduce());
    ASSERT_PRECISION_EQ(9.807_p, v2.reduce());
    ASSERT_PRECISION_EQ(9.807_p, v3.reduce());
    ASSERT_PRECISION_EQ(9.807_p, v4.reduce());

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
    using namespace iso::literals;

    acceleration earth = 9.807_G;
    acceleration mars = 3.711_G;
    acceleration venus = 8.87_G;
    ASSERT_TRUE(mars < venus && earth > venus) << "Mars < Venus < Earth";
    ASSERT_TRUE(mars != earth);
}

TEST(IsoTest, IECObjects) {
    using namespace SI;
    using namespace SI::literals;

    uint64_t kib = 24_kibibytes;
    uint64_t kil = 24_kilobytes;
    ASSERT_TRUE(1_kibibytes == 1024) << "Must be power of two valued";
    ASSERT_TRUE(1_kilobytes == 1000) << "Must be power of ten valued";
    ASSERT_TRUE(kib != kil) << "Must not be equal!";
}

TEST(IsoTest, VoltageAmperesOhms) {
    using namespace iso;
    using namespace iso::literals;
    using namespace iso::operators;

    volts V = 24_mV;
    ASSERT_PRECISION_EQ(V.value, 0.024_p) << "Must be equal";
    amperes I = 3_mA;
    ohms R = V / I;  // operator/
    ASSERT_TRUE(R == 8_Ohm) << "Must be equal resistance!";
}

TEST(IsoTest, JoulesAndCompound) {
    using namespace iso;
    using namespace iso::literals;
    using namespace iso::operators;

    joules E = 87_J;
    torque F = 87_N * 1.0_m;
    ASSERT_PRECISION_EQ(E.value, F.value) << "By raw value they are the same!";
    ASSERT_TRUE(10_J == 5_N * 2.0_m) << "Not the same values but the same force!";
}

TEST(IsoTest, GramsAndOverloads) {
    using namespace iso;
    using namespace iso::literals;
    using namespace iso::operators;

    grams w1 = 118841_g;
    grams w2 = 10_g;
    ASSERT_TRUE((w1 + w2) == 118851_g) << "Must be equal";
}
