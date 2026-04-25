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

using point2 = xmmt::point_<xmmt::double2, 2>;
using point3 = xmmt::point_<xmmt::double3, 3>;
using point4 = xmmt::point_<xmmt::double4, 4>;

using vector2 = xmmt::vector_<xmmt::double2, 2>;
using vector3 = xmmt::vector_<xmmt::double3, 3>;
using vector4 = xmmt::vector_<xmmt::double4, 4>;

using pixel4f = xmmt::pixel_<xmmt::float4, 4>;
using pixel4d = xmmt::pixel_<xmmt::double4, 4>;

template <typename PACK_TYPE>
class XMMContainer2 : public ::testing::Test {
public:
    using point = xmmt::point_<PACK_TYPE, 2>;
    using vector = xmmt::vector_<PACK_TYPE, 2>;
    using pixel = xmmt::pixel_<PACK_TYPE, 2>;
};

typedef ::testing::Types<XMMContainer2<xmmt::double2>> Implementations2;

template <typename PACK_TYPE>
class XMMContainer3 : public ::testing::Test {
public:
    using point = xmmt::point_<PACK_TYPE, 3>;
    using vector = xmmt::vector_<PACK_TYPE, 3>;
    using pixel = xmmt::pixel_<PACK_TYPE, 3>;
};

typedef ::testing::Types<XMMContainer3<xmmt::float3>, XMMContainer3<xmmt::double3>> Implementations3;

template <typename PACK_TYPE>
class XMMContainer4 : public ::testing::Test {
public:
    using point = xmmt::point_<PACK_TYPE, 4>;
    using vector = xmmt::vector_<PACK_TYPE, 4>;
    using pixel = xmmt::pixel_<PACK_TYPE, 4>;
};

typedef ::testing::Types<XMMContainer4<xmmt::float4>, XMMContainer4<xmmt::double4>> Implementations4;

TYPED_TEST_SUITE(XMMContainer2, Implementations2);
TYPED_TEST_SUITE(XMMContainer3, Implementations3);
TYPED_TEST_SUITE(XMMContainer4, Implementations4);
