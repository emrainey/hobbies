/// @file
/// @author "Erik Rainey" (erik.rainey@gmail.com)
/// @copyright Copyright (c) 2007-2020 Erik Rainey

#include <iostream>

#include "linalg/matrix.hpp"

#include "linalg/solvers.hpp"

#if defined(__x86_64__)
#include <emmintrin.h>
#endif

namespace linalg {

// ****************************************************************************
// STATIC METHODS
// ****************************************************************************
static char const* g_filename = __FILE__;

matrix matrix::zeros(size_t rows, size_t cols) {
    return matrix{rows, cols}.fill(0.0_p);
}

matrix matrix::ones(size_t rows, size_t cols) {
    return matrix{rows, cols}.fill(1.0_p);
}

matrix matrix::identity(size_t rows, size_t cols) {
    ref_coord_iterator iter = [](size_t r, size_t c, precision& v) -> void { v = (r == c ? 1.0_p : 0.0_p); };
    return matrix{rows, cols}.for_each(iter);
}

matrix matrix::col(std::vector<precision>& data) {
    return matrix(data.size(), 1U, data.data());
}

matrix matrix::row(std::vector<precision>& data) {
    return matrix(1U, data.size(), data.data());
}

// ****************************************************************************
// CTORS
// ****************************************************************************

matrix::matrix(size_t r, size_t c) : matrix(r, c, true) {
}

matrix::matrix(size_t r, size_t c, bool allocate)
    : rows{r}
    , cols{c}
    , bytes{static_cast<size_t>(rows * cols) * sizeof(precision)}
    , external_memory{false}
    , memory{nullptr}
    , array{nullptr} {
    basal::exception::throw_unless(rows > 0, g_filename, __LINE__);
    basal::exception::throw_unless(cols > 0, g_filename, __LINE__);
    if (allocate && create(rows, cols, bytes)) {
        for (size_t i = 0; array && i < rows; i++) {
            array[i] = &memory[i * cols];
        }
    } else {
        memory = nullptr;
        array = nullptr;
    }
}

matrix::matrix(size_t rs, size_t cs, precision mat[])
    : rows{rs}
    , cols{cs}
    , bytes{static_cast<size_t>(rows * cols) * sizeof(precision)}
    , external_memory{true}
    , memory{mat}
    , array{nullptr} {
    basal::exception::throw_unless(rows > 0, g_filename, __LINE__);
    basal::exception::throw_unless(cols > 0, g_filename, __LINE__);
    basal::exception::throw_unless(memory != nullptr, g_filename, __LINE__);
    // we still have to allocate an array of pointers when using external memory
    array = static_cast<precision**>(malloc(static_cast<size_t>(rows) * sizeof(precision*)));
    basal::exception::throw_unless(array != nullptr, g_filename, __LINE__);
    for (size_t r = 0; r < rows; r++) {
        array[r] = &mat[r * cols];
    }
}

matrix::matrix(std::vector<std::vector<precision>> const& a)
    : rows{a.size()}
    , cols{a[0].size()}
    , bytes{static_cast<size_t>(rows * cols) * sizeof(precision)}
    , external_memory{false}
    , memory{nullptr}
    , array{nullptr} {
    if (create(rows, cols, bytes)) {
        for (size_t i = 0; array && i < rows; i++) {
            array[i] = &memory[i * cols];
        }
        for (size_t r = 0; r < a.size(); r++) {
            for (size_t c = 0; c < a[r].size(); c++) {
                array[r][c] = a[r][c];
            }
        }
    }
}

// copy constructor, shallow copy
matrix::matrix(matrix const& other) noexcept(false)
    : rows{other.rows}, cols{other.cols}, bytes{other.bytes}, external_memory{false}, memory{nullptr}, array{nullptr} {
    if (create(rows, cols, bytes)) {
        memcpy(memory, other.memory, bytes);
        // copy row order
        std::vector<ptrdiff_t> ro(rows);
        for (size_t r = 0; r < rows; r++) {
            ptrdiff_t pdiff = other.array[r] - other.memory;
            ro[r] = pdiff / static_cast<ptrdiff_t>(cols);
        }
        for (size_t r = 0; r < rows; r++) {
            array[r] = &memory[static_cast<size_t>(ro[r]) * cols];
        }
    }
}

// move constructor, deep copy (transfer of ownership), no chaining constructor
matrix::matrix(matrix&& other) noexcept(false)
    : rows{other.rows}
    , cols{other.cols}
    , bytes{other.bytes}
    , external_memory{other.external_memory}
    , memory{nullptr}
    , array{nullptr} {
    basal::exception::throw_unless(this->rows == other.rows, g_filename, __LINE__, "Must be equal rows");
    basal::exception::throw_unless(this->cols == other.cols, g_filename, __LINE__, "Must be equal cols");
    memory = other.memory;
    other.memory = nullptr;
    array = other.array;
    other.array = nullptr;
}

matrix::matrix(precision m[2][2]) : matrix(2, 2, m[0]) {
}
matrix::matrix(precision m[3][3]) : matrix(3, 3, m[0]) {
}
matrix::matrix(precision m[4][4]) : matrix(4, 4, m[0]) {
}

// copy assignment
matrix& matrix::operator=(matrix const& other) noexcept(false) {
    basal::exception::throw_unless(this->rows == other.rows, g_filename, __LINE__,
                                   "Must match rows (copy constructor)");
    basal::exception::throw_unless(this->cols == other.cols, g_filename, __LINE__,
                                   "Must match cols (copy constructor)");
    if (this != &other) {
        memcpy(memory, other.memory, bytes);
        // copy row order by computing the pointer offsets
        std::vector<ptrdiff_t> ro(rows);
        for (size_t r = 0; r < rows; r++) {
            ptrdiff_t pdiff = other.array[r] - other.memory;
            ro[r] = pdiff / static_cast<ptrdiff_t>(cols);
        }
        // and duplicating the same offsets from our memory
        for (size_t r = 0; r < rows; r++) {
            array[r] = &memory[static_cast<size_t>(ro[r]) * cols];
        }
    }
    return *this;
}

// move assignment
matrix& matrix::operator=(matrix&& other) noexcept(false) {
    basal::exception::throw_unless(this->rows == other.rows, g_filename, __LINE__,
                                   "Must match rows %zu != %zu (move constructor)", this->rows, other.rows);
    basal::exception::throw_unless(this->cols == other.cols, g_filename, __LINE__,
                                   "Must match cols %zu != %zu (move constructor)", this->cols, other.cols);
    if (this != &other) {
        if (memory) {
#if defined(__x86_64__)
            _mm_free(memory);
#else
            free(memory);
#endif
            memory = nullptr;
        }
        memory = other.memory;
        other.memory = nullptr;
        if (array) {
            free(array);
            array = nullptr;
        }
        array = other.array;
        other.array = nullptr;
    }
    return *this;
}

void matrix::operator=(precision const value) {
    fill(value);
}

matrix::~matrix() {
    destroy();
    basal::exception::throw_unless(memory == nullptr, g_filename, __LINE__);
    basal::exception::throw_unless(array == nullptr, g_filename, __LINE__);
}

// ****************************************************************************
// Regular Methods
// ****************************************************************************

bool matrix::create(size_t _rows, size_t _cols, size_t _bytes) {
    basal::exception::throw_unless(_rows == rows, g_filename, __LINE__, "Must allocate correct rows");
    basal::exception::throw_unless(_cols == cols, g_filename, __LINE__, "Must allocate correct cols");
    basal::exception::throw_unless((_rows * _cols * sizeof(decltype(*memory))) <= _bytes, g_filename, __LINE__,
                                   "Not enough memory allocated");
    basal::exception::throw_unless(memory == nullptr, g_filename, __LINE__, "Memory must be freed before allocation!");
#if defined(__x86_64__)
    memory = static_cast<precision*>(_mm_malloc(_bytes, 16));
#else
    memory = static_cast<precision*>(malloc(_bytes));
#endif
    basal::exception::throw_unless(memory != nullptr, g_filename, __LINE__);
    basal::exception::throw_unless(array == nullptr, g_filename, __LINE__,
                                   "Array of rows must be free before allocation");
    array = new precision*[_rows];
    basal::exception::throw_unless(array != nullptr, g_filename, __LINE__);
    return true;
}

void matrix::destroy() {
    if (not external_memory) {
        if (memory) {
#if defined(__x86_64__)
            _mm_free(memory);
#else
            free(memory);
#endif
            memory = nullptr;
        }
    } else {
        // this is externally allocated so just forget the pointer.
        memory = nullptr;
    }
    if (array) {
        delete[] array;
        array = nullptr;
    }
}

void matrix::for_each(const_coord_iterator const_functor) const {
    for (size_t r = 0; r < rows; r++) {
        for (size_t c = 0; c < cols; c++) {
            const_functor(r, c, array[r][c]);
        }
    }
}

matrix& matrix::for_each(ref_coord_iterator functor) {
    for (size_t r = 0; r < rows; r++) {
        for (size_t c = 0; c < cols; c++) {
            functor(r, c, array[r][c]);
        }
    }
    return *this;
}

void matrix::for_each(const_ref_iterator functor) const {
    for (size_t r = 0; r < rows; r++) {
        for (size_t c = 0; c < cols; c++) {
            functor(array[r][c]);
        }
    }
}

matrix& matrix::for_each(ref_iterator functor) {
    for (size_t r = 0; r < rows; r++) {
        for (size_t c = 0; c < cols; c++) {
            functor(array[r][c]);
        }
    }
    return *this;
}

matrix& matrix::fill(precision v) {
    ref_iterator iter = [&](precision& val) { val = v; };
    return for_each(iter);
}

matrix& matrix::zero() {
    fill(0.0_p);
    return *this;
}

precision* matrix::operator[](size_t row) noexcept(false) {
    basal::exception::throw_unless(row < rows, g_filename, __LINE__);
    return array[row];
}

precision const* matrix::operator[](size_t row) const noexcept(false) {
    basal::exception::throw_unless(row < rows, g_filename, __LINE__);
    return array[row];
}

precision& matrix::index(size_t row, size_t col) noexcept(false) {
    basal::exception::throw_unless(row < rows, g_filename, __LINE__);
    basal::exception::throw_unless(col < cols, g_filename, __LINE__);
    return array[row][col];
}

precision matrix::index(size_t const row, size_t const col) const noexcept(false) {
    basal::exception::throw_unless(row < rows, g_filename, __LINE__);
    basal::exception::throw_unless(col < cols, g_filename, __LINE__);
    return array[row][col];
}

precision& matrix::index(size_t idx) noexcept(false) {
    basal::exception::throw_unless(idx < (rows * cols), g_filename, __LINE__);
    size_t row = idx / cols;
    size_t col = idx % cols;
    return array[row][col];
}

precision matrix::index(size_t const idx) const noexcept(false) {
    basal::exception::throw_unless(idx < (rows * cols), g_filename, __LINE__);
    size_t row = idx / cols;
    size_t col = idx % cols;
    return array[row][col];
}

precision& matrix::operator()(size_t row, size_t col) noexcept(false) {
    basal::exception::throw_unless(row < rows, g_filename, __LINE__);
    basal::exception::throw_unless(col < cols, g_filename, __LINE__);
    return array[row][col];
}

precision matrix::operator()(size_t const row, size_t const col) const noexcept(false) {
    basal::exception::throw_unless(row < rows, g_filename, __LINE__);
    basal::exception::throw_unless(col < cols, g_filename, __LINE__);
    return array[row][col];
}

precision& matrix::at(short _r, short _c) noexcept(false) {
    size_t r = static_cast<size_t>(_r);
    size_t c = static_cast<size_t>(_c);
    basal::exception::throw_unless(0 < r && r <= rows, g_filename, __LINE__,
                                   "1's based counting, must be within bounds");
    basal::exception::throw_unless(0 < c && c <= cols, g_filename, __LINE__,
                                   "1's based counting, must be within bounds");
    return array[r - 1][c - 1];
}

precision matrix::at(short _r, short _c) const noexcept(false) {
    size_t r = static_cast<size_t>(_r);
    size_t c = static_cast<size_t>(_c);
    basal::exception::throw_unless(0 < r && r <= rows, g_filename, __LINE__,
                                   "1's based counting, must be within bounds");
    basal::exception::throw_unless(0 < c && c <= cols, g_filename, __LINE__,
                                   "1's based counting, must be within bounds");
    return array[r - 1][c - 1];
}

precision& matrix::at(letters i) noexcept(false) {
    short idx = basal::to_underlying(i);
    size_t index = static_cast<size_t>(idx > 0 ? idx : 0);
    basal::exception::throw_unless(0 < index && index <= (rows * cols), g_filename, __LINE__,
                                   "1's based counting, must be within bounds");
    size_t r = (index - 1) / cols;
    size_t c = (index - 1) % cols;
    return array[r][c];
}

precision matrix::at(letters i) const noexcept(false) {
    short idx = basal::to_underlying(i);
    size_t index = static_cast<size_t>(idx > 0 ? idx : 0);
    basal::exception::throw_unless(0 < index && index <= (rows * cols), g_filename, __LINE__,
                                   "1's based counting, must be within bounds");
    size_t r = (index - 1) / cols;
    size_t c = (index - 1) % cols;
    return array[r][c];
}

void matrix::assignInto(matrix& dst, size_t start_row, size_t start_col) {
    basal::exception::throw_unless(start_row + rows <= dst.rows, g_filename, __LINE__,
                                   "Not enough destination space for rows");
    basal::exception::throw_unless(start_col + cols <= dst.cols, g_filename, __LINE__,
                                   "Not enough destination space for columns");
    ref_coord_iterator iter = [&](size_t row, size_t col, precision& v) { dst[start_row + row][start_col + col] = v; };
    for_each(iter);
}

bool matrix::operator==(matrix const& a) const {
    basal::exception::throw_unless(a.rows == rows && a.cols == cols, g_filename, __LINE__, "Must be equal dimensions");
    bool match = true;
    const_coord_iterator iter = [&](size_t r, size_t c, precision v) {
        if (!basal::nearly_equals(a[r][c], v))
            match = false;
    };
    for_each(iter);
    return match;
}

bool matrix::operator!=(matrix const& a) const {
    basal::exception::throw_unless(a.rows == rows && a.cols == cols, g_filename, __LINE__, "Must be equal dimensions");
    bool match = true;
    const_coord_iterator iter = [&](size_t r, size_t c, precision v) {
        if (!basal::nearly_equals(a[r][c], v))
            match = false;
    };
    for_each(iter);
    return !match;
}

matrix& matrix::operator*=(precision const a) {
    ref_iterator iter = [&](precision& v) { v *= a; };
    return for_each(iter);
}

matrix& matrix::operator/=(precision const a) {
    ref_iterator iter = [&](precision& v) { v /= a; };
    return for_each(iter);
}

matrix matrix::copy() {
    return matrix(*this);
}

precision matrix::trace() const {
    basal::exception::throw_unless(rows == cols, g_filename, __LINE__);
    precision sum = 0.0_p;
    for (size_t i = 0; i < rows; i++) {
        sum += array[i][i];
    }
    return sum;
}

matrix matrix::transpose() const {
    matrix AT{cols, rows};  // create new matrix.
    // NOTE a generic inplace transpose is nearly impossible.
    // this will iterator ourselves, not the AT matrix.
    const_coord_iterator iter = [&](size_t r, size_t c, precision v) { AT[c][r] = v; };
    for_each(iter);
    return AT;
}

matrix matrix::T() const {
    return transpose();
}

matrix& matrix::negative() {
    operator*=(-1.0_p);
    return (*this);
}

matrix matrix::negatived() const {
    return matrix(*this).negative();
}

precision matrix::magnitude() const {
    return determinant();
}

bool matrix::singular() const {
    if (rows != cols)
        return false;
    return basal::nearly_zero(magnitude());
}

bool matrix::degenerate() const {
    return singular();
}

bool matrix::orthagonal() const {
    if (rows != cols) {
        return false;
    }

    using namespace operators;

    // Q^T*Q == Q*Q^T == I
    matrix qtq = T() * (*this);
    matrix qqt = (*this) * T();
    matrix I = matrix::identity(rows, cols);
    return qtq == qqt && qqt == I;
}

bool matrix::invertible() const {
    if (rows == cols && !basal::nearly_zero(determinant()))
        return true;
    return false;
}

bool matrix::symmetric() const {
    return (T() == *this);
}

bool matrix::skew_symmetric() const {
    return (T() == negatived());
}

bool matrix::diagonal() const {
    bool diag = true;
    const_coord_iterator iter = [&](size_t row, size_t col, precision const& v) {
        if (row != col && !basal::nearly_zero(v)) {
            diag = false;
        }
    };
    for_each(iter);
    return diag;
}

bool matrix::triangular() const {
    return lower_triangular() || upper_triangular();
}

bool matrix::lower_triangular() const {
    bool ret = true;
    const_coord_iterator iter = [&](size_t row, size_t col, precision v) {
        if (col > row && !basal::nearly_zero(v)) {
            ret = false;
        }
    };
    for_each(iter);
    return ret;
}

bool matrix::upper_triangular() const {
    bool ret = true;
    const_coord_iterator iter = [&](size_t row, size_t col, precision v) {
        if (row > col && !basal::nearly_zero(v)) {
            ret = false;
        }
    };
    for_each(iter);
    return ret;
}

bool matrix::eigenvalue(precision lambda) const {
    using namespace operators;
    matrix lamI = lambda * matrix::identity(rows, cols);
    return (basal::nearly_zero(det(lamI - (*this))));
}

matrix matrix::eigenvalues() const noexcept(false) {
    basal::exception::throw_unless(rows == cols, g_filename, __LINE__, "Must be a square matrix");
    if (rows == 2 or rows == 3) {
        // solve the determinant
        // (a - L)*(d - L) - b*c = 0
        // a*d - d*L - a*L +L*L - b*c = 0
        precision a = 1.0_p;
        precision b = -trace();
        precision c = determinant();
        auto roots = quadratic_roots(a, b, c);
        return matrix{{{std::get<0>(roots)}, {std::get<1>(roots)}}};
    } else {  // if (rows == 3) {
        // solve the determinant

        // fail for now
        // TODO (Linalg) implement eigenvalues() for 3x3 or largers
        basal::exception::throw_if(true, g_filename, __LINE__, "TODO Implement for 3x3 or larger");
    }
    return matrix::col(rows);
}

matrix matrix::inverse() const noexcept(false) {
    basal::exception::throw_unless(rows == cols, g_filename, __LINE__,
                                   "Must be a square matrix");  // no inverses for non square matrix
    matrix m{rows, cols};
    precision det = determinant();
    basal::exception::throw_if(basal::nearly_zero(det), g_filename, __LINE__, "Matrix is singular, not invertible");

    if (rows == 1) {
        m[0][0] = 1.0_p / det;
    } else if (rows == 2) {
        m[0][0] = array[1][1] / det;
        m[0][1] = -array[0][1] / det;
        m[1][0] = -array[1][0] / det;
        m[1][1] = array[0][0] / det;
    } else {
        using namespace operators;
        return (1.0_p / det) * adjugate();
    }
    return m;
}

matrix matrix::sub(size_t nrow, size_t ncol) const {
    matrix s{rows - 1, cols - 1};
    for (size_t r = 0, j = 0; r < rows; r++) {
        if (r == nrow)
            continue;
        for (size_t c = 0, i = 0; c < cols; c++) {
            if (c == ncol)
                continue;
            s[j][i] = array[r][c];
            i++;
        }
        j++;
    }
    return s;
}

matrix matrix::subset(size_t row, size_t col, size_t nrows, size_t ncols) const noexcept(false) {
    basal::exception::throw_unless(nrows + row <= rows, g_filename, __LINE__,
                                   "Subset rows request is larger than parent");
    basal::exception::throw_unless(ncols + col <= cols, g_filename, __LINE__,
                                   "Subset cols request is larger than parent");
    matrix m{nrows, ncols};
    for (size_t r = row, mr = 0; r < rows && mr < m.rows; r++, mr++) {
        for (size_t c = col, mc = 0; c < cols && mc < m.cols; c++, mc++) {
            m[mr][mc] = array[r][c];
        }
    }
    return m;
}

matrix matrix::resize(size_t rs, size_t cs) noexcept(false) {
    basal::exception::throw_unless((rs * cs) == (rows * cols), g_filename, __LINE__,
                                   "Must have same number of elements");
    matrix _new(rs, cs);
    for (size_t idx = 0; idx < rows * cols; idx++) {
        _new.index(idx) = index(idx);
    }
    return _new;
}

precision matrix::minor(size_t nrow, size_t ncol) const {
    return sub(nrow, ncol).determinant();
}

precision matrix::cofactor(size_t nrow, size_t ncol) const {
    return minor(nrow, ncol) * pow(-1.0_p, (nrow + 1) + (ncol + 1));
}

matrix matrix::minors() const {
    ref_coord_iterator iter = [&](size_t r, size_t c, precision& v) { v = minor(r, c); };
    return matrix{rows, cols}.for_each(iter);
}

matrix matrix::comatrix() const {
    ref_coord_iterator iter = [&](size_t r, size_t c, precision& v) { v = cofactor(r, c); };
    return matrix{rows, cols}.for_each(iter);
}

matrix matrix::adjugate() const {
    return comatrix().T();
}

precision matrix::determinant() const noexcept(false) {
    precision det = 0.0_p;
    basal::exception::throw_unless(rows == cols, g_filename, __LINE__, "Must be a square matrix");
    statistics::get().matrix_determinants++;
    if (rows == 1) {
        // identity
        det = at(1, 1);
    } else if (rows == 2) {
        // fprintf(stdout, "det shortcut! ad-bc\n");
        det = at(1, 1) * at(2, 2) - at(1, 2) * at(2, 1);
    } else if (rows == 3) {
        // fprintf(stdout, "Sarrus's rule\n");
        det = at(1, 1) * (at(2, 2) * at(3, 3) - at(3, 2) * at(2, 3))
              - at(1, 2) * (at(3, 3) * at(2, 1) - at(2, 3) * at(3, 1))
              + at(1, 3) * (at(2, 1) * at(3, 2) - at(3, 1) * at(2, 2));
    } else {
        // fprintf(stdout, "Co-factors method\n");
        for (size_t c = 0; c < cols; c++) {
            det += cofactor(0, c) * array[0][c];
        }
    }
    return det;
}

matrix matrix::random(size_t rows, size_t cols, precision min, precision max, precision p) {
    matrix R{rows, cols};
    std::uniform_real_distribution<precision> unif(min, max);
    // std::default_random_engine re;
    std::random_device rd;
    std::mt19937 gen(rd());
    ref_iterator iter = [&](precision& v) { v = std::ceil(unif(gen) * std::pow(10.0_p, p)) / std::pow(10.0_p, p); };
    return R.for_each(iter);
}

matrix& matrix::random(precision min, precision max, precision p) {
    std::uniform_real_distribution<precision> unif(min, max);
    std::random_device rd;
    std::mt19937 gen(rd());
    ref_iterator iter = [&](precision& v) { v = std::ceil(unif(gen) * std::pow(10.0_p, p)) / std::pow(10.0_p, p); };
    return for_each(iter);
}

// element-wise accumulator
matrix& matrix::operator+=(matrix const& a) {
    basal::exception::throw_unless(a.rows == rows, g_filename, __LINE__);
    basal::exception::throw_unless(a.cols == cols, g_filename, __LINE__);
    ref_coord_iterator iter = [&](size_t r, size_t c, precision& v) { v += a[r][c]; };
    return for_each(iter);
}

// element-wise decumulator
matrix& matrix::operator-=(matrix const& a) {
    basal::exception::throw_unless(a.rows == rows, g_filename, __LINE__);
    basal::exception::throw_unless(a.cols == cols, g_filename, __LINE__);
    ref_coord_iterator iter = [&](size_t r, size_t c, precision& v) { v -= a[r][c]; };
    return for_each(iter);
}

void matrix::print(std::ostream& os, char const name[]) const {
    os << name << " matrix = {\n";
    for (size_t r = 0; r < rows; r++) {
        os << "\t{";
        for (size_t c = 0; c < cols; c++) {
            os << array[r][c] << (c == (cols - 1) ? "}," : ", ");
        }
        os << (r == (rows - 1) ? "\n};\n" : "\n");
    }
}

std::ostream& operator<<(std::ostream& os, matrix const& m) {
    m.print(os, "");
    return os;
}

void matrix::row_swap(size_t row_a, size_t row_b) {
    basal::exception::throw_unless(row_a < rows, g_filename, __LINE__);
    basal::exception::throw_unless(row_b < rows, g_filename, __LINE__);
    if (row_a != row_b) {
        std::swap(array[row_a], array[row_b]);
    }
}

void matrix::row_scale(size_t row, precision a) {
    basal::exception::throw_unless(row < rows, g_filename, __LINE__);
    for (size_t c = 0; c < cols; c++) {
        array[row][c] *= a;
    }
}

bool matrix::row_compare(size_t row, precision a) {
    basal::exception::throw_unless(row < rows, g_filename, __LINE__);
    bool match = true;
    for (size_t c = 0; c < cols; c++) {
        if (!basal::nearly_equals(array[row][c], a)) {
            match = false;
            break;
        }
    }
    return match;
}

void matrix::row_add(size_t row_dst, size_t row_src, precision r) {
    basal::exception::throw_unless(row_dst < rows, g_filename, __LINE__);
    basal::exception::throw_unless(row_src < rows, g_filename, __LINE__);
    basal::exception::throw_unless(row_src != row_dst, g_filename, __LINE__);
    for (size_t c = 0; c < cols; c++) {
        array[row_dst][c] += (r * array[row_src][c]);
    }
}

void matrix::row_sub(size_t row_dst, size_t row_src, precision a) {
    basal::exception::throw_unless(row_dst < rows, g_filename, __LINE__);
    basal::exception::throw_unless(row_src < rows, g_filename, __LINE__);
    basal::exception::throw_unless(row_src != row_dst, g_filename, __LINE__);
    for (size_t c = 0; c < cols; c++) {
        array[row_dst][c] = (array[row_dst][c] - (a * array[row_src][c]));
    }
}

// normal multiply
matrix& matrix::operator*=(matrix const& a) {
    using namespace operators;
    matrix m = (*this) * a;  // create new matrix
    (*this) = m;             // copy assignment
    return *this;
}

matrix& matrix::operator/=(matrix const& a) {
    using namespace operators;
    matrix m = (*this) / a;
    (*this) = m;
    return *this;
}

matrix& matrix::for_each_row(std::function<void(size_t r)> func) {
    for (size_t r = 0; r < rows; r++) {
        func(r);
    }
    return *this;
}
matrix& matrix::for_each_col(std::function<void(size_t c)> func) {
    for (size_t c = 0; c < cols; c++) {
        func(c);
    }
    return *this;
}

bool matrix::row_is_zero(size_t r, size_t sc) const {
    bool match = true;
    for (size_t c = sc; c < cols; c++) {
        if (!basal::nearly_zero(array[r][c])) {
            match = false;
        }
    }
    return match;
}

bool matrix::col_is_zero(size_t c, size_t sr) const {
    bool match = true;
    for (size_t r = sr; r < rows; r++) {
        if (!basal::nearly_zero(array[r][c])) {
            match = false;
        }
    }
    return match;
}

bool matrix::elem_is_zero(size_t r, size_t c) const {
    return basal::nearly_zero(array[r][c]);
}

matrix& matrix::sort_zero_rows() {
    // push zeroes to the bottom.
    for (size_t r = 0, lr = rows - 1; r <= rows && r < lr; /*no inc*/) {
        bool zr = row_is_zero(r);
        bool zlr = row_is_zero(lr);
        if (zr && zlr) {  // if both zero, move lr up try again
            lr--;
            continue;
        } else if (zr && !zlr) {  // swap and move both idx
            row_swap(r++, lr--);
            continue;
        } else if (!zr && zlr) {  // move both idx
            r++;
            lr--;
            continue;
        } else if (!zr && !zlr) {  // move down a row
            r++;                   /* leave lr */
            continue;
        }
    }
    return (*this);
}

matrix& matrix::sort_leading_nonzero(size_t col, size_t i, size_t& j) {
    for (size_t r = i; r < rows && r < j; /* */) {
        bool ze = basal::nearly_zero(array[r][col]);
        bool jze = basal::nearly_zero(array[j][col]);
        if (ze && jze) {
            // both are zero... but we know there's one in here
            // r++;
            j--;
        } else if (ze && !jze) {
            row_swap(r, j);
            r++;
            j--;
        } else if (!ze && jze) {
            // good, move on
            r++;
        } else if (!ze && !jze) {
            // move down one to see if we need to swap somethign below with
            // the bottom
            r++;
        }
    }
    return (*this);
}

void matrix::PLU(matrix& P, matrix& L, matrix& U) const noexcept(false) {
    auto plu = PLU();
    P = std::get<0>(plu);
    L = std::get<1>(plu);
    U = std::get<2>(plu);
}

std::tuple<matrix, matrix, matrix> matrix::PLU() const noexcept(false) {
    basal::exception::throw_unless(rows == cols, g_filename, __LINE__, "PLU only allowed on square matrix");
    matrix P = matrix::identity(rows, cols);
    matrix L = matrix::identity(rows, cols);
    matrix U{*this};  // copy
    for (size_t c = 0, r = 0, lr = (U.rows - 1); c < U.cols && r < U.rows; c++) {
        if (U.col_is_zero(c, r)) {
            continue;
        }
        for (size_t ir = r; ir < rows && ir < lr; /* */) {
            bool ze = U.elem_is_zero(ir, c);
            bool lze = U.elem_is_zero(lr, c);
            if (ze && lze) {
                lr--;
            } else if (ze && !lze) {
                U.row_swap(ir, lr);
                P.row_swap(ir, lr);
                ir++;
                lr--;
            } else if (!ze && lze) {
                ir++;
            } else if (!ze && !lze) {
                ir++;
            }
        }
        for (size_t ir = r + 1; ir < U.rows; ir++) {
            if (!basal::nearly_zero(U[ir][c])) {
                precision v = U[ir][c] / U[r][c];
                // printf("L[%u][%u] = %E\n", ir, c, v);
                L[ir][c] = v;
                U.row_sub(ir, r, v);
            }
        }
        r++;
        lr = (rows - 1);
    }
    return std::make_tuple(P, L, U);
}

matrix matrix::escheloned(size_t stop_col) const {
    matrix m{*this};
    return m.eschelon(stop_col);
}

matrix& matrix::eschelon(size_t stop_col) {
    if (stop_col > cols) {
        stop_col = cols;
    }
    for (size_t c = 0, r = 0, lr = (rows - 1); c < stop_col && r < rows; c++) {
        // if the column is zero (from this row down)
        if (col_is_zero(c, r)) {
            // skip the column
            continue;
        }
        sort_leading_nonzero(c, r, lr);  // in this column
        // printf("Pivot: %E\n", array[r][c]);
        // r aught to be on the pivot then
        row_scale(r, 1.0_p / array[r][c]);
        for (size_t i = r + 1; i < rows; i++) {
            if (!basal::nearly_zero(array[i][c])) {
                row_scale(i, -1.0_p / array[i][c]);
                row_add(i, r, 1.0_p);
            }
        }
        // reset counters
        r++;
        lr = (rows - 1);
    }
    return (*this);
}

// inplace operation
matrix& matrix::reduce(size_t stop_row) {
    // take a row_eschelon matrix and reduce each column with a leading 1 to be the only
    // nonzero value
    static_assert(std::is_unsigned<decltype(stop_row)>::value, "Must be an unsigned input type");
    if (stop_row > rows) {
        // throw basal::exception("Stop Row is out of bounds!", __FILE__, __LINE__);
        stop_row = rows;
    }
    auto lnz = leading_nonzero();
    for (size_t i = 0; i < lnz.size(); i++) {
        // pivot column
        size_t pc = lnz[i];
        // pivot row
        size_t pr = i;
        // zero row gets skipped
        if (pc == std::numeric_limits<size_t>::max()) {
            continue;
        }
        for (size_t r = 0; r < stop_row; r++) {
            // if the row is the same as the row in the lnz skip
            if (r == pr) {
                continue;
            }
            // get the value at this row
            precision v = array[r][pc];
            // if this row entry is non-zero, scale it to zero
            if (!basal::nearly_zero(v)) {
                // make the dst col element of this row zero
                row_add(r, pr, -v);
            }
        }
    }
    return (*this);
}

matrix matrix::reduced(size_t stop_row) const {
    matrix a{*this};  // copy constructor
    return a.reduce(stop_row);
}

std::vector<size_t> matrix::leading_nonzero(size_t stop_col) const {
    std::vector<size_t> lnz(rows);
    // initialize to zero rows
    for (auto& i : lnz) {
        i = std::numeric_limits<size_t>::max();
    }
    if (stop_col > cols) {
        stop_col = cols;
    }
    for (size_t r = 0; r < rows; r++) {
        for (size_t c = 0; c < stop_col; c++) {
            if (not basal::nearly_zero(array[r][c])) {
                lnz[r] = c;
                break;
            }
        }
    }
    return lnz;
}

std::vector<size_t> matrix::pivots() const {
    std::vector<size_t> lnz = leading_nonzero();  // get the locations PER ROW
    // same as lnz but don't mention -1
    std::vector<size_t> pc;
    for (auto& c : lnz) {
        if (c < cols) {
            pc.push_back(c);
        }
    }
    return pc;
}

std::vector<size_t> matrix::frees() const {
    // free variable indexes
    std::vector<size_t> fvi(cols);
    // initialize to max;
    for (auto& i : fvi) {
        i = std::numeric_limits<size_t>::max();
    }
    // now get the pivots and modify the fvi to show which are not the pivots
    std::vector<size_t> pvc = pivots();  // e.g. {0,1,3}
    // clear the pivots and leave the
    for (auto& c : pvc) {
        fvi[c] = 0;
    }  // e.g. {0, 0, SIZE_T_MAX, 0, ...}
    std::vector<size_t> fvc;
    for (size_t c = 0; c < cols; c++) {
        if (fvi[c] >= cols) {
            fvc.push_back(c);
        }
    }
    return fvc;
}

std::vector<bool> matrix::is_pivots() const {
    // initialize to free variables
    std::vector<bool> bpv(cols, false);
    // get the pivots
    auto pv = pivots();
    // set the pivot columns
    for (auto& c : pv) {
        bpv[c] = true;
    }
    return bpv;
}

size_t matrix::rank() const {
    matrix A = escheloned().reduce();  // produce the the reduced row eschelon format
    // now count the number of non-zero rows
    size_t rank = 0;
    A.for_each_row([&](size_t r) { rank += A.row_is_zero(r) ? 0 : 1; });
    return rank;
}

matrix matrix::basis() const noexcept(false) {
    matrix A = escheloned().reduce();
    auto pc = A.pivots();
    basal::exception::throw_if(pc.size() == 0, g_filename, __LINE__, "No pivot variables in rref matrix\n");
    matrix B = matrix::zeros(A.rows, pc.size());
    size_t np = 0;
    for (auto c : pc) {
        matrix C = A.col(c);
        C.assignInto(B, 0, np++);
    }
    return B;
}

matrix matrix::nullspace() const noexcept(false) {
    // copy & rref
    matrix A = escheloned().reduce();
    auto fv = A.frees();
    basal::exception::throw_if(fv.size() == 0, g_filename, __LINE__, "No free variables in rref matrix");
    auto bpv = A.is_pivots();
    // create a zero matrix then assign each free variable column into it (including a 1 for itself)
    matrix B = matrix::zeros(A.cols, fv.size());
    size_t nf = 0;
    for (auto c : fv) {
        // get the column matrix
        matrix C = A.col(c).negative();
        // assign into B matrix
        for (size_t br = 0, cr = 0; br < B.rows; br++) {
            if (br != c) {            // variable is not the free variable in this column
                if (bpv[br] == true)  // variable is not a free variable in another column
                    B[br][nf] = C[cr++][0];
                else
                    B[br][nf] = 0.0_p;  // column is a free variable in another column
            } else if (br == c) {
                B[br][nf] = 1;  // column is the free variable in this spot
            }
        }
        nf++;
    }
    return B;
}

// NON-CLASS OPERATIONS
namespace operators {

matrix operator^(matrix& a, int p) noexcept(false) {
    if (p == 0) {
        return matrix::identity(a.rows, a.cols);
    } else if (p == 1) {
        return matrix{a};
    } else if (p == -1) {
        return a.inverse();
    } else if (p > 1) {
        return (a ^ (p - 1)) * a;
    } else {  // if (p < -1) {
        if ((std::abs(p) & 1) == 1) {
            return a.inverse();
        } else {
            return matrix{a};  // copy
        }
    }
}

matrix operator^(matrix& a, letters l) noexcept(false) {
    basal::exception::throw_unless(l == letters::T, g_filename, __LINE__,
                                   "Special transpose semantics require value of letter");
    if (l == letters::T)
        return a.T();  // can't easily mark with basal::oper_e without incurring copy
    else
        return matrix::identity(a.rows, a.cols);
}

matrix operator^(matrix const& a, letters l) noexcept(false) {
    matrix b{a};  // const copy
    return operator^(b, l);
}

}  // namespace operators

matrix addition(matrix const& a, matrix const& b) noexcept(false) {
    // a.print("a");
    // b.print("b");
    basal::exception::throw_unless(a.rows == b.rows, g_filename, __LINE__);
    basal::exception::throw_unless(a.cols == b.cols, g_filename, __LINE__);
    matrix c{a};  // copy constructor
    c += b;       // class +=
    return c;     // returns a full object?
}

matrix subtraction(matrix const& a, matrix const& b) noexcept(false) {
    basal::exception::throw_unless(a.rows == b.rows, g_filename, __LINE__);
    basal::exception::throw_unless(a.cols == b.cols, g_filename, __LINE__);
    matrix c{a};  // copy constructor
    c -= b;       // class +=
    return c;     // returns a full object?
}

matrix multiply(matrix const& a, matrix const& b) noexcept(false) {
    basal::exception::throw_unless(a.cols == b.rows, g_filename, __LINE__, "Columns and Rows must match!");
    matrix m{a.rows, b.cols};
    statistics::get().matrix_multiply++;
    for (size_t r = 0; r < m.rows; r++) {
        for (size_t c = 0; c < m.cols; c++) {
            m[r][c] = 0.0_p;
            for (size_t i = 0; i < a.cols; i++) {
                m[r][c] += (a[r][i] * b[i][c]);
                // printf("m[%u][%u] = %lf += (%lf * %lf)\n", r, c, m[r][c], a[r][i], b[i][c]);
            }
        }
    }
    return m;
}

matrix multiply(matrix const& a, precision const r) noexcept(false) {
    matrix m{a};  // copy
    return m *= r;
}

matrix multiply(precision const r, matrix const& a) noexcept(false) {
    return multiply(a, r);
}

matrix hadamard(matrix const& a, matrix const& b) noexcept(false) {
    basal::exception::throw_unless(a.rows == b.rows, g_filename, __LINE__);
    basal::exception::throw_unless(a.cols == b.cols, g_filename, __LINE__);
    matrix c{a.rows, b.cols};
    return c.for_each([&](size_t y, size_t x, precision& v) { v = a[y][x] * b[y][x]; });
}

namespace pairwise {
matrix multiply(matrix const& A, matrix const& B) noexcept(false) {
    basal::exception::throw_unless(A.rows == 1 and B.cols == 1, g_filename, __LINE__, "A[%dx%d] (pair) B[%dx%d]",
                                   A.rows, A.cols, B.rows, B.cols);
    matrix C{B.rows, A.cols};
    return C.for_each([&](size_t y, size_t x, precision& v) { v = A[0][x] * B[y][0]; });
}
}  // namespace pairwise

namespace rowwise {
matrix scale(matrix const& a, matrix const& b) noexcept(false) {
    basal::exception::throw_unless(b.cols == 1, g_filename, __LINE__, "Must be a column matrix (%u)", b.cols);
    matrix m{a.rows, a.cols};
    return m.for_each([&](size_t y, size_t x, precision& v) { v = a[y][x] * b[y][0]; });
}
}  // namespace rowwise

void swap(matrix& a, matrix& b) noexcept(false) {
    basal::exception::throw_unless(a.rows == b.rows, g_filename, __LINE__);
    basal::exception::throw_unless(a.cols == b.cols, g_filename, __LINE__);
    std::swap(a.memory, b.memory);
    for (size_t r = 0; r < a.rows; r++) {
        std::swap(a.array[r], b.array[r]);
    }
}

matrix rowjoin(matrix& a, matrix& b) noexcept(false) {
    basal::exception::throw_unless(a.rows == b.rows, g_filename, __LINE__, "To join, the number of rows must be equal");
    matrix::ref_coord_iterator iter = [&](size_t r, size_t c, precision& v) {
        if (c < a.cols)
            v = a[r][c];
        else
            v = b[r][c - a.cols];
    };
    return matrix(a.rows, a.cols + b.cols).for_each(iter);
}

matrix coljoin(matrix const& a, matrix const& b) noexcept(false) {
    basal::exception::throw_unless(a.cols == b.cols, g_filename, __LINE__, "To join, the number of cols must be equal");
    matrix::ref_coord_iterator iter = [&](size_t r, size_t c, precision& v) {
        if (r < a.rows)
            v = a[r][c];
        else
            v = b[r - a.rows][c];
    };
    return matrix(a.rows + b.rows, a.cols).for_each(iter);
}

matrix matrix::rule_of_sarrus() noexcept(false) {
    basal::exception::throw_if(rows != cols, g_filename, __LINE__, "Must be a square matrix");
    matrix sub = subset(0, 0, rows, cols - 1);
    return rowjoin(*this, sub);
}

bool matrix::to_file(std::string path) const {
    FILE* fp = fopen(path.c_str(), "wb+");
    if (fp) {
        size_t elem = 0;
        elem += fwrite(&rows, sizeof(rows), 1, fp);
        elem += fwrite(&cols, sizeof(cols), 1, fp);
        for_each([&](precision const& v) { elem += fwrite(&v, sizeof(v), 1, fp); });
        fclose(fp);
        return (elem == (rows * cols) + 2);
    }
    return false;
}

matrix matrix::from_file(std::string path) {
    FILE* fp = fopen(path.c_str(), "rb+");
    if (fp) {
        size_t rows, cols;
        size_t elem = 0;
        elem += fread(&rows, sizeof(rows), 1, fp);
        elem += fread(&cols, sizeof(cols), 1, fp);
        matrix m{rows, cols};
        m.for_each([&](precision& v) {
            precision value;
            elem += fread(&value, sizeof(value), 1, fp);
            v = value;
        });
        fclose(fp);
        return m;
    }
    return matrix{};
}

// ****************************************************************************
}  // namespace linalg
// ****************************************************************************
