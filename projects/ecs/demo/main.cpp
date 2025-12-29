// g++ -O3 -g -std=c++20 -Wall -Werror -Wextra -pedantic -o ecs ecs.cpp -I/opt/homebrew/include/SDL -L/opt/homebrew/lib
// -lsdl && ./ecs
#include <cstdint>
#include <cstddef>
#include <memory>
#include <limits>
#include <thread>
#include <type_traits>
#include <iostream>
#include <stdexcept>
#include <iomanip>
#include <random>

#include <SDL.h>

/// These will be used as bits in a bitfield for a compact representation
/// of the runtime type association for a Entity
enum class Types : uint8_t {
    Typing = 0,
    Nameable = 1,
    Moveable = 2,
    Drawable = 3,
    _max
};

using Index = std::size_t;
using Count = std::size_t;

template <typename ENUM>
class Typeable {
public:
    using Type = ENUM;
    static_assert(std::is_enum<ENUM>::value, "Must be an enum");
    constexpr Typeable(Type t) : type{t} {
    }
    Type type;
};

/// A component manager, uses indexes into the arrays for O(1) lookup.
template <typename TYPE, Count COUNT>
class Manager {
public:
    constexpr static Count Capacity{COUNT};
    constexpr static Index Invalid{std::numeric_limits<Index>::max()};
    using Type = TYPE;
    Manager() : next_available_{0U}, items_{}, valid_{} {
    }  // must put all elements into a known state

    [[nodiscard]] Index Acquire() noexcept {
        if (next_available_ < items_.size()) {
            // inplace construct entity
            Type* tmp = new (&items_[next_available_]) Type();
            valid_[next_available_] = (tmp != nullptr);
            // forget tmp, use index
            Index i = next_available_;
            find_next_availble();
            return i;
        }
        // nothing is available.
        return Invalid;
    }

    [[nodiscard]] Index Acquire(Index index) {
        if (index < items_.size()) {
            // inplace construct entity
            Type* tmp = new (&items_[index]) Type();
            valid_[index] = (tmp != nullptr);
            if (next_available_ == index) {
                find_next_availble();
            }
            return index;
        }
        return Invalid;
    }

    void Release(Index index) noexcept {
        if (index < Capacity) {
            valid_[index] = false;
            // could use this as the next_available_, but could also search
        }
        // do nothing
    }

    [[nodiscard]] Count Available() const noexcept {
        Count total = 0U;
        for (auto v : valid_) {
            total += (v ? 1 : 0U);
        }
        return total;
    }

    [[nodiscard]] Type& operator[](Index index) {
        if (index < Capacity) {
            return items_[index];
        }
        throw std::out_of_range("Index is too large");
    }

    [[nodiscard]] bool IsValid(Index index) const noexcept {
        if (index < Capacity) {
            return valid_[index];
        }
        return false;
    }

protected:
    void find_next_availble() noexcept {
        Index end = next_available_;
        for (Index start = next_available_ + 1; start != end; start++) {
            if (start == Capacity) {
                start = 0;
            }
            if (not valid_[start]) {
                next_available_ = start;
                return;
            }
        }
        // foudn nothing
        next_available_ = Capacity;
        return;
    }

    Count next_available_;
    std::array<Type, Capacity> items_;
    std::array<bool, Capacity> valid_;
};

// Basic structures which have no typing and should be stupid simple

struct Point {
    double x;
    double y;
    double z;
};

struct Vector {
    double dx;
    double dy;
    double dz;
};

constexpr Point operator+(Point p0, Vector v) {
    Point p1{0.0, 0.0, 0.0};
    p1.x = p0.x + v.dx;
    p1.y = p0.y + v.dy;
    p1.z = p0.z + v.dz;
    return p1;
}

constexpr Point operator+(Vector v, Point p0) {
    return p0 + v;
}

constexpr Vector operator-(Point p1, Point p0) {
    return Vector{p1.x - p0.x, p1.y - p0.y, p1.z - p0.z};
}

constexpr Vector operator+(Vector a, Vector b) {
    return Vector{a.dx + b.dx, a.dy + b.dy, a.dz + b.dz};
}

constexpr Vector operator-(Vector a, Vector b) {
    return Vector{a.dx - b.dx, a.dy - b.dy, a.dz - b.dz};
}

constexpr Vector operator*(Vector a, double n) {
    return Vector{a.dx * n, a.dy * n, a.dz * n};
}

constexpr Vector operator/(Vector a, double n) {
    return Vector{a.dx / n, a.dy / n, a.dz / n};
}

constexpr Vector operator-(Vector a) {
    return a * -1.0;
}

constexpr double dot(Vector a, Vector b) {
    return (a.dx * b.dx) + (a.dy * b.dy) + (a.dz * b.dz);
}

