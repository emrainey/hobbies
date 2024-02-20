#include <cstdint>

namespace basal {

char const *boolean(bool b) {
    static char const *values[2] = {"true", "false"};
    return (b ? values[0] : values[1]);
}

}  // namespace basal
