#include <cstdint>

namespace basal {

const char *boolean(bool b) {
    static const char *values[2] = { "true", "false" };
    return (b ? values[0] : values[1]);
}

} // namespace basal
