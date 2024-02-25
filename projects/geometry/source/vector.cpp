
#include "geometry/vector.hpp"

#include "geometry/types.hpp"

namespace geometry {

using namespace operators;

namespace R3 {

vector cross(vector const& a, vector const& b) noexcept(false) {
    precision x, y, z;
    x = (a[1] * b[2]) - (a[2] * b[1]);
    y = (a[2] * b[0]) - (a[0] * b[2]);
    z = (a[0] * b[1]) - (a[1] * b[0]);
    statistics::get().cross_products++;
    return vector{{x, y, z}};
}

bool parallel(vector const& a, vector const& b) {
    return (R3::null == cross(a, b));
}

vector rodrigues(vector const& k, vector const& v, iso::radians theta) {
    vector v_prj_k = dot(v, k) * k;
    vector v_rej_k = v - v_prj_k;
    vector w{cross(k, v)};
    vector v_ort_rot = cos(theta.value) * v_rej_k + sin(theta.value) * cross(k, v_rej_k);
    return v_ort_rot + v_prj_k;
}

precision triple(vector const& u, vector const& v, vector const& w) noexcept(false) {
    vector vw{cross(v, w)};
    return dot(u, vw);
}

}  // namespace R3

}  // namespace geometry
