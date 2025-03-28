#pragma once
/// @file
/// Definitions for matrix object.
/// @copyright Copyright 2019 (C) Erik Rainey.

#include <basal/exception.hpp>
#include <basal/printable.hpp>
#include <basal/ieee754.hpp>
#include <cinttypes>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <vector>

#include "linalg/types.hpp"

#if defined(__linux__)
#undef minor
#endif

namespace linalg {

using namespace basal::literals;

/// The matrix objects, the core of the linear algebra library.
class matrix : public basal::printable {
public:
    /// The constant number of rows in the matrix
    size_t const rows;
    /// The constant number of columns in the matrix
    size_t const cols;

protected:
    /// The size of the allocation in bytes
    size_t const bytes;
    /// Used to remember if the memory was imported from an external allocator
    bool const external_memory;
    /// The pointer to the internal memory allocation
    precision *memory;
    /// The array of memory pointers
    precision **array;

    /// Allocates the memory for the array and the memory pointers
    bool create(size_t rows, size_t cols, size_t bytes);

    /// Frees the memory related to the matrix.
    void destroy();

    /// Protected child class constructor
    matrix(size_t rows, size_t cols, bool allocate);

public:
    /// A user defined constructor (which assumes at least a 1x1)
    matrix(size_t rows = 3, size_t cols = 3);

    /// A user defined copy constructor from basic precision array
    matrix(size_t rows, size_t cols, precision mat[]);

    /// Explicit Constructor for a 2x2 precision matrix
    explicit matrix(precision m[2][2]);
    /// Explicit Constructor for a 3x3 precision matrix
    explicit matrix(precision m[3][3]);
    /// Explicit Constructor for a 4x4 precision matrix
    explicit matrix(precision m[4][4]);
    /// Explicit constructor for a nested vector of precisions
    explicit matrix(std::vector<std::vector<precision>> const &a);

    /// Copy constructor
    matrix(matrix const &a) noexcept(false);
    /// Move constructor
    matrix(matrix &&a) noexcept(false);
    /// Copy assignment
    matrix &operator=(matrix const &a) noexcept(false);
    /// Move assignment
    matrix &operator=(matrix &&a) noexcept(false);
    /// Assignment operator, fills each matrix value with v
    void operator=(precision const v);
    /// Virtual Destructor
    virtual ~matrix();

    /// Static method to create an identity matrix
    static matrix identity(size_t rows, size_t cols);

    /// Static method to create a zero matrix
    static matrix zeros(size_t rows, size_t cols);

    /// Static method to create a ones matrix
    static matrix ones(size_t rows, size_t cols);

    /// Static method to create a random value matrix
    static matrix random(size_t rows, size_t cols, precision min = 0.0_p, precision max = 1.0_p, precision p = 3.0_p);

    /// Makes a row matrix from a vector
    static matrix row(std::vector<precision> &data);

    /// Makes a column matrix from a vector
    static matrix col(std::vector<precision> &data);

    /// Creates a copy of the matrix
    matrix copy();

    /// zeros the matrix and returns a reference
    matrix &zero();

    /// Fills a matrix with a specific value
    matrix &fill(precision v);

    /// Fill a matrix with random values between the given parameters
    matrix &random(precision min = 0.0_p, precision max = 1.0_p, precision p = 3.0_p);

    /// Returns the trace of a matrix
    precision trace() const;

    /// Returns the value at the row and column. 0 based indexing.
    virtual precision &index(size_t idx) noexcept(false);
    /// Returns the value at the row and column. 0 based indexing.
    virtual precision index(size_t const idx) const noexcept(false);

    /// Returns the value at the row and column. 0 based indexing.
    virtual precision &index(size_t row, size_t col) noexcept(false);
    /// Returns the value at the row and column. 0 based indexing.
    virtual precision index(size_t const row, size_t const col) const noexcept(false);

    /// Returns the value at the row and column. 0 based indexing.
    precision operator()(size_t const row, size_t const col) const noexcept(false);
    /// Returns the value at the row and column. 0 based indexing.
    precision &operator()(size_t row, size_t col) noexcept(false);

    /// Returns a pointer of a row which can be further indexed
    virtual precision const *operator[](size_t idx) const noexcept(false);
    /// Returns a const pointer to a const value row
    virtual precision *operator[](size_t idx) noexcept(false);

    /// This method uses a 1 based, not 0 based indexes
    virtual precision &at(short r, short c) noexcept(false);
    /// This method uses a 1 based, not 0 based indexes
    virtual precision at(short r, short c) const noexcept(false);

    /// This method uses a 1 based, not 0 based indexes
    virtual precision &at(letters i) noexcept(false);
    /// This method uses a 1 based, not 0 based indexes
    virtual precision at(letters i) const noexcept(false);

