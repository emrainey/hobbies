#pragma once

#include <htm/sparse_word.hpp>

namespace htm {
namespace encoders {
template <size_t N>
sparse_word<N> scalar(size_t value, size_t W) {
    // static_assert(W < N, "Width must be smaller than number of bits.");
    assert(W > 0);  //  "Width must be positive");
    static_assert(N > 0, "Number of bits must be positive");
    assert(W & 1);  //  "Width should be an odd number!");
    sparse_word<N> o;

    // @TODO must be able to assert that value is less than N too! Otherwise these will
    // not be able to encode!
    assert(value < N);

    // in the case where W == 1, and N is the total range, it's just a bit set.
    if (W == 1) {
        o.set((size_t)value);
    } else {
        size_t hW = W / 2;
        size_t _min = value - hW;
        size_t _max = value + hW;
        printf("encode %zu in %zu bits with %zu width from %zu to %zu!\n", value, N, W, _min, _max);
        // when the width is higher, width also should be odd.
        for (size_t i = _min; i <= _max; i++) {
            o.set(i);
        }
    }
    return o;
}
// TODO fix for leapyears, see leap year rules 4th, 100th, 400th, etc
sparse_word<365> dayofyear();
sparse_word<31> dayofmonth();
sparse_word<7> dayofweek();
sparse_word<365 + 31 + 7> dayof();

}  // namespace encoders
}  // namespace htm
