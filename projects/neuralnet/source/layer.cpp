
#include "neuralnet/layer.hpp"

#include <cstdlib>

namespace nn {

layer::layer() : layer_type{m_type}, values{}, beta{}, m_type(type::input) {
}

layer::layer(type _t, size_t num) : layer_type{m_type}, values(num, 1), beta(num, 1), m_type{_t} {
    values.zero();
    beta.zero();
}

layer::layer(const layer& other) : layer_type{m_type}, values(other.values), beta(other.beta), m_type(other.m_type) {
}

layer::layer(layer&& other)
    : layer_type{m_type}, values{std::move(other.values)}, beta{std::move(other.beta)}, m_type{other.m_type} {
}

layer& layer::operator=(const layer& other) {
    m_type = other.m_type;
    values = other.values;
    beta = other.beta;
    return (*this);
}

layer& layer::operator=(layer&& other) {
    m_type = other.m_type;
    values = std::move(other.values);
    beta = std::move(other.beta);
    return (*this);
}

layer::~layer() {
}

void layer::forward() {
}

void layer::forward(layer& other __attribute__((unused))) {
}

void layer::backward() {
}

void layer::backward(layer& other __attribute__((unused)), precision alpha __attribute__((unused)),
                     precision gamma __attribute__((unused))) {
}

bool layer::isa(type t) const {
    return (m_type == t);
}

size_t layer::get_num(void) const {
    return static_cast<size_t>(values.rows);
}

}  // namespace nn