    /// Copies the matrix into another matrix at a specified row and column.
    void assignInto(matrix &dst, size_t start_row, size_t start_col);

    // linear algebra ops
    matrix &operator+=(matrix const &a);
    matrix &operator-=(matrix const &a);
    matrix &operator*=(matrix const &a);
    matrix &operator/=(matrix const &a);

    // scalar ops
    matrix &operator*=(precision const r);
    matrix &operator/=(precision const r);

    // comparisons
    bool operator==(matrix const &a) const;
    bool operator!=(matrix const &a) const;

    /// Don't allow the bool operators as it's too ambiguous
    explicit operator bool() const = delete;

    /// Returns the inverse of the matrix
    matrix inverse() const noexcept(false);
    /// Returns the determinant of the matrix
    precision determinant() const noexcept(false);
    /// Returns the magnitude of the matrix
    virtual precision magnitude() const;
    /// Returns the transpose of the matrix
    matrix transpose() const;
    matrix T() const;  ///< shortening of the transpose()

    /// Removes one row and column to make a sub-matrix.
    /// \note This creates a copy of the memory and does not refer to the original memory */
    matrix sub(size_t number_of_rows, size_t number_of_columns) const;

    /// Create a new matrix from a subset of this matrix
    matrix subset(size_t row, size_t col, size_t number_of_rows, size_t number_of_columns) const noexcept(false);

    /// Creates a subset from a single row
    inline matrix row(size_t r) const noexcept(false) {
        return subset(r, 0, 1, cols);
    }

    /// Creates a subset from a single column
    inline matrix col(size_t c) const noexcept(false) {
        return subset(0, c, rows, 1);
    }

    /// Resizes the existing matrix into a matrix of a new dimensions.
    /// @warning Each element will be copied in index order to the new matrix
    ///
    matrix resize(size_t rs, size_t cs) noexcept(false);

#undef minor  // somewhere deep in Unix land this is used to make minor numbers

    /// Returns the determinant of the sub matrix
    precision minor(size_t number_of_rows, size_t number_of_columns) const;
    /// Returns a matrix of the minors of the object matrix
    matrix minors() const;
    /// Returns the minor multiplied by a power
    precision cofactor(size_t number_of_rows, size_t number_of_columns) const;
    /// Return a matrix of cofact   ors
    matrix comatrix() const;
    /// Returns the tranpose of the comatrix
    matrix adjugate() const;
    /// Modifies the matrix to be the negative version of the matrix
    matrix &negative();
    /// Returns the negative version of the matrix
    virtual matrix negatived() const;

    /// Determines if the matrix is singular
    bool singular() const;
    /// Determines if the matrix is degenerate
    bool degenerate() const;
    /// Determines if the matrix is orthogonal
    bool orthagonal() const;
    /// Determines if a matrix has an inverse.
    bool invertible() const;
    /// Determines if a matrix is symmetric
    bool symmetric() const;
    /// Determines if a matrix is skew-symmetric
    bool skew_symmetric() const;

#if 0
    /// Determines if the matrix is elementary
    /// TODO (Linalg) implement bool elementary();
#endif
    /// Determines if the matrix is diagonal
    bool diagonal() const;
    /// Determines if a matrix is lower or upper triangular
    bool triangular() const;
    /// Determines if a matrix is lower triangular
    bool lower_triangular() const;
    /// Determines if a matrix is upper triangular
    bool upper_triangular() const;

    /// Determines if a value is an eigenvalue of a matrix
    bool eigenvalue(precision lambda) const;

    /// Returns the eigenvalues of the matrix
    matrix eigenvalues() const noexcept(false);

    /// Returns a new matrix from a square matrix which is extended by the Rule of Sarrus
    matrix rule_of_sarrus() noexcept(false);

    /// Reduces the matrix and returns a reference to itself.
    /// @param stop_row The row to stop the algorithm on. Defaults too all rows.
    matrix &reduce(size_t stop_row = std::numeric_limits<size_t>::max());
    /// Copies and reduces a matrix
    /// @param stop_row The row to stop the algorithm on. Defaults too all rows.
    matrix reduced(size_t stop_row = std::numeric_limits<size_t>::max()) const;

    /// Computes the PLU decomposition of the matrix.
    /// Returns the PLU matrices through the reference parameters.
    ///
    void PLU(matrix &P, matrix &L, matrix &U) const noexcept(false);

    /// Computes the PLU decomposition of the matrix.
    /// \return tuple of matricies.
    ///
    std::tuple<matrix, matrix, matrix> PLU() const noexcept(false);

    /// For use with const methods
    typedef std::function<void(size_t, size_t, precision const &)> const_coord_iterator;

    /// For use with non-const methods
    typedef std::function<void(size_t, size_t, precision &)> ref_coord_iterator;

    /// For use with filling non-const methods
    typedef std::function<void(precision const &)> const_ref_iterator;

