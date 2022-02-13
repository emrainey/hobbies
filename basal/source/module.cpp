
#include <dlfcn.h>
#include <functional>
#include "basal/basal.hpp"
#include "basal/module.hpp"

namespace basal {

module::module(const char name[])
    : handle(nullptr)
    {
    static const char * const formats[] = {
        "%s",
#if defined(__APPLE__)
        "lib%s.dylib",
#elif defined(__linux__)
        "lib%s.so",
#elif defined(__CYGWIN__)
        "cyg%s.dll"
#elif defined(_WIN64)
        "%s.dll",
#endif
    };
    for (size_t i = 0; i < dimof(formats) and handle == nullptr; i++) {
        char buffer[256];
        snprintf(buffer, dimof(buffer), formats[i], name);
        printf("Trying %s\n", buffer);
        handle = dlopen(buffer, RTLD_NOW);
        if (handle == nullptr) {
            printf("Load failed: %s\r\n", dlerror());
        } else {
            printf("Load passed for %s (error: %s)\r\n", buffer, dlerror());
        }
    }
    printf("Loaded into %p handle\r\n", handle);
}

module::module(module&& other) : handle(std::move(other.handle)) {}

module& module::operator=(module&& other) {
    handle = std::move(other.handle);
    return (*this);
}

module::~module() {
    dlclose(handle);
    handle = nullptr;
}


bool module::is_loaded() const {
    return not (handle == nullptr);
}

} // namespace module
