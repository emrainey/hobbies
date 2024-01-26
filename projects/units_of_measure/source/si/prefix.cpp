
#include "si/prefix.hpp"

#include <cmath>

namespace SI {

namespace operators {
iso::precision operator*(iso::precision const a, SI::prefix p) {
    return a * pow(10.0, static_cast<iso::precision>(p));
}

}  // namespace operators

}  // namespace SI
