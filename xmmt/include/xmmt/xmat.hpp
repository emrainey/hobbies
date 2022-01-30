#pragma once

#include <cstddef>
#include <initializer_list>

#include <emmintrin.h>
#include <immintrin.h>

#include <basal/exception.hpp>

namespace intel {

#if defined(__AVX2__) and (__AVX2__ == 1)
#pragma message("AVX2 Enabled")
#endif

#if defined(__AVX__) and (__AVX__ == 1)
#pragma message("AVX Enabled")
#endif

template <size_t ROWS, size_t COLS>
class xmat_ {
public:
    constexpr static size_t rows = ROWS;
    constexpr static size_t cols = COLS;
    constexpr static size_t align_size = 16;
    //static_assert(1 < rows and rows < 5, "Must have between 1 and 5 exclusive rows");
    static_assert(1 < cols and cols < 9, "Must have between 2 to 8 columns (inclusive)");
private:
    /** Each row is a 8 element, so it will have the required alignment (16) */
    union {
        __m512d m512[1];
        __m256d m256[2];
        __m128d m128[4];
        double  m064[8];
    } data[rows];
public:
    xmat_() { zero(); }
    xmat_(const xmat_& o) : xmat_() { copy_from(o); }
    xmat_(xmat_&& o) : xmat_() { copy_from(o); }
    xmat_& operator=(const xmat_& o) { copy_from(o); return (*this); }
    xmat_& operator=(xmat_& o) { copy_from(o); return (*this); }
    virtual ~xmat_() = default;

    /** Zeros the whole structure, even parts "outside" the established area, but within max allocation */
    inline void zero() {
        for (size_t j = 0; j < rows; j++) {
#if defined(__AVX2__) and defined(__AVX512F__)
            data[j].m512[0] = _mm512_setzero_pd();
#elif defined(__AVX__)
            data[j].m256[0] = _mm256_setzero_pd();
            data[j].m256[1] = _mm256_setzero_pd();
#endif
        }
    }

    /** Fills all relevant memory locations with the value. */
    inline void fill(double v) {
        for (size_t j = 0; j < rows; j++) {
            for (size_t i = 0; i < cols; i++) {
                data[j].m064[i] = v;
            }
        }
    }

    /** Copies all data over from one array to the other */
    inline void copy_from(const xmat_& o) {
        for (size_t j = 0; j < rows; j++) {
            data[j].m512[0] = o.data[j].m512[0];
        }
    }

    inline xmat_(std::initializer_list<double> list) {
        basal::exception::throw_if(list.size() != rows*cols, __FILE__, __LINE__, "Not exact number of elements");
        auto iter = list.begin();
        for (size_t j = 0; j < rows; j++) {
            for (size_t i = 0; i < cols; i++) {
                data[j].m064[i] = *iter++;
            }
        }
    }

    inline const double& at(size_t j, size_t i) const {
        basal::exception::throw_unless(j < rows and i < cols, __FILE__, __LINE__, "Must be within bounds");
        return data[j].m064[i];
    }

    inline double& at(size_t j, size_t i) {
        basal::exception::throw_unless(j < rows and i < cols, __FILE__, __LINE__, "Must be within bounds");
        return data[j].m064[i];
    }

    inline double *operator[](size_t index) { return &data[index].m064[0]; }

    inline const double *operator[](size_t index) const { return &data[index].m064[0]; }

    inline xmat_ operator*(const double b) {
        xmat_ C = (*this); // copy
        C *= b;
        return C;
    }

    inline xmat_ operator/(const double b) {
        xmat_ C = (*this); // copy
        C /= b;
        return C;
    }

    // [shortcut] Do not implement xmat_ operator+(const double b);
    // [shortcut] Do not implement xmat_ operator-(const double b);

    inline xmat_& operator+=(const xmat_& B) {
        for (size_t j = 0; j < rows; j++) {
            if constexpr (cols == 2) {
                data[j].m128[0] = _mm_add_pd(data[j].m128[0], B.data[j].m128[0]);
            } else if constexpr (cols <= 4) {
                data[j].m256[0] = _mm256_add_pd(data[j].m256[0], B.data[j].m256[0]);
            } else {
                data[j].m512[0] = _mm512_add_pd(data[j].m512[0], B.data[j].m512[0]);
            }
        }
        return (*this);
    }

    inline xmat_& operator-=(const xmat_& B) {
        for (size_t j = 0; j < rows; j++) {
            if constexpr (cols == 2) {
                data[j].m128[0] = _mm_sub_pd(data[j].m128[0], B.data[j].m128[0]);
            } else if constexpr (cols <= 4) {
                data[j].m256[0] = _mm256_sub_pd(data[j].m256[0], B.data[j].m256[0]);
            } else {
                data[j].m512[0] = _mm512_sub_pd(data[j].m512[0], B.data[j].m512[0]);
            }
        }
        return (*this);
    }

    // [shortcut] Do not implement xmat_& operator*=(const xmat& b);
    // [shortcut] Do not implement xmat_& operator/=(const xmat& b);

    /** Equivalent to adding a same sized matrix with every element being the value given */
    inline xmat_& operator+=(const double value) {
        for (size_t j = 0; j < rows; j++) {
            if constexpr (cols == 2) {
                __m128d v = _mm_set1_pd(value);
                data[j].m128[0] = _mm_add_pd(data[j].m128[0], v);
            } else if constexpr (cols <= 4) {
                __m256d v = _mm256_set1_pd(value);
                data[j].m256[0] = _mm256_add_pd(data[j].m256[0], v);
            } else {
                __m512d v = _mm512_set1_pd(value);
                data[j].m512[0] = _mm512_add_pd(data[j].m512[0], v);
            }
        }
        return (*this);
    }

