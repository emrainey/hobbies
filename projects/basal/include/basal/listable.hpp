
#pragma once
#include <cassert>
#include <utility>
#include <type_traits>
namespace basal {

template <typename UNIT_TYPE>
class listable : public UNIT_TYPE {
    static_assert(std::is_default_constructible<UNIT_TYPE>::value, "Must be default constructible");
protected:
    listable *_next;
    listable *_prev;

public:
    /// default ctor
    listable() noexcept : UNIT_TYPE{}, _next{nullptr}, _prev{nullptr} {} // must be default constructable
    /// subclass copy
    listable(UNIT_TYPE const& sub) : UNIT_TYPE{sub}, _next{nullptr}, _prev{nullptr} {} // must be copy constructable
    /// copy ctor (not supported)
    listable(listable const&) noexcept = delete;
    /// move constructor (replace the other in the list)
    listable(listable &&other) noexcept : UNIT_TYPE{std::move(other)}, _next{other._next}, _prev{other._prev} {
        if (_next) {
            _next->_prev = this;
            other._next = nullptr;
        }
        if (_prev) {
            _prev->_next = this;
            other._prev = nullptr;
        }
    }
    /// copy assign (not supported)
    listable &operator=(listable const& other) noexcept = delete;
    /// move assign, we replace to other item in the list
    listable &operator=(listable &&other) noexcept {
        if (this != &other) {
            UNIT_TYPE::operator=(std::move(other));
            // remove ourselves first
            if (_next) {
                _next->_prev = _prev;
            }
            if (_prev) {
                _prev->_next = _next;
            }
            _next = other._next;
            _prev = other._prev;

            // take over the other spot
            if (other._next) {
                other._next->_prev = this;
                other._next = nullptr;
            }
            if (other._prev) {
                other._prev->_next = this;
                other._prev = nullptr;
            }
        }
        return (*this);
    }
    /// Remove ourselves from the list if we are in one.
    virtual ~listable() noexcept {
        if (_next) {
            _next->_prev = _prev;
        }
        if (_prev) {
            _prev->_next = _next;
        }
        _next = nullptr;
        _prev = nullptr;
    }
    // swap the object into the spot where "this" is.
    void insert_swap(listable *obj) noexcept {
        if (obj) {
            if (obj->_next) {
                _next = obj->_next;
                obj->_next->_prev = this;
                obj->_next = nullptr;
            }
            if (obj->_prev) {
                _prev = obj->_prev;
                obj->_prev->_next = this;
                obj->_prev = nullptr;
            }
        }
    }
    void insert_prev(listable *obj) noexcept {
        if (obj) {
            // no nullptr deref!
            obj->_prev = _prev;
            if (_prev) {
                assert(_prev->_next == this);
                _prev->_next = obj;
            }
            _prev = obj;
            obj->_next = this;
        }
    }

    void insert_next(listable *obj) noexcept  {
        if (obj) {
            obj->_next = _next;
            if (_next) {
                assert(_next->_prev == this);
                _next->_prev = obj;
            }
            _next = obj;
            obj->_prev = this;
        }
    }

    /// Returns the next listable
    listable const* next() const noexcept {
        return _next;
    }

    /// Returns previous listable
    listable const* prev() const noexcept {
        return _prev;
    }

    /// Returns a listable pointer to itself.
    listable const* as_listable() const noexcept {
        return this;
    }

    UNIT_TYPE const& as_base() const noexcept {
        return (*this);
    }
};

}  // namespace basal
