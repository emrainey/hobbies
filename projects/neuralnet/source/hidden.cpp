
#include "neuralnet/hidden.hpp"

namespace nn {

hidden::hidden(size_t prev, size_t num) : inner(layer::type::hidden, prev, num) {
}

hidden::~hidden() {
}

}  // namespace nn
