#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "htm/dense_word.hpp"

namespace htm {
template <size_t BITS>
class sparse_word {
public:
    // start with a sparsity of 2%
    sparse_word() : bits() {
        bits.clear();
    }
    sparse_word(const std::vector<uint16_t>& that) : bits() {
        bits.clear();
        bits = that;  // copy
        correct();
    }
    // sparse_word(const std::initializer_list& list) : sparse_word(std::vector(list)) {}
    sparse_word(const dense_word<BITS>& dense) : bits() {
        bits.clear();
        for (size_t b = 0; b < BITS; b++) {
            if (dense.test(b)) {
                bits.push_back(b);
            }
        }
    }
    ~sparse_word() {
    }

    constexpr size_t size() const {
        return BITS;
    }

    void set(size_t index) {
        printf("size_t index %zu of %zu\n", index, BITS);
        index %= BITS;
        bits.push_back(index);
        correct();  // this will sort
    }

    void set(int index) {
        if (-BITS < index and index < 0) {
            index = BITS + index;
            printf("Negative index %d of %zu\n", index, BITS);
        } else if (index >= BITS) {
            index %= BITS;
        }
        bits.push_back((size_t)index);
        correct();  // this will sort
    }

    void reset(size_t index) {
        auto it = std::find(bits.begin(), bits.end(), index);
        if (it != bits.end()) {
            bits.erase(it);
        }
    }

    bool test(size_t index) const {
        auto it = std::find(bits.begin(), bits.end(), index);
        return it == bits.end() ? false : true;
    }

    void clear() {
        bits.clear();
    }

    size_t population() const {
        return bits.size();
    }

    double capacity() const {
        // n! / (w! * (n - w)!)
        size_t num = std::tgamma(BITS + 1);
        size_t n_w = BITS - population();
        size_t den = std::tgamma(population() + 1) * std::tgamma(n_w + 1);
        return ((double)num / den);
    }

    float sparsity() const {
        return (float)population() / (float)BITS;
    }

    sparse_word& operator=(const dense_word<BITS>& dense) {
        bits.clear();
        for (size_t b = 0; b < BITS; b++) {
            if (dense.test(b)) {
                bits.push_back(b);
            }
        }
        return (*this);
    }

    void print(void) {
        printf("w=%zu n=%zu cap=%lf sparsity=%lf\n", population(), BITS, capacity(), sparsity());
        for (auto& bit : bits) {
            printf("%hu ", bit);
        }
        printf("\n");
    }

    void to_image(const char* filename) {
        to_image(filename, 0);
    }

    void to_image(const char* filename, size_t cols) {
        const uint32_t SQ = static_cast<uint32_t>(std::ceil(std::sqrt(BITS)));
        const uint32_t W = (cols == 0 ? SQ : cols);
        uint32_t H = (cols == 0 ? SQ : BITS / W);
        if ((BITS % W) > 0) {
            H++;  // add an extra row
        }
        // any spares will be rendered as gray...
        FILE* fp = fopen(filename, "wb");
        fprintf(fp, "P5\n");
        fprintf(fp, "%u %u\n", W, H);
        fprintf(fp, "%u\n", 255);
        size_t idx = 0;
        // buffer the values into an array
        uint8_t* values = new uint8_t[W];
        // for each row
        for (size_t y = 0; y < H; y++) {
            memset(values, 0, W);
            // initialize idx to the number from the preceeding rows
            idx = y * W;
            // for each column
            for (size_t x = 0; x < W; x++) {
                if (idx < BITS) {
                    values[x] = test(idx + x) ? 0xFF : 0x00;
                } else {
                    values[x] = 0x80;
                }
            }
            fwrite(values, 1, W, fp);
        }
        delete[] values;
        fclose(fp);
    }

protected:
    void correct() {
        if (not std::is_sorted(bits.begin(), bits.end())) {
            // sort in ascending order
            std::sort(bits.begin(), bits.end(), std::less<uint16_t>());
        }
        // remove duplicate entries
        std::unique(bits.begin(), bits.end());
    }
    template <size_t B2>
    friend sparse_word<B2> operator&(const sparse_word<B2>& a, const sparse_word<B2>& b);
    template <size_t B2>
    friend sparse_word<B2> operator|(const sparse_word<B2>& a, const sparse_word<B2>& b);
    template <size_t B2>
    friend sparse_word<B2> operator^(const sparse_word<B2>& a, const sparse_word<B2>& b);
    template <size_t B2>
    friend sparse_word<B2> operator!(const sparse_word<B2>& a);
    // template <size_t B1, size_t B2> friend sparse_word<B1+B2> operator<<(const sparse_word<B1> &, const
    // sparse_word<B2>&);
    std::vector<uint16_t> bits;
};

template <size_t B1, size_t B2>
sparse_word<B1 + B2> operator<<(const sparse_word<B1>& a, const sparse_word<B2>& b) {
    sparse_word<B1 + B2> o;
    for (size_t i = 0; i < B1; i++) {
        if (a.test(i)) {
            o.set(i);
        }
    }
    for (size_t i = B1; i < (B1 + B2); i++) {
        if (b.test(i - B1)) {
            o.set(i);
        }
    }
    return o;
}

template <size_t BITS>
sparse_word<BITS> operator&(const sparse_word<BITS>& a, const sparse_word<BITS>& b) {
    std::vector<uint16_t> o;
    std::set_intersection(a.bits.begin(), a.bits.end(), b.bits.begin(), b.bits.end(), std::back_inserter(o));
    return sparse_word<BITS>(o);
}

template <size_t BITS>
sparse_word<BITS> operator|(const sparse_word<BITS>& a, const sparse_word<BITS>& b) {
    std::vector<uint16_t> o;
    std::set_union(a.bits.begin(), a.bits.end(), b.bits.begin(), b.bits.end(), std::back_inserter(o));
    return sparse_word<BITS>(o);
}

template <size_t BITS>
sparse_word<BITS> operator^(const sparse_word<BITS>& a, const sparse_word<BITS>& b) {
    std::vector<uint16_t> o;
    std::set_difference(a.bits.begin(), a.bits.end(), b.bits.begin(), b.bits.end(), std::back_inserter(o));
    return sparse_word<BITS>(o);
}
}  // namespace htm
