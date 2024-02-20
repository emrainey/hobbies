#pragma once

#include <inttypes.h>

#include <iostream>
#include <string>

#ifndef PRIz
#if defined(__x86_64__)
#define PRIz "zu"
#endif
#endif

namespace basal {

/// A base class used to enforce a printing mechanism
class printable {
public:
    /// Character Array printing mechanism
    virtual void print(char const[]) const = 0;

    /// String printing mechanism.
    inline void print(std::string str) const {
        print(str.c_str());
    }

protected:
    /// Virtual deconstructor
    virtual ~printable() {
    }
};

}  // namespace basal
