#pragma once

#include <basal/debuggable.hpp>
#include <basal/exception.hpp>
#include <basal/ieee754.hpp>
#include <basal/listable.hpp>
#include <basal/printable.hpp>
#include <type_traits>

namespace basal {
char const *boolean(bool b);

template <typename TYPE>
constexpr typename std::underlying_type<TYPE>::type to_underlying(TYPE e) {
    static_assert(std::is_enum<TYPE>::value, "Must be an enumeration type");
    return static_cast<typename std::underlying_type<TYPE>::type>(e);
}

template <typename TYPE, size_t N>
constexpr size_t dimof(TYPE (&)[N]) {
    return N;
}

template <typename TYPE>
constexpr bool is_even(TYPE value) {
    static_assert(std::is_integral<TYPE>::value, "Must be an integral type");
    return (value & 0b1) == 0b0;
}

template <typename TYPE>
constexpr bool is_odd(TYPE value) {
    static_assert(std::is_integral<TYPE>::value, "Must be an integral type");
    return (value & 0b1) == 0b1;
}

namespace {
int __secret_anonymous_array__[2];
static_assert(dimof(__secret_anonymous_array__) == 2, "Must be 2");
}  // namespace
}  // namespace basal
