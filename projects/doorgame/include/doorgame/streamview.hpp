#pragma once

#include "doorgame/view.hpp"

namespace doorgame {

class StreamView : public View {
public:
    StreamView();
    void attempt(Event event) noexcept(false) override;
    void complete(Event event, bool result) noexcept(false) override;
    void display(Actions const& actions) override;
    void display(Damages const& damages) override;
    void display(Directions const& directions) override;
    void display(Items const& items) override;
    void display(Targets const& targets) override;
    void display(Animate const& object, std::string preface) override;
    // void display(Game const& game) override;
    void display(Room const& room) override;
    void display(Storage const& storage, std::string preface) override;
    void display(Map const& map) override;
    void display(Player const& player) override;
    void display(Monster const& monster) override;
    Action choose(Actions const& actions) override;
    Damage choose(Damages const& damages) override;
    Direction choose(Directions const& directions) override;
    Item choose(Items const& items) override;
    Target choose(Targets const& targets) override;
    void greeting(void) override;
    void completed(void) override;
    void goodbye(void) override;
};

}  // namespace doorgame
