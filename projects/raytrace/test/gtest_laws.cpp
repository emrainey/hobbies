#include "basal/gtest_helper.hpp"

#include <raytrace/laws.hpp>

#include "raytrace/gtest_helper.hpp"

using namespace raytrace;

TEST(LawsTest, SnellsNormalIncident) {
    vector N{0, 0, 1};
    vector I{0, 0, -1};
    // should pass through exactly the same no matter the coefficients
    vector R = laws::snell(N, I, 1.0, 1.0).normalize();
    ASSERT_VECTOR_EQ(I, R);
    R = laws::snell(N, I, 1.0, 2.0).normalize();
    ASSERT_VECTOR_EQ(I, R);
}

TEST(LawsTest, SnellsAcuteIncident) {
    vector N{0, 0, 1};
    vector I{0, -1, -1};
    I.normalize();

    // Should pass through identically
    vector R = laws::snell(N, I, 1.0, 1.0).normalize();
    ASSERT_VECTOR_EQ(I, R);

    // Should bend vector
    R = laws::snell(N, I, 1.0, 2.0).normalize();
    iso::radians phi = geometry::angle(-N, R);
    vector R1{0, -0.35355339059327373_p, -0.93541434669348533_p};
    ASSERT_NEAR(0.36136712390670783_p, phi.value, basal::epsilon);
    ASSERT_VECTOR_EQ(R1, R);
}

TEST(LawsTest, SnellsTotalInternalReflection) {
    vector N{0, 0, 1};
    vector I{0, -1, -1};
    I.normalize();

    // Should pass through identically
    vector R = laws::snell(N, I, 2.0, 2.0).normalize();
    ASSERT_VECTOR_EQ(I, R);

    // Should be TIR
    R = laws::snell(N, I, 2.0, 1.0);
    ASSERT_VECTOR_EQ(geometry::R3::null, R);
}

TEST(LawsTest, Reflection) {
    vector N{0, 0, 1};
    vector I{0, -1, -1};
    vector R = laws::reflection(N, I);
    vector R1{0, -1, 1};
    ASSERT_VECTOR_EQ(R1, R);

    // does reflection "work" if the Normal and the incident are backwards?
    vector J{0, 1, 1};
    vector R2{0, 1, -1};
    R = laws::reflection(N, J);
    ASSERT_VECTOR_EQ(R2, R);
}

TEST(LawsTest, Fresnel) {
    /// @internal used calculator at @see https://www.calctool.org/CALC/phys/optics/reflec_refrac
    vector N{0, 0, 1};
    vector I{0, -1, -1};
    I.normalize();
    precision n1 = 1.0;
    precision n2 = 2.0;
    vector R = laws::snell(N, I, n1, n2).normalize();
    iso::radians incident_angle = geometry::angle(N, -I);
    ASSERT_NEAR(iso::pi / 4.0, incident_angle.value, basal::epsilon);
    iso::radians transmitted_angle = geometry::angle(-N, R);
    ASSERT_NEAR(0.36136712390670783_p, transmitted_angle.value, basal::epsilon);  // the calculator was in float, not double.
    precision Rf = laws::fresnel(n1, n2, incident_angle, transmitted_angle);
    precision Rr = 1.0 - Rf;
    precision Rc = 1.0 - 0.5 * (0.203777 + 0.0415249);  // numbers from calc
    ASSERT_NEAR(Rc, Rr, 1E-5);  // should be accurate to 5 decimal places since the numbers are from a float calculator
}

TEST(LawsTest, Beers) {
    vector D{{8, 0, 0}};
    precision dropoff = laws::beers(D, 0.186);
    ASSERT_NEAR(0.226, dropoff, 1E-2);

    // at no distance the dropoff coefficient should be 1.0 unity
    dropoff = laws::beers(geometry::R3::null, 0.186);
    ASSERT_PRECISION_EQ(1.0, dropoff);
}