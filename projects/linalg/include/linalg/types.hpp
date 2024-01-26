#pragma once
/**
 * @file
 * Definitions for smaller types within the linalg space
 * @copyright Copyright 2019 (C) Erik Rainey.
 */

#include <limits>
//#include <iso/iso.hpp>
#include <basal/basal.hpp>

namespace linalg {

/** These are specialized symbols used to notate math ops or commonly used values in a simple way */
typedef enum letters_e : short
{
    T = std::numeric_limits<short>::min(),  //!< The Transpose operation
    H,                                      //< The Hermitian transpose

    // for use with the PLU tuple
    P = 0,  //!< When using \ref matrix::PLU, provide this to get the P value
    L = 1,  //!< When using \ref matrix::PLU, provide this to get the L value
    U = 2,  //!< When using \ref matrix::PLU, provide this to get the U value
} letters;

/** All the linalg element types are using basal's types */
using precision = basal::precision;

/** Collects the statistics from the linalg library */
struct statistics {
public:
    /** The number of quadratic roots */
    size_t quadratic_roots;
    /** The number of cubic roots */
    size_t cubic_roots;
    /** The number of quartic roots */
    size_t quartic_roots;
    /** The number of matrix multiplies */
    size_t matrix_multiply;
    /** The number of determinants */
    size_t matrix_determinants;

    static statistics& get() {
        static statistics s;
        return s;
    }

protected:
    constexpr statistics()
        : quadratic_roots{0}, cubic_roots{0}, quartic_roots{0}, matrix_multiply{0}, matrix_determinants{0} {
    }
};
}  // namespace linalg
