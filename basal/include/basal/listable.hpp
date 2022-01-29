
#pragma once
#include <cassert>
namespace basal {
class listable {
protected:
    listable *_next;
    listable *_prev;
public:
    /// default ctor
    listable() noexcept;
    /// copy ctor (not supported)
    listable(const listable *) noexcept = delete;
    /// move constructor (replace the other in the list)
    listable(listable&& other) noexcept;
    /// copy assign (not supported)
    listable &operator=(const listable &other) noexcept = delete;
    /// move assign, we replace to other item in the list
    listable &operator=(listable&& other) noexcept;
    /// Remvoe ourselves from the list if we are in one.
    virtual ~listable() noexcept;
    // swap the object into the spot where "this" is.
    void insert_swap(listable *obj) noexcept;
    void insert_prev(listable *obj) noexcept;
    void insert_next(listable *obj) noexcept;
};

}
