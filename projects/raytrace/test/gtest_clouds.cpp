#include <gtest/gtest.h>

#include <iostream>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "basal/gtest_helper.hpp"
#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

using namespace raytrace;
using namespace raytrace::clouds;

TEST(CloudLayerTest, BoundingBox) {
    clouds::CloudLayer layer{R3::origin, 2.0_p, 1.0_p, 3.0_p};
    Bounds box = layer.bounds();
    EXPECT_PRECISION_EQ(-2.0_p, box.min.x());
    EXPECT_PRECISION_EQ(2.0_p, box.max.x());
    EXPECT_PRECISION_EQ(-1.0_p, box.min.y());
    EXPECT_PRECISION_EQ(1.0_p, box.max.y());
    EXPECT_PRECISION_EQ(-3.0_p, box.min.z());
    EXPECT_PRECISION_EQ(3.0_p, box.max.z());

    EXPECT_TRUE(layer.contains(R3::origin));
    EXPECT_TRUE(layer.contains(point{2.0_p, 0.0_p, 0.0_p}));
    EXPECT_FALSE(layer.contains(point{2.5_p, 0.0_p, 0.0_p}));
    EXPECT_FALSE(layer.contains(point{0.0_p, 1.5_p, 0.0_p}));
}

TEST(CloudLayerTest, NormalizedCoordinates) {
    clouds::CloudLayer layer{R3::origin, 2.0_p, 1.0_p, 3.0_p};
    point n = layer.normalized(point{4.0_p, 1.0_p, -6.0_p});
    EXPECT_PRECISION_EQ(2.0_p, n.x());   // 4 / 2
    EXPECT_PRECISION_EQ(1.0_p, n.y());   // 1 / 1
    EXPECT_PRECISION_EQ(-2.0_p, n.z());  // -6 / 3
}

TEST(CloudLayerTest, VerticalProfile) {
    clouds::CloudLayer layer{R3::origin, 1.0_p, 1.0_p, 1.0_p};
    // Peaks mid-layer, tapers to zero at the top and bottom faces.
    EXPECT_NEAR(1.0_p, layer.vertical_profile(0.0_p), 1e-6);
    EXPECT_NEAR(0.0_p, layer.vertical_profile(1.0_p), 1e-6);
    EXPECT_NEAR(0.0_p, layer.vertical_profile(-1.0_p), 1e-6);
    // Smoothly decreasing from mid to face.
    EXPECT_GT(layer.vertical_profile(0.0_p), layer.vertical_profile(0.5_p));
    EXPECT_GT(layer.vertical_profile(0.5_p), layer.vertical_profile(1.0_p));
    // Even outside the box the profile is clamped and stays in [0, 1].
    EXPECT_NEAR(0.0_p, layer.vertical_profile(3.0_p), 1e-6);
}

TEST(CloudLayerTest, HorizontalEnvelope) {
    clouds::CloudLayer layer{R3::origin, 1.0_p, 1.0_p, 1.0_p};
    // Default strength gives a broad bank (no feathering).
    EXPECT_NEAR(1.0_p, layer.horizontal_envelope(0.0_p, 0.0_p), 1e-6);
    EXPECT_NEAR(1.0_p, layer.horizontal_envelope(0.9_p, 0.9_p), 1e-6);

    layer.horizontal_strength(2.0_p);
    EXPECT_NEAR(1.0_p, layer.horizontal_envelope(0.0_p, 0.0_p), 1e-6);
    EXPECT_LT(layer.horizontal_envelope(1.0_p, 0.0_p), 1.0_p);
    EXPECT_LT(layer.horizontal_envelope(0.0_p, 1.0_p), 1.0_p);
}

TEST(CloudLayerTest, DensityIsNonNegativeAndBounded) {
    clouds::CloudLayer layer{R3::origin, 1.0_p, 1.0_p, 1.0_p};
    layer.density_seed(noise::vector3{{0.123_p, 0.456_p, 0.789_p}}).density_octaves(5).density_scale(6.0_p);
    for (int z = -4; z <= 4; z++) {
        for (int y = -4; y <= 4; y++) {
            for (int x = -4; x <= 4; x++) {
                point p{static_cast<precision>(x) * 0.25_p, static_cast<precision>(y) * 0.25_p,
                        static_cast<precision>(z) * 0.25_p};
                precision d = layer.density(p);
                EXPECT_GE(d, 0.0_p);
                EXPECT_LE(d, 1.0_p) << "at " << p;
            }
        }
    }
}

TEST(CloudLayerTest, DensityTapersToZeroAtVerticalFaces) {
    clouds::CloudLayer layer{R3::origin, 1.0_p, 1.0_p, 1.0_p};
    // The vertical profile forces near-zero density at the top and bottom faces
    // regardless of the FBM field.
    EXPECT_NEAR(0.0_p, layer.density(point{0.0_p, 1.0_p, 0.0_p}), 1e-4);
    EXPECT_NEAR(0.0_p, layer.density(point{0.0_p, -1.0_p, 0.0_p}), 1e-4);
    EXPECT_GT(layer.density(point{0.0_p, 0.0_p, 0.0_p}), 0.0_p);
}

TEST(CloudLayerTest, DensityOverride) {
    clouds::CloudLayer layer{R3::origin, 1.0_p, 1.0_p, 1.0_p};
    layer.density_callback([&](point const& p) -> precision { return 0.5_p; });
    EXPECT_PRECISION_EQ(0.5_p, layer.density(R3::origin));
    // Custom callbacks can exceed the unit range but density clamps to [0, 1].
    layer.density_callback([&](point const& p) -> precision { return 2.0_p; });
    EXPECT_PRECISION_EQ(1.0_p, layer.density(R3::origin));
    layer.density_callback([&](point const& p) -> precision { return -1.0_p; });
    EXPECT_PRECISION_EQ(0.0_p, layer.density(R3::origin));
}

