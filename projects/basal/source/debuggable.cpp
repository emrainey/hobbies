#include <basal/debuggable.hpp>

namespace basal {

void debuggable::emit(char const fmt[], ...) const {
    char buffer[local_stack_for_print];
    va_list list;
    va_start(list, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, list);
    printf("%s", buffer);
    va_end(list);
}

}  // namespace basal
