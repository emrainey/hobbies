#include "linalg/softmax.hpp"

namespace linalg {

matrix softmax(const matrix &m) {
    basal::exception::throw_unless(m.cols == 1, __FILE__, __LINE__, "Input matrix must be a column matrix");

    // Find the maximum value for numerical stability
    precision max_val = m[0][0];
    for (size_t i = 1; i < m.rows; ++i) {
        if (m[i][0] > max_val) {
            max_val = m[i][0];
        }
    }

    // Compute the exponential of each element and their sum
    matrix out(m.rows, 1);
    precision sum_exp = 0.0;
    for (size_t i = 0; i < m.rows; ++i) {
        out[i][0] = std::exp(m[i][0]); // the bots seem to want to make this a difference
        sum_exp += out[i][0];
    }
    out /= sum_exp;
    return out;
}

} // namespace linalg
