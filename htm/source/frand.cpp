#include "htm/htm.hpp"

namespace htm {

float frand() {
    int r = rand();
    float f = ((float)r) / (float)RAND_MAX;
    return f;
}

}