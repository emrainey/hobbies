
#include "basal/exception.hpp"

#if defined(__linux__) || defined(__APPLE__)
#include <execinfo.h>
#endif
#include <cxxabi.h>
#include <unistd.h>

#include <csignal>
#include <cstdarg>
#include <cstdlib>
#include <cstring>

namespace basal {
exception::exception(std::string desc, std::string loc, std::size_t line)
    : std::exception{}, description{desc}, location{loc}, line_number{line} {
#if defined(__linux__) || defined(__APPLE__)
    if (support_backtrace) {
        size = static_cast<size_t>(backtrace(stack.data(), stack.size()));
        // print out all the frames to stderr
        stack_strings = backtrace_symbols(stack.data(), size);
        if (stack_strings) {
            description.append("\n");
            for (size_t s = 1; s < size; s++) {
                size_t function_name_size = 256;
                if constexpr (debug::exceptions) {
                    printf("%s", stack_strings[s]);
                }
                char *funcname = static_cast<char *>(malloc(function_name_size));
                if (funcname) {
                    char *depth = std::strtok(stack_strings[s], " \t");
                    char *module = std::strtok(NULL, " \t");
                    char *pc = std::strtok(NULL, " \t");
                    char *begin_name = std::strtok(NULL, " \t");
                    char *sign = std::strtok(NULL, " \t");
                    char *begin_offset = std::strtok(NULL, " \t");

                    // mangled name is now in [begin_name, begin_offset) and caller
                    // offset in [begin_offset, end_offset). now apply
                    // __cxa_demangle():
                    depth[0] |= 0;
                    module[0] |= 0;
                    pc[0] |= 0;
                    sign[0] |= 0;
                    begin_offset[0] |= 0;

                    int status;
                    char *ret = abi::__cxa_demangle(begin_name, funcname, &function_name_size, &status);
                    if (status == 0) {
                        description.append("\t");
                        description.append(ret);
                        description.append(" (demangled)\n");
                    } else {
                        description.append("\t");
                        description.append(begin_name);
                        description.append(" (normal)\n");
                    }
                    free(funcname);
                    funcname = nullptr;
                }
            }
            free(stack_strings);
        }
    } else {
        description.append("Backtraces disabled\n");
    }
#else
    description.append("Backtraces not supported\n");
#endif
}

exception::~exception() {
}

char const *exception::why() const {
    return description.c_str();
}

char const *exception::where() const {
    static char buffer[256 + 20] = {0};
    snprintf(buffer, sizeof(buffer), "%s:%zu", location.c_str(), line_number);
    return buffer;
}

void exception::throw_if(bool condition, char const loc[], std::size_t line, char const fmt[], ...) noexcept(false) {
    if (condition) {
        char const *value = std::getenv("ABORT");
        if (value) {
            fprintf(stdout, "ABORT=%s\n", value);
            std::string state{value};
            if (state == "1" || state == "true") {
                std::abort();  // lldb or gdb should catch
            }
        } else {
            fprintf(stdout, "Use ABORT=1 in env to cause an std::abort\n");
        }
        char buffer[1024];
        va_list list;
        va_start(list, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, list);
        va_end(list);
        fprintf(stdout, "%s", buffer);
        throw basal::exception(buffer, loc, line);
    }
}

void exception::throw_if(bool condition, char const loc[], std::size_t line) noexcept(false) {
    if (condition) {
        char const *value = std::getenv("ABORT");
        if (value) {
            fprintf(stdout, "ABORT=%s\n", value);
            std::string state{value};
            if (state == "1" || state == "true")
                std::abort();  // lldb or gdb should catch
        } else {
            fprintf(stdout, "Use ABORT=1 in env to cause an std::abort\n");
        }
        throw basal::exception("basal::exception", loc, line);
    }
}

void exception::throw_unless(bool condition, char const loc[], std::size_t line, char const fmt[],
                             ...) noexcept(false) {
    if (not condition) {
        char buffer[1024];
        va_list list;
        va_start(list, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, list);
        va_end(list);
        fprintf(stdout, "%s", buffer);
        char const *value = std::getenv("ABORT");
        if (value) {
            fprintf(stdout, "ABORT=%s\n", value);
            std::string state{value};
            if (state == "1" || state == "true") {
                std::abort();  // lldb or gdb should catch
            }
        } else {
            fprintf(stdout, "Use ABORT=1 in env to cause an std::abort\n");
        }
        throw basal::exception(buffer, loc, line);
    }
}

void exception::throw_unless(bool condition, char const loc[], std::size_t line) noexcept(false) {
    if (not condition) {
        char const *value = std::getenv("ABORT");
        if (value) {
            fprintf(stdout, "ABORT=%s\n", value);
            std::string state{value};
            if (state == "1" || state == "true")
                std::abort();  // lldb or gdb should catch
        } else {
            fprintf(stdout, "Use ABORT=1 in env to cause an std::abort\n");
        }
        throw basal::exception("basal::exception", loc, line);
    }
}

/// Simplifies catch the same exception within code blocks
void exit_if_thrown(char const statement[], std::function<void(void) noexcept(false)> block) noexcept {
    try {
        block();
        fprintf(stdout, "%s did not throw! Success!\n", statement);
    } catch (basal::exception const &me) {
        fprintf(stderr,
                "ERROR: Unexpected math exception!\n"
                "Why: %s\nWhere: %s\n",
                me.why(), me.where());
        exit(-1);
    } catch (...) {
        fprintf(stderr, "ERROR: Unexpected unknown exception!\n");
        exit(-1);
    }
}

/// Allows the execution of a block which must throw an exception
void exit_if_not_thrown(char const statement[], std::function<void(void) noexcept(false)> block) noexcept {
    try {
        block();
        fprintf(stderr, "ERROR: %s did not throw an exception\n", statement);
        exit(-1);
    } catch (basal::exception const &me) {
        fprintf(stdout, "%s threw Exception! Success: %s\nWhere: %s\n", statement, me.why(), me.where());
    } catch (...) {
        fprintf(stdout, "%s threw unknown exception! Success?\n", statement);
    }
}

void exit_if(bool condition, char const loc[], size_t const line, char const fmt[], ...) noexcept {
    if (condition) {
        fprintf(stdout, "ERROR: Condition was true!\nWhere: %s:%zu\n", loc, line);
        char buffer[1024];
        va_list list;
        va_start(list, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, list);
        va_end(list);
        exit(-1);
    }
}

void exit_unless(bool condition, char const loc[], size_t const line, char const fmt[], ...) noexcept {
    if (not condition) {
        fprintf(stdout, "ERROR: Condition was false!\nWhere: %s:%zu\n", loc, line);
        char buffer[1024];
        va_list list;
        va_start(list, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, list);
        va_end(list);
        exit(-1);
    }
}

}  // namespace basal
