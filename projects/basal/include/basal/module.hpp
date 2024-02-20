#pragma once

#include <dlfcn.h>

#include <functional>

namespace basal {

/// Wraps loading and unloading a module along with "C" function lookups
class module final {
public:
    module() = delete;  // No default

    /// Must give a name of a module to open
    explicit module(char const name[]);

    // No copies (looses track of counts)
    module(module const& other) = delete;
    module& operator=(module const& other) = delete;

    /// Move Constructor
    module(module&& other);

    /// Move Assignment
    module& operator=(module&& other);

    /// Determines if the module loaded successfully
    bool is_loaded() const;

    /// Destructor
    ~module();

    /// Method retrieve a "C" linkage compatible method from the module
    template <class function_type>
    std::function<function_type> get_symbol(char const symbol_name[]) {
        static_assert(std::is_function<function_type>::value, "Must be a function type");
        function_type* fp = reinterpret_cast<function_type*>(dlsym(handle, symbol_name));
        printf("Handle %p Function %s returned %p\r\n", handle, symbol_name, fp);
        return std::function<function_type>(fp);
    }

protected:
    void* handle;
};

}  // namespace basal