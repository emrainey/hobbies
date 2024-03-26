
#include "neuralnet/input.hpp"

namespace nn {

input::input(size_t num) : layer(layer::type::input, num) {
}

input::~input() {
}

void input::encode(mnist& db, size_t index) {
    mnist::image& img = db.get_image(index);
    values.for_each([&](size_t r, size_t c, precision& v) {
        (void)c;
        size_t ir = r / mnist::dim;
        size_t ic = r % mnist::dim;
        v = precision(img.data[ir][ic]) / 255.0_p;
    });
}

}  // namespace nn
