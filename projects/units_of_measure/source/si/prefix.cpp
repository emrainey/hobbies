
#include "si/prefix.hpp"

#include <cmath>

namespace SI {

namespace operators {
iso::precision operator*(iso::precision const a, SI::prefix p) {
    double const base{10.0};
    return a * pow(base, static_cast<iso::precision>(p));
}

}  // namespace operators

}  // namespace SI
