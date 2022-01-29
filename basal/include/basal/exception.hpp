#pragma once

#include <cstdlib> // exit
#include <string>
#include <exception>
#include <functional>
#include <array>
#include <memory>
#include <stdexcept>

namespace basal {
    /*! Stack Tracking Exception */
    class exception : public std::exception {
    private:
        std::array<void *, 25> stack;
        std::size_t size;
        char **stack_strings;
        std::string description;
        std::string location;
        std::size_t line_number;
    public:
        /*! String Constructor */
        exception(std::string desc, std::string loc, size_t line);

        /*! Destructor */
        virtual ~exception();

        /*! Returns the description */
        const char *why() const;

        /*! Returns a summary of the location of the exception */
        const char *where() const;

        // STATIC METHODS

        /*! Throws an exception if the condition is true */
        static void throw_if(bool condition, const char loc[], const size_t line) noexcept(false);

        /*! Throws an exception if the condition is true and remembers the formatted string as the reason. */
        static void throw_if(bool condition, const char loc[], const size_t line, const char fmt[], ...) noexcept(false);

        /*! Throws an exception if the condition is false */
        static void throw_unless(bool condition, const char loc[], const size_t line) noexcept(false);

        /*! Throws an exception if the condition is false and remembers the given string as the reason */
        static void throw_unless(bool condition, const char loc[], const size_t len, const char fmt[], ...) noexcept(false);
    };

    /*! Simplifies catch the same exception within code blocks */
    void assert_if_thrown(const char statement[], std::function<void(void) noexcept(false)> block) noexcept(true);

/*! Wraps a try {} catch{} block around a statement. Asserts if an exception is thrown. */
#define try_catch(statement) basal::assert_if_thrown(#statement, [&](void) -> void { statement; })

    /*! Allows the execution of a block which must throw an exception */
    void assert_if_not_thrown(const char statement[], std::function<void(void) noexcept(false)> block) noexcept(true);

/*! The wrapped statement must produce an exception or an assert will fire */
#define catch_exception(statement) basal::assert_if_not_thrown(#statement,[&](void) -> void { statement; })

/*! Macro to wrap static method with filename and line information */
#define throw_exception_if(condition, fmt, ...)     basal::exception::throw_if(condition, __FILE__, __LINE__, fmt, __VA_ARGS__)

/*! Macro to wrap static method with filename and line information */
#define throw_exception_unless(condition, fmt, ...) basal::exception::throw_unless(condition, __FILE__, __LINE__, fmt, __VA_ARGS__)
}