TEST(CloudIntegratorTest, ClipMisses) {
    clouds::CloudLayer layer{R3::origin, 1.0_p, 1.0_p, 1.0_p};
    clouds::CloudIntegrator integrator{layer, 0.1_p};
    ray miss{point{-10.0_p, 20.0_p, 0.0_p}, R3::basis::X};
    precision t0 = 0.0_p, t1 = 0.0_p;
    EXPECT_FALSE(integrator.clip(miss, t0, t1));
    EXPECT_FALSE(integrator.intersects(miss));
    EXPECT_PRECISION_EQ(0.0_p, integrator.optical_depth(miss));
    EXPECT_PRECISION_EQ(1.0_p, integrator.transmittance(miss));
    EXPECT_PRECISION_EQ(0.0_p, integrator.brightening(miss, R3::basis::X));
}

TEST(CloudIntegratorTest, ClipThroughCenter) {
    clouds::CloudLayer layer{R3::origin, 1.0_p, 1.0_p, 1.0_p};
    clouds::CloudIntegrator integrator{layer, 0.25_p};
    ray r{point{-10.0_p, 0.0_p, 0.0_p}, R3::basis::X};
    precision t0 = 0.0_p, t1 = 0.0_p;
    EXPECT_TRUE(integrator.clip(r, t0, t1));
    EXPECT_NEAR(9.0_p, t0, 1e-6);   // enters at x = -1
    EXPECT_NEAR(11.0_p, t1, 1e-6);  // exits at x = +1
}

TEST(CloudIntegratorTest, EmptyBoxTransmitsAll) {
    clouds::CloudLayer layer{R3::origin, 1.0_p, 1.0_p, 1.0_p};
    layer.extinction(2.0_p);
    layer.density_callback([&](point const& p) -> precision { return 0.0_p; });
    clouds::CloudIntegrator integrator{layer, 0.25_p};
    ray r{point{-10.0_p, 0.0_p, 0.0_p}, R3::basis::X};
    EXPECT_PRECISION_EQ(0.0_p, integrator.optical_depth(r));
    EXPECT_PRECISION_EQ(1.0_p, integrator.transmittance(r));
    EXPECT_PRECISION_EQ(0.0_p, integrator.brightening(r, R3::basis::X));
}

TEST(CloudIntegratorTest, UniformDensityRecoversBeerLambert) {
    // rho = 0.25, box extent 2 along x, sigma = 2.0
    //   tau  = rho * 2 = 0.5
    //   T    = exp(-sigma * tau) = exp(-1) ~= 0.3679
    clouds::CloudLayer layer{R3::origin, 1.0_p, 1.0_p, 1.0_p};
    layer.extinction(2.0_p);
    layer.density_callback([&](point const& p) -> precision { return 0.25_p; });
    clouds::CloudIntegrator integrator{layer, 0.05_p};
    ray r{point{-10.0_p, 0.0_p, 0.0_p}, R3::basis::X};
    precision tau = integrator.optical_depth(r);
    EXPECT_NEAR(0.5_p, tau, 1e-4);
    EXPECT_NEAR(std::exp(-1.0_p), integrator.transmittance(r), 1e-4);
}

TEST(CloudIntegratorTest, UniformDensityBrightensLessWithDepth) {
    // With uniform density and light travelling +X, points near the sun-facing
    // (-X) face see a short optical path to the sun, so the average brightening is
    // positive but strictly less than the albedo (the far side is shaded).
    clouds::CloudLayer layer{R3::origin, 1.0_p, 1.0_p, 1.0_p};
    layer.extinction(2.0_p).albedo(0.8_p);
    layer.density_callback([&](point const& p) -> precision { return 0.25_p; });
    clouds::CloudIntegrator integrator{layer, 0.1_p};
    ray r{point{-10.0_p, 0.0_p, 0.0_p}, R3::basis::X};
    precision b = integrator.brightening(r, R3::basis::X);
    EXPECT_GT(b, 0.0_p);
    EXPECT_LT(b, 0.8_p);
}

TEST(CloudIntegratorTest, TransmittanceDecreasesWithDistance) {
    // A thicker cloud along the ray exposes a longer chord of the same density, so it
    // scatters/absorbs more than a thinner one.
    auto make_layer = [&](precision xhw) -> clouds::CloudLayer {
        clouds::CloudLayer layer{R3::origin, xhw, 1.0_p, 1.0_p};
        layer.extinction(1.0_p);
        layer.density_callback([&](point const& p) -> precision { return 0.5_p; });
        return layer;
    };
    clouds::CloudLayer thin_layer = make_layer(1.0_p);   // extent 2
    clouds::CloudLayer thick_layer = make_layer(2.0_p);  // extent 4
    // The integrator references its layer, so the layers must outlive the integrators.
    clouds::CloudIntegrator thin{thin_layer, 0.1_p};
    clouds::CloudIntegrator thick{thick_layer, 0.1_p};
    ray r{point{-10.0_p, 0.0_p, 0.0_p}, R3::basis::X};
    precision t_thin = thin.transmittance(r);
    precision t_thick = thick.transmittance(r);
    EXPECT_GT(t_thin, t_thick);
}