    /// For use with filling non-const methods
    typedef std::function<void(precision &)> ref_iterator;

    /// A method to iterator over all elements of the matrix by row and column with a const value
    void for_each(const_coord_iterator const_functor) const;

    /// A method to iterate over all elements of the matrix by row and column and a reference to the value
    matrix &for_each(ref_coord_iterator functor);

    /// A method to iterate over all elements of the matrix by value
    void for_each(const_ref_iterator functor) const;

    /// A method to iterate over all elements of the matrix by value
    matrix &for_each(ref_iterator functor);

    /// A method to iterate over all rows of the matrix by value
    matrix &for_each_row(std::function<void(size_t r)>);

    /// A method to iterate over all columns of the matrix by value
    matrix &for_each_col(std::function<void(size_t c)>);

    /// Condenses Row Eschelon Form (Gaussian elimination) into a single algorithm
    matrix &eschelon(size_t stop_col = std::numeric_limits<size_t>::max());
    matrix escheloned(size_t stop_col = std::numeric_limits<size_t>::max()) const;

    bool row_is_zero(size_t r, size_t sc = 0) const;
    bool col_is_zero(size_t c, size_t sr = 0) const;
    bool elem_is_zero(size_t r, size_t c) const;

    /// Reduced Row Eschelon Format
    /// \note Destructive! This will modify the current object and return a reference to itself for chaining.
    ///
    inline matrix &rref() {
        return eschelon().reduce();
    }

    /// Returns the rank of the matrix (the numer of linearly independent rows)
    /// \see https://en.wikipedia.org/wiki/Rank_(linear_algebra)
    /// \see http://mathworld.wolfram.com/MatrixRank.html
    ///
    size_t rank() const;

    /// Returns the nullspace matrix
    matrix nullspace() const noexcept(false);

    /// Returns the basis vectors of a matrix
    matrix basis() const noexcept(false);

    /// Print the matrix to stdout
    void print(std::ostream &, char const[]) const override;

    /// Save the values of this matrix to a file.
    bool to_file(std::string path) const;

    /// Produces a matrix from a file.
    static matrix from_file(std::string path);

    /// Returns the list of column indexes for each row of leading non-zeros.
    /// Values larger than cols is for zero rows.
    /// @param stop_col The column to stop the algorithm on. Defaults too all columns
    ///
    std::vector<size_t> leading_nonzero(size_t stop_col = std::numeric_limits<size_t>::max()) const;

    /// Returns the column indexes which are pivots
    /// \pre \ref rref()
    ///
    std::vector<size_t> pivots() const;

    /// Returns the column indexes which are free variables
    /// \pre \ref rref()
    ///
    std::vector<size_t> frees() const;

    /// Returns a bool mask of each column describing whether it is a pivot (true) or free (false)
    std::vector<bool> is_pivots() const;

    // FRIEND METHODS WHICH CAN SEE INTERNALLY

    /// exchange the data between two matrix
    friend void swap(matrix &a, matrix &b) noexcept(false);

    /// Debug Decomposition Functions which are protected
    friend void debug_decomposition(matrix &M);

    // ROW operations
protected:
    /// Swap two rows
    void row_swap(size_t row_a, size_t row_b);
    /// Scale a row by 'a'
    void row_scale(size_t row, precision a);
    /// Row addition
    /// \code
    /// A[dst][c] = A[dst][c] + (a * A[src][c]);
    /// \endcode
    ///
    void row_add(size_t row_dst, size_t row_src, precision a = 1.0_p);
    /// Row subtraction
    /// \code
    /// A[dst][c] = A[dst][c] - (a * A[src][c]);
    /// \endcode
    ///
    void row_sub(size_t row_dst, size_t row_src, precision a = 1.0_p);

    /// Compares row to a value
    bool row_compare(size_t row, precision a);

    /// Pushes zeroes rows to bottom of matrix
    matrix &sort_zero_rows();