    /** Equivalent to subtracting a same sized matrix with every element being the value given */
    inline xmat_& operator-=(const double value) {
        for (size_t j = 0; j < rows; j++) {
            if constexpr (cols == 2) {
                __m128d v = _mm_set1_pd(value);
                data[j].m128[0] = _mm_sub_pd(data[j].m128[0], v);
            } else if constexpr (cols <= 4) {
                __m256d v = _mm256_set1_pd(value);
                data[j].m256[0] = _mm256_sub_pd(data[j].m256[0], v);
            } else {
                __m512d v = _mm512_set1_pd(value);
                data[j].m512[0] = _mm512_sub_pd(data[j].m512[0], v);
            }
        }
        return (*this);
    }

    /** Scales the matrix */
    inline xmat_& operator*=(const double value) {
        for (size_t j = 0; j < rows; j++) {
            if constexpr (cols == 2) {
                __m128d v = _mm_set1_pd(value);
                data[j].m128[0] = _mm_mul_pd(data[j].m128[0], v);
            } else if constexpr (cols <= 4) {
                __m256d v = _mm256_set1_pd(value);
                data[j].m256[0] = _mm256_mul_pd(data[j].m256[0], v);
            } else {
                __m512d v = _mm512_set1_pd(value);
                data[j].m512[0] = _mm512_mul_pd(data[j].m512[0], v);
            }
        }
        return (*this);
    }

    /** Divides the matrix */
    inline xmat_& operator/=(const double value) {
        for (size_t j = 0; j < rows; j++) {
            if constexpr (cols == 2) {
                __m128d v = _mm_set1_pd(value);
                data[j].m128[0] = _mm_div_pd(data[j].m128[0], v);
            } else if constexpr (cols <= 4) {
                __m256d v = _mm256_set1_pd(value);
                data[j].m256[0] = _mm256_div_pd(data[j].m256[0], v);
            } else {
                __m512d v = _mm512_set1_pd(value);
                data[j].m512[0] = _mm512_div_pd(data[j].m512[0], v);
            }
        }
        return (*this);
    }

    /** The fill operator */
    xmat_& operator=(const double v) {
        for (size_t j = 0; j < rows; j++) {
            for (size_t i = 0; i < cols; i++) {
                data[j][i] = v;
            }
        }
    }

    // TODO bool operator!=(const xmat_& o);
    // TODO xmat_ inverse();

    /** Returns the transpose matrix */
    xmat_<COLS, ROWS> transpose() {
        xmat_<COLS, ROWS> T;
        for (size_t j = 0; j < T.rows; j++) {
            for (size_t i = 0; i < T.cols; i++) {
                T.data[j].m064[i] = data[i].m064[j];
            }
        }
        return T;
    }
};

template <size_t ROWS0, size_t COLS0, size_t COLS1>
xmat_<ROWS0, COLS1> operator*(const xmat_<ROWS0, COLS0> &a, const xmat_<COLS0, COLS1>& b) {
    xmat_<ROWS0, COLS1> C;
    for (size_t j = 0; j < C.rows; j++) {
        for (size_t i = 0; i < C.cols; i++) {
            if constexpr (a.cols == 2) {
                alignas(16) union {
                    __m128d m128;
                    double  m064[2];
                } c;
                // take from b's columns not rows
                c.m064[0] = b.data[0].m064[i];
                c.m064[1] = b.data[1].m064[i];
                c.m128 = _mm_mul_pd(a.data[j].m128[0], c.m128);
                C.data[j].m064[i] = c.m064[0] + c.m064[1];
            }
        }
    }
    return C;
}

    //TODO xmat_ operator/(const xmat_& b); // same as multiply by inverse

template <size_t ROWS, size_t COLS>
xmat_<ROWS, COLS> operator+(const xmat_<ROWS, COLS>& a, const xmat_<ROWS, COLS>& b) {
    xmat_<ROWS, COLS> C;
    for (size_t j = 0; j < C.rows; j++) {
        if constexpr (C.cols == 2) {
            C.data[j].m128[0] = _mm_add_pd(a.data[j].m128[0], b.data[j].m128[0]);
        } else if constexpr (C.cols <= 4) {
            C.data[j].m256[0] = _mm256_add_pd(a.data[j].m256[0], b.data[j].m256[0]);
        } else {
            C.data[j].m512[0] = _mm512_add_pd(a.data[j].m512[0], b.data[j].m512[0]);
        }
    }
    return C;
}

template <size_t ROWS, size_t COLS>
xmat_<ROWS, COLS> operator-(const xmat_<ROWS, COLS>& a, const xmat_<ROWS, COLS>& b) {
    xmat_<ROWS, COLS> C;
    for (size_t j = 0; j < C.rows; j++) {
        if constexpr (C.cols == 2) {
            C.data[j].m128[0] = _mm_sub_pd(a.data[j].m128[0], b.data[j].m128[0]);
        } else if constexpr (C.cols <= 4) {
            C.data[j].m256[0] = _mm256_sub_pd(a.data[j].m256[0], b.data[j].m256[0]);
        } else {
            C.data[j].m512[0] = _mm512_sub_pd(a.data[j].m512[0], b.data[j].m512[0]);
        }
    }
    return C;
}

}
