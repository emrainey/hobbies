#include "doorgame/animate.hpp"

#include <basal/basal.hpp>

namespace doorgame {

Animate::Animate(size_t location) : m_health{Health::Normal}, m_location{location} {
}

bool Animate::is_alive() const {
    return (get_health() != Health::Dead);
}

Health Animate::get_health() const {
    return m_health;
}

void Animate::location(size_t newloc) {
    m_location = newloc;
}

size_t Animate::location() const {
    return m_location;
}

void Animate::take(Damage amount) {
    size_t tmp_health = basal::to_underlying(m_health);
    if (basal::to_underlying(amount) > tmp_health) {
        m_health = Health::Dead;
    } else {
        m_health = static_cast<Health>(tmp_health - basal::to_underlying(amount));
    }
}

void Animate::resurrect() {
    m_health = Health::Hurt;
}

void Animate::give_healing(size_t amount) {
    size_t tmp_health = basal::to_underlying(m_health);
    if (amount > basal::to_underlying(Health::Normal)) {
        m_health = Health::Normal;
    } else {
        m_health = static_cast<Health>(tmp_health + amount);
    }
}

Animate const& Animate::as_animate() const {
    return *this;
}

Animate& Animate::as_animate() {
    return *this;
}

}  // namespace doorgame