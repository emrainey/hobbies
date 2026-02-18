#include <fourcc/types.hpp>
#include <fourcc/encoding.hpp>

namespace fourcc {
namespace gamma {
precision interpolate(precision const a, precision const b, precision const s) {
    if (gamma::enforce_contract) {
        basal::exception::throw_unless(0.0_p <= a and a <= 1.0_p, __FILE__, __LINE__, "Channel is incorrect");
        basal::exception::throw_unless(0.0_p <= b and b <= 1.0_p, __FILE__, __LINE__, "Channel is incorrect");
        basal::exception::throw_unless(0.0_p <= s and s <= 1.0_p, __FILE__, __LINE__,
                                       "Interpolation Scalar is incorrect");
    }
    constexpr static precision const constants[] = {0.0_p, 1.0_p};
    // linear to log
    precision ag = gamma::remove_correction(a);
    precision bg = gamma::remove_correction(b);
    // weighted equation
    precision cg = (ag * s) + (bg * (constants[1] - s));
    // log to linear
    return gamma::apply_correction(cg);
}
}  // namespace gamma
}  // namespace fourcc