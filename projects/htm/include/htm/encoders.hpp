#pragma once

#include <htm/sparse_word.hpp>

namespace htm {

template <size_t N>
sparse_word<N> scalar_encoder(size_t value, size_t W) {
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
        int _min = value - hW;
        int _max = value + hW;
        printf("encode %zu in %zu bits with %zu width from %d to %d!\n", value, N, W, _min, _max);
        // when the width is higher, width also should be odd.
        for (int i = _min; i <= _max; i++) {
            o.set(i);
        }
    }
    return o;
}

std::tm const* const now();

// TODO fix for leapyears, see leap year rules 4th, 100th, 400th, etc
sparse_word<365> dayofyear_encoder();
sparse_word<31> dayofmonth_encoder();
sparse_word<7> dayofweek_encoder();
sparse_word<365 + 31 + 7> dayof_encoder();

}  // namespace htm