    /// Sorts a specific column starting and ending at a specific row
    matrix &sort_leading_nonzero(size_t col, size_t i, size_t &j);
};

/// Add two matrix together
matrix addition(matrix const &a, matrix const &b) noexcept(false);

/// Subtracts b from a (a-b)
matrix subtraction(matrix const &a, matrix const &b) noexcept(false);

/// Multiplies to matrix together
matrix multiply(matrix const &a, matrix const &b) noexcept(false);

/// Multiplies matrix a by scalar r
matrix multiply(matrix const &a, precision const r) noexcept(false);

/// Multiplies matrix a by scalar r
matrix multiply(precision const r, matrix const &a) noexcept(false);

namespace operators {
inline matrix operator+(matrix const &a, matrix const &b) noexcept(false) {
    return addition(a, b);
}

inline matrix operator-(matrix const &a, matrix const &b) noexcept(false) {
    return subtraction(a, b);
}

inline matrix operator*(matrix const &a, matrix const &b) noexcept(false) {
    return multiply(a, b);
}

inline matrix operator*(matrix const &a, precision const r) noexcept(false) {
    return multiply(a, r);
}

inline matrix operator*(precision const r, matrix const &a) noexcept(false) {
    return multiply(a, r);
}

/// Divides a by b. This is equivalent to a*b^-1
inline matrix operator/(matrix const &a, matrix const &b) noexcept(false) {
    matrix binv = const_cast<matrix &>(b).inverse();
    return multiply(a, binv);
}

inline matrix operator/(matrix const &a, precision const r) noexcept(false) {
    return multiply(a, (1.0_p / r));
}

inline matrix operator/(precision const r, matrix const &a) noexcept(false) {
    return multiply(a, (1.0_p / r));
}

/// An easy mechanism to raise a matrix to a specific integer power
matrix operator^(matrix &a, int p) noexcept(false);

/// The shortcut version of a power operation for specialized symbols like T (transpose) or H (hermitian). Specialized
/// for conjoined operators */
matrix operator^(matrix &a, letters l) noexcept(false);

/// The shortcut version of a power operation for specialized symbols like T (transpose) or H (hermitian). Specialized
/// for conjoined operators */
matrix operator^(matrix const &a, letters l) noexcept(false);
}  // namespace operators

/// Joins the matricies horizontally, mxn and mxk to make a mx(n+k) matrix
matrix rowjoin(matrix &a, matrix &b) noexcept(false);

namespace operators {
/// Same as rowjoin
inline matrix operator|(matrix &a, matrix &b) noexcept(false) {
    return rowjoin(a, b);
}
}  // namespace operators

/// Joins the matricies vertically, mxn and kxn to make a (m+k)xn matrix.
matrix coljoin(matrix const &a, matrix const &b) noexcept(false);

namespace operators {
/// Same as coljoin
inline matrix operator||(matrix const &a, matrix const &b) noexcept(false) {
    return coljoin(a, b);
}
}  // namespace operators

namespace pairwise {
///
/// Takes a column matrix and a row matrix and makes a new matrix using these
/// rules.
/// A \op B = \[a, c, e\] \op \[ b| d| f\] = \[ a*b, c*b, e*b |
///                                             a*d, c*d, e*d |
///                                             a*f, c*f, e*f\]
/// \code
/// size_t A[1][3] = {{a,c,e}};
/// size_t B[3][1] = {{b},{d},{f}};
/// // output
/// size_t C[3][3] = {{a*b,c*b,e*b},{a*d,c*d,e*d},{a*f,c*f,e*f}};
/// \endcode
/// \note A must be a row matrix
/// \note B must be a col matrix
/// \note There might be a better name for this but I don't know it.
///
matrix multiply(matrix const &a, matrix const &b) noexcept(false);
}  // namespace pairwise

namespace rowwise {
///
/// Scales each row in the matrix a by the element in b (a column matrix).
/// \code
///  // inputs
/// size_t A[2][2] = {{a,b},{c,d}};
/// size_t B[2][1] = {{e},{f}};
///  // result
/// size_t C[2][2] = {{a*e,b*e},{c*f,d*f}};
/// \endcode
///
matrix scale(matrix const &a, matrix const &b) noexcept(false);
}  // namespace rowwise

/// The element-wise multiplication of a matrix
/// @note a and b must have the same dimensions
matrix hadamard(matrix const &a, matrix const &b) noexcept(false);

// INLINE SHORTCUTS

/// Returns the determinant of the matrix
inline precision determinant(matrix const &A) noexcept(false) {
    return A.determinant();
}

/// Returns the determinant of the const matrix
inline precision det(matrix const &A) noexcept(false) {
    return A.determinant();
}

/// Returns the adjugate of the matrix
inline matrix adj(matrix const &A) noexcept(false) {
    return A.adjugate();
}

/// Return the trace of the matrix.
inline precision tr(matrix const &A) {
    return A.trace();
}

/// Inverts the const matrix
inline matrix inv(matrix const &A) noexcept(false) {
    return A.inverse();
}

/// Returns the absolute value of the matrix
inline precision abs(matrix const &A) noexcept(false) {
    return A.determinant();
}

/// Computes the dot of two matrixes (not vectors)
inline precision dot(matrix const &u, matrix const &v) noexcept(false) {
    using namespace operators;
    return (v.T() * u).determinant();
}

/// Returns the nullspace of the matrix
inline matrix nullspace(matrix const &A) noexcept(false) {
    return A.nullspace();
}

/// Returns the kernel of a matrix
inline matrix kern(matrix const &A) noexcept(false) {
    return A.nullspace();
}

/// Prints the value of a matrix
std::ostream &operator<<(std::ostream &os, matrix const &m);

}  // namespace linalg
