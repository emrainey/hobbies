#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <iostream>

#include "htm/dense_word.hpp"

namespace htm {
template <size_t BITS>
class sparse_word {
public:
    // start with a sparsity of 2%
    sparse_word() : bits{} {
        bits.clear();
    }
    sparse_word(std::vector<uint16_t> const& that) : bits{} {
        bits.clear();
        bits = that;  // copy
        correct();
    }
    // sparse_word(std::initializer_list const& list) : sparse_word(std::vector(list)) {}
    sparse_word(dense_word<BITS> const& dense) : bits{} {
        bits.clear();
        for (size_t b = 0; b < BITS; b++) {
            if (dense.test(b)) {
                bits.push_back(b);
            }
        }
    }

    sparse_word& operator=(dense_word<BITS> const& dense) {
        bits.clear();
        for (size_t b = 0; b < BITS; b++) {
            if (dense.test(b)) {
                bits.push_back(b);
            }
        }
        return (*this);
    }

    ~sparse_word() {
    }

    constexpr size_t size() const {
        return BITS;
    }

    void set(size_t index) {
        // printf("size_t index %zu of %zu\n", index, BITS);
        index %= BITS;
        bits.push_back(index);
        correct();  // this will sort
    }

    void set(int index) {
        if ((0 - int(BITS)) < index and index < 0) {
            index = BITS + index;
            // printf("Negative index %d of %zu\n", index, BITS);
        } else if (size_t(index) >= BITS) {
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

    bool all() const {
        return bits.size() == BITS;
    }

    bool any() const {
        return not bits.empty();
    }

    bool none() const {
        return bits.empty();
    }

    void clear() {
        bits.clear();
    }

    size_t count() const {
        return population();
    }

    size_t population() const {
        return bits.size();
    }

    /// @brief Returns the capacity of the sparse word which is not a simple count of the bits set.
    /// @note @code n! / (w! * (n - w)!) @endcode
    /// @return
    float capacity() const {
        size_t num = std::tgamma(BITS + 1);
        size_t n_w = BITS - population();
        size_t den = std::tgamma(population() + 1) * std::tgamma(n_w + 1);
        return ((float)num / den);
    }

    float sparsity() const {
        return (float)population() / (float)BITS;
    }

    void to_image(char const* filename) {
        to_image(filename, 0);
    }

    void to_image(char const* filename, size_t cols) {
        uint32_t const SQ = static_cast<uint32_t>(std::ceil(std::sqrt(BITS)));
        uint32_t const W = (cols == 0 ? SQ : cols);
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
            // initialize idx to the number from the preceding rows
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

    bool operator==(sparse_word<BITS> const& that) const {
        return bits == that.bits;
    }

    bool operator!=(sparse_word<BITS> const& that) const {
        return bits != that.bits;
    }

protected:
    void correct() {
        if (not std::is_sorted(bits.begin(), bits.end())) {
            // sort in ascending order
            std::sort(bits.begin(), bits.end(), std::less<uint16_t>());
        }
        // remove duplicate entries
        auto last = std::unique(bits.begin(), bits.end());
        // erase the duplicates at the end of the vector
        bits.erase(last, bits.end());
    }
    template <size_t B2>
    friend sparse_word<B2> operator&(sparse_word<B2> const& a, sparse_word<B2> const& b);
    template <size_t B2>
    friend sparse_word<B2> operator|(sparse_word<B2> const& a, sparse_word<B2> const& b);
    template <size_t B2>
    friend sparse_word<B2> operator^(sparse_word<B2> const& a, sparse_word<B2> const& b);
    template <size_t B2>
    friend sparse_word<B2> operator!(sparse_word<B2> const& a);
    template <size_t B1, size_t B2>
    friend sparse_word<B1+B2> operator+(sparse_word<B1> const&, sparse_word<B2> const&);
    template <size_t B2>
    friend std::ostream& operator<<(std::ostream const&, sparse_word<B2> const&);

    /// the storage of the bits
    std::vector<uint16_t> bits;
};


template <size_t B2>
std::ostream& operator<<(std::ostream& os, sparse_word<B2> const& word) {
    os << "sparse_word<" << word.size() << ">: population=" << word.population() << " capacity=" << word.capacity() << " sparsity=" << word.sparsity()
        << " bits={";
    for (size_t i = 0; i < B2; i++) {
        if (word.test(i)) {
            os << i << ", ";
        }
    }
    os << "}";
    return os;
}

template <size_t B1, size_t B2>
sparse_word<B1 + B2> operator+(sparse_word<B1> const& a, sparse_word<B2> const& b) {
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
sparse_word<BITS> operator&(sparse_word<BITS> const& a, sparse_word<BITS> const& b) {
    std::vector<uint16_t> o;
    std::set_intersection(a.bits.begin(), a.bits.end(), b.bits.begin(), b.bits.end(), std::back_inserter(o));
    return sparse_word<BITS>(o);
}

template <size_t BITS>
sparse_word<BITS> operator|(sparse_word<BITS> const& a, sparse_word<BITS> const& b) {
    std::vector<uint16_t> o;
    std::set_union(a.bits.begin(), a.bits.end(), b.bits.begin(), b.bits.end(), std::back_inserter(o));
    return sparse_word<BITS>(o);
}

template <size_t BITS>
sparse_word<BITS> operator^(sparse_word<BITS> const& a, sparse_word<BITS> const& b) {
    std::vector<uint16_t> o;
    std::set_symmetric_difference(a.bits.begin(), a.bits.end(), b.bits.begin(), b.bits.end(), std::back_inserter(o));
    return sparse_word<BITS>(o);
}
}  // namespace htm
