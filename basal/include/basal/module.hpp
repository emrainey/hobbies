#pragma once

#include <dlfcn.h>
#include <functional>

namespace basal {

/** Wraps loading and unloading a module along with "C" function lookups */
class module final {
public:
    module() = delete; // No default

    /** Must give a name of a module to open */
    explicit module(const char name[]) {
        handle = dlopen(name, RTLD_NOW);
        if (handle == nullptr) {
            printf("Load failed: %s\r\n", dlerror());
        }
        dlerror(); // several guides use this to "Reset" the error message
    }
    // No copies (looses track of counts)
    module(const module& other) = delete;
    module& operator=(const module& other) = delete;

    /** Move Constructor */
    module(module&& other) : handle(std::move(other.handle)) {}

    /** Move Assignment */
    module& operator=(module&& other) {
        handle = std::move(other.handle);
        return (*this);
    }

    /** Determines if the module loaded successfully */
    bool is_loaded() const {
        return not (handle == nullptr);
    }

    /* Destructor */
    ~module() {
        dlclose(handle);
        handle = nullptr;
    }

    /** Method retrieve a "C" linkage compatible method from the module */
    template <class function_type>
    std::function<function_type> get_symbol(const char symbol_name[]) {
        static_assert(std::is_function<function_type>::value, "Must be a function type");
        function_type *fp = (function_type*)dlsym(handle, symbol_name);
        printf("Handle %p Function %s returned %p\r\n", handle, symbol_name, fp);
        return std::function<function_type>(fp);
    }
protected:
    void *handle;
};

}