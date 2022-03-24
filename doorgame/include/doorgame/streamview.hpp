#pragma once

#include "doorgame/view.hpp"

namespace doorgame {

class StreamView : public View {
public:
    StreamView();
    void attempt(Event event) noexcept(false) override;
    void complete(Event event, bool result) noexcept(false) override;
    void display(const Actions& actions) override;
    void display(const Damages& damages) override;
    void display(const Directions& directions) override;
    void display(const Items& items) override;
    void display(const Targets& targets) override;
    void display(const Animate& object, std::string preface) override;
    // void display(const Game& game) override;
    void display(const Room& room) override;
    void display(const Storage& storage, std::string preface) override;
    void display(const Map& map) override;
    void display(const Player& player) override;
    void display(const Monster& monster) override;
    Action choose(const Actions& actions) override;
    Damage choose(const Damages& damages) override;
    Direction choose(const Directions& directions) override;
    Item choose(const Items& items) override;
    Target choose(const Targets& targets) override;
};

}  // namespace doorgame
