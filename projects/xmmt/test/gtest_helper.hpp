#pragma once
#include <gtest/gtest.h>

#include <chrono>
#include <vector>

#include "basal/gtest_helper.hpp"
#include "xmmt/xmat.hpp"
#include "xmmt/xmmt.hpp"

using namespace basal::literals;
using precision = basal::precision;

namespace {
constexpr static precision sixteenth = 0.0625_p;
constexpr static precision eighth = 0.125_p;
constexpr static precision quarter = 0.25_p;
constexpr static precision half = 0.5_p;
constexpr static precision zero = 0.0_p;
constexpr static precision one = 1.0_p;
constexpr static precision two = 2.0_p;
constexpr static precision four = 4.0_p;
}  // namespace

using point2 = intel::point_<intel::double2, 2>;
using point3 = intel::point_<intel::double3, 3>;
using point4 = intel::point_<intel::double4, 4>;

using vector2 = intel::vector_<intel::double2, 2>;
using vector3 = intel::vector_<intel::double3, 3>;
using vector4 = intel::vector_<intel::double4, 4>;

using pixel4f = intel::pixel_<intel::float4, 4>;
using pixel4d = intel::pixel_<intel::double4, 4>;

template <typename PACK_TYPE>
class XMMContainer2 : public ::testing::Test {
public:
    using point = intel::point_<PACK_TYPE, 2>;
    using vector = intel::vector_<PACK_TYPE, 2>;
    using pixel = intel::pixel_<PACK_TYPE, 2>;
};

typedef ::testing::Types<XMMContainer2<intel::double2>> Implementations2;

template <typename PACK_TYPE>
class XMMContainer3 : public ::testing::Test {
public:
    using point = intel::point_<PACK_TYPE, 3>;
    using vector = intel::vector_<PACK_TYPE, 3>;
    using pixel = intel::pixel_<PACK_TYPE, 3>;
};

typedef ::testing::Types<XMMContainer3<intel::float3>, XMMContainer3<intel::double3>> Implementations3;

template <typename PACK_TYPE>
class XMMContainer4 : public ::testing::Test {
public:
    using point = intel::point_<PACK_TYPE, 4>;
    using vector = intel::vector_<PACK_TYPE, 4>;
    using pixel = intel::pixel_<PACK_TYPE, 4>;
};

typedef ::testing::Types<XMMContainer4<intel::float4>, XMMContainer4<intel::double4>> Implementations4;

TYPED_TEST_SUITE(XMMContainer2, Implementations2);
TYPED_TEST_SUITE(XMMContainer3, Implementations3);
TYPED_TEST_SUITE(XMMContainer4, Implementations4);
