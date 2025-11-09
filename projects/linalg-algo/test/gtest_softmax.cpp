#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <basal/gtest_helper.hpp>

#include <linalg/softmax.hpp>

using namespace basal::literals;
using precision = basal::precision;

TEST(SoftMaxTest, Simple) {
    linalg::matrix m(3, 1);
    m[0][0] = 1.0_p;
    m[1][0] = 2.0_p;
    m[2][0] = 3.0_p;

    linalg::matrix result = linalg::softmax(m);

    // Expected softmax values
    precision sum_exp = std::exp(1.0_p) + std::exp(2.0_p) + std::exp(3.0_p);
    precision expected_0 = std::exp(1.0_p) / sum_exp;
    precision expected_1 = std::exp(2.0_p) / sum_exp;
    precision expected_2 = std::exp(3.0_p) / sum_exp;

    ASSERT_PRECISION_EQ(expected_0, result[0][0]);
    ASSERT_PRECISION_EQ(expected_1, result[1][0]);
    ASSERT_PRECISION_EQ(expected_2, result[2][0]);

    // the sum of all elements should be 1
    precision total = result[0][0] + result[1][0] + result[2][0];
    ASSERT_PRECISION_EQ(1.0_p, total);

    std::cout << "Input Matrix:\n" << m << std::endl;
    std::cout << "Softmax Result:\n" << result << std::endl;
}