constexpr Vector cross(Vector a, Vector b) {
    double x = (a.dy * b.dz) - (a.dz * b.dy);
    double y = (a.dz * b.dx) - (a.dx * b.dz);
    double z = (a.dx * b.dy) - (a.dy * b.dx);
    return Vector{x, y, z};
}

constexpr double quadrance(Vector a) {
    return dot(a, a);
}

constexpr double magnitude(Vector a) {
    return sqrt(quadrance(a));
}

constexpr Vector normalized(Vector a) {
    double n = magnitude(a);
    return a / n;
}

constexpr static const double G = 6.6E-5;  // really 6.674E-11;

constexpr double gravity(double mass0, double mass1, double distance) {
    if (distance > 0.0) {
        return (mass0 * mass1 * G) / (distance * distance);
    }
    return 0.0;
}

constexpr Point origin{0.0, 0.0, 0.0};
constexpr Vector zero{0.0, 0.0, 0.0};
namespace basis {
// constexpr Vector X{1.0, 0.0, 0.0};
// constexpr Vector Y{0.0, 1.0, 0.0};
constexpr Vector Z{0.0, 0.0, 1.0};
}  // namespace basis

/// Components (must have a type field)

/// This holds the
struct Typing : Typeable<Types> {
    Typing() : Typeable<Types>{Types::Typing} {
    }
    constexpr static Count NumTypes{static_cast<Count>(Types::_max)};
    std::bitset<NumTypes> types;
};

struct Nameable : Typeable<Types> {
    Nameable() : Typeable<Types>{Types::Nameable} {
    }
    constexpr static Count limit{23 + 1};
    char name[limit];

    void clear() {
        memset(name, 0, limit);
    }
};

struct Moveable : Typeable<Types> {
    Moveable() : Typeable<Types>{Types::Moveable} {
    }
    Point position;
    Vector velocity;
    Vector acceleration;
    // Vector jerk;
    // Vector snap;
    // Vector crackle;
    // Vector pop;
    double mass;

    void clear() {
        position.x = position.y = position.z = 0.0;
        velocity.dx = velocity.dy = velocity.dz = 0.0;
        acceleration.dx = acceleration.dy = acceleration.dz = 0.0;
    }
};

struct Drawable : Typeable<Types> {
    Drawable() : Typeable<Types>{Types::Drawable} {
    }
    int diameter;
    uint8_t color[4];  // RGBA
};

class Context {
public:
    constexpr static Count NumEntities{2048U};
    constexpr static Count Fraction{32};
    constexpr static double CentralMass{512.0};
    constexpr static double UnitMass{1.0 / 512.0};
    constexpr static int width = 1024;
    constexpr static int height = 1024;

    Context() : attractor{} {
        attractor.mass = CentralMass;
        attractor.position = origin;
    }

    void List() {
        for (Index i = 0U; i < nameables_manager.Capacity; i++) {
            if (nameables_manager.IsValid(i)) {
                Nameable& n = nameables_manager[i];
                std::cout << "[" << i << "]Name: " << n.name << std::endl;
            }
        }
    }

    void Mover(double const dt) {
        for (Index i = 0U; i < moveables_manager.Capacity; i++) {
            if (moveables_manager.IsValid(i)) {
                Moveable& m = moveables_manager[i];
                // update position
                m.position.x = m.position.x + (m.velocity.dx * dt) + (0.5 * m.acceleration.dx * dt * dt);
                m.position.y = m.position.y + (m.velocity.dy * dt) + (0.5 * m.acceleration.dy * dt * dt);
                m.position.z = m.position.z + (m.velocity.dz * dt) + (0.5 * m.acceleration.dz * dt * dt);
                // update velocity
                m.velocity.dx = m.velocity.dx + m.acceleration.dx * dt;
                m.velocity.dy = m.velocity.dy + m.acceleration.dy * dt;
                m.velocity.dz = m.velocity.dz + m.acceleration.dz * dt;

                // update acceleation as gravity to the attractor
                Vector difference = attractor.position - m.position;
                double distance = magnitude(difference);
                double g = gravity(m.mass, attractor.mass, distance);
                m.acceleration = (difference / distance) * g;
            }
        }
    }

    void Draw() {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        int hh = height / 2;
        int hw = width / 2;
        for (Index i = 0U; i < drawables_manager.Capacity; i++) {
            if (drawables_manager.IsValid(i)) {
                Drawable& d = drawables_manager[i];
                Moveable& m = moveables_manager[i];
                SDL_SetRenderDrawColor(renderer, d.color[0], d.color[1], d.color[2], d.color[3]);
                int x = hw + floor(m.position.x / 10 * hw);
                int y = hh + floor(m.position.y / 10 * hh);
                SDL_Rect const r{x - d.diameter / 2, y - d.diameter / 2, d.diameter, d.diameter};
                //                SDL_RenderDrawPoint(renderer, x, y);
                SDL_RenderFillRect(renderer, &r);
            }
        }
        SDL_RenderPresent(renderer);
    }

