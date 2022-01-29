#include <cassert>
#include <basal/listable.hpp>

namespace basal {

listable::listable() noexcept : _next(nullptr), _prev(nullptr) {}

listable::listable(listable&& other) noexcept
    : _next(other._next)
    , _prev(other._prev)
    {
    if (_next) {
        _next->_prev = this;
        other._next = nullptr;
    }
    if (_prev) {
        _prev->_next = this;
        other._prev = nullptr;
    }
}

listable::~listable() noexcept {
    if (_next) {
        _next->_prev = _prev;
    }
    if (_prev) {
        _prev->_next = _next;
    }
    _next = nullptr;
    _prev = nullptr;
}

listable& listable::operator=(listable&& other) noexcept {
    if (this != &other) {
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

void listable::insert_prev(listable *obj) noexcept {
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

void listable::insert_next(listable *obj) noexcept {
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

};
