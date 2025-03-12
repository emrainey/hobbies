#pragma once

#include <array>
#include <cstdlib>  // exit
#include <exception>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

namespace basal {
/// Stack Tracking Exception
class exception : public std::exception {
private:
    std::array<void *, 25> stack;
    std::size_t size;
    char **stack_strings;
    std::string description;
    std::string location;
    std::size_t line_number;

public:
    /// String Constructor
    exception(std::string desc, std::string loc, size_t line);

    /// Destructor
    virtual ~exception();

    /// Returns the description
    char const *why() const;

    /// Returns a summary of the location of the exception
    char const *where() const;

    // STATIC METHODS

    /// Throws an exception if the condition is true
    static void throw_if(bool condition, char const loc[], size_t const line) noexcept(false);

    /// Throws an exception if the condition is true and remembers the formatted string as the reason.
    static void throw_if(bool condition, char const loc[], size_t const line, char const fmt[], ...) noexcept(false);

    /// Throws an exception if the condition is false
    static void throw_unless(bool condition, char const loc[], size_t const line) noexcept(false);

    /// Throws an exception if the condition is false and remembers the given string as the reason
    static void throw_unless(bool condition, char const loc[], size_t const len, char const fmt[], ...) noexcept(false);
};

void exit_if(bool condition, char const loc[], size_t const line, char const fmt[], ...) noexcept(true);

void exit_unless(bool condition, char const loc[], size_t const line, char const fmt[], ...) noexcept(true);

/// Simplifies catch the same exception within code blocks
void exit_if_thrown(char const statement[], std::function<void(void) noexcept(false)> block) noexcept(true);

/// Allows the execution of a block which must throw an exception
void exit_if_not_thrown(char const statement[], std::function<void(void) noexcept(false)> block) noexcept(true);
}  // namespace basal
