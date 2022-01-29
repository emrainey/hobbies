
#include "geometry/types.hpp"
#include "geometry/vector.hpp"

namespace geometry {

using namespace operators;

namespace R3 {

vector cross(const vector& a, const vector& b) noexcept(false) {
    element_type x, y, z;
    x = (a[1] * b[2]) - (a[2] * b[1]);
    y = (a[2] * b[0]) - (a[0] * b[2]);
    z = (a[0] * b[1]) - (a[1] * b[0]);
    statistics::get().cross_products++;
    return vector{{x, y, z}};
}

bool parallel(const vector_<element_type, 3>& a, const vector_<element_type, 3>& b) {
    return (R3::null == cross(a, b));
}

vector rodrigues(const vector& k, const vector& v, iso::radians theta) {
    vector v_prj_k = dot(v, k) * k;
    vector v_rej_k = v - v_prj_k;
    vector w(cross(k,v));
    vector v_ort_rot = cos(theta.value) * v_rej_k + sin(theta.value) * cross(k, v_rej_k);
    return v_ort_rot + v_prj_k;
}

element_type triple(const vector& u, const vector& v, const vector& w) noexcept(false) {
    vector vw(cross(v, w));
    return dot(u, vw);
}

} // namespace R3

} // namespace geometry
