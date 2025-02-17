#include "raytrace/animator.hpp"

namespace raytrace {
namespace animation {
Animator::Animator(precision frame_rate, anchors const& a) : delta_(0.0_p), anchors_{a} {
    if (frame_rate > 0.0_p) {
        delta_ = iso::seconds(1.0_p / frame_rate);
    }
    // using namespace iso::operators;
    std::cout << "Animator created with " << frame_rate << " fps which is " << delta_.value << " seconds" << std::endl;
}

Animator::operator bool() const {
    return index_ < anchors_.size();
}

Attributes Animator::operator()() {
    using namespace iso::operators; // exposes operators for iso types
    if (index_ >= anchors_.size()) {
        return Attributes{};
    }
    auto dt = (now_ - start_) / anchors_[index_].duration;
    std::cout << "Animator: " << index_ << " at " << now_.value << " with dt=" << dt << std::endl;
    auto a = interpolate(anchors_[index_].start, anchors_[index_].limit, anchors_[index_].mappers, dt);
    now_ += delta_;
    if (now_ >= (start_ + anchors_[index_].duration)) {
        start_ += anchors_[index_].duration;
        index_++;
    }
    return a;
}

Attributes Animator::interpolate(Attributes const& start, Attributes const& limit, Mappers const& mappers, precision dt) {
    return Attributes{
        geometry::lerp(start.from, limit.from, mappers.from, dt),
        geometry::lerp(start.at, limit.at, mappers.at, dt),
        iso::degrees{geometry::lerp(start.fov.value, limit.fov.value, mappers.fov, dt)}
    };
}

} // namespace animation

}  // namespace raytrace