    void Cycle(std::chrono::microseconds until) {
        std::chrono::microseconds delta{1'000'000U / Fraction};
        std::chrono::microseconds error{0u};
        double const dt = 1.0 / double(Fraction);
        double now = 0.0;
        //        union {
        //            double f;
        //            uint64_t v;
        //        } u;
        SDL_Init(SDL_INIT_VIDEO);
        window = SDL_CreateWindow("ECS Particles", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
                                  SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        SDL_RenderClear(renderer);
        SDL_Rect fillRect = {0, 0, width, height};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &fillRect);
        SDL_RenderPresent(renderer);
        SDL_Event event;
        do {
            auto start_time = std::chrono::steady_clock::now();
            Mover(dt);
            Draw();
            // since it's a power of 2 it will minimize float error,
            // but it will not eliminate it! As it gets bigger, it will lose
            // precision at some point.
            now += dt;

            //            u.f = now;
            //            std::bitset<64> b{u.v};
            //            std::cout << "Now: " << std::fixed << std::setprecision(7) << now << " =0b" << b << std::endl;

            // NOTHING AFTER HERE BUT TIMING
            auto end_time = std::chrono::steady_clock::now();
            auto diff_time = end_time - start_time;
            auto diff_us = std::chrono::duration_cast<std::chrono::microseconds>(diff_time);
            start_time = end_time;
            auto increment = delta - diff_us;

            if (delta > diff_us) {
                std::this_thread::sleep_for(increment);
            } else {
                error += increment;
                std::cout << "Diff was " << diff_us.count() << ", delta=" << delta.count() << " error=" << error.count()
                          << std::endl;
                // throw std::runtime_error("Timing Violation!");
            }

            if (SDL_PollEvent(&event) and event.type == SDL_QUIT) {
                break;
            }

        } while (now < until.count());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    bool Add(Index i, char const* name, Point p) {
        Index t = typeables_manager.Acquire(i);
        Index n = nameables_manager.Acquire(i);
        Index m = moveables_manager.Acquire(i);
        Index d = drawables_manager.Acquire(i);
        if (i == t and i == n and i == m) {
            Typing& tc = typeables_manager[t];
            Nameable& nc = nameables_manager[n];
            Moveable& mc = moveables_manager[m];
            Drawable& dc = drawables_manager[d];
            //            std::random_device rd;
            //            std::mt19937 gen(rd());
            //            std::uniform_real_distribution<double> ecce(0.24, sqrt(2));
            tc.types.set(0);
            tc.types.set(1);
            tc.types.set(2);
            size_t mn = std::min(nc.limit, strlen(name));
            strncpy(nc.name, name, mn);
            mc.clear();
            mc.position = p;
            Vector d = p - origin;
            double r = magnitude(d);
            double speed = sqrt(G * attractor.mass / r);
            printf("Speed=%lf\r\n", speed);
            mc.velocity = normalized(cross(d, basis::Z)) * speed * 8;
            mc.acceleration = zero;
            mc.mass = 100.0;
            dc.color[0] = 0x0;
            dc.color[1] = 0x0;
            dc.color[2] = 0x0;
            dc.color[3] = rand() % 256;
            dc.diameter = 4;
            return true;
        } else {
            // release all
            typeables_manager.Release(t);
            nameables_manager.Release(n);
            moveables_manager.Release(m);
            return false;
        }
    }

protected:
    using TypeableManager = Manager<Typing, NumEntities>;
    using NameableManager = Manager<Nameable, NumEntities>;
    using MoveableManager = Manager<Moveable, NumEntities>;
    using DrawableManager = Manager<Drawable, NumEntities>;

    TypeableManager typeables_manager;
    NameableManager nameables_manager;
    MoveableManager moveables_manager;
    DrawableManager drawables_manager;
    Moveable attractor;
    SDL_Window* window;
    SDL_Renderer* renderer;
};

Context c;

int main(void) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> unit(-1.0, 1.0);
    std::uniform_real_distribution<double> radius(1.25, 3.75);
    std::uniform_real_distribution<double> angle(-M_PI, M_PI);
    std::chrono::microseconds until{(Context::Fraction * 3600 * 24 * 365)};
    for (size_t i = 0; i < Context::NumEntities; i++) {
        char buffer[25];
        snprintf(buffer, sizeof(buffer), "rando-%zu", i);
        double r = radius(gen);
        double phi = angle(gen);
        double j = r * cos(phi);
        double k = r * sin(phi);
        double l = unit(gen);
        c.Add(i, buffer, Point{j, k, l});
    }
    c.Cycle(until);
    return 0;
}
