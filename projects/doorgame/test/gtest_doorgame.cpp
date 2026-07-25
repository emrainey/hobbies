#include <gtest/gtest.h>

#include <algorithm>
#include <deque>
#include <optional>

#include <doorgame/doorgame.hpp>

namespace doorgame {
namespace {

class TestGame final : public Game {
public:
    using Game::Game;

    Health player_health() const {
        return player.get_health();
    }

    Health monster_health(size_t index) const {
        return monsters.at(index).get_health();
    }

    bool monster_alive(size_t index) const {
        return monsters.at(index).is_alive();
    }
};

class MockView final : public View {
public:
    std::deque<Action> action_choices{};
    std::deque<Damage> damage_choices{};
    std::deque<Direction> direction_choices{};
    std::deque<Item> item_choices{};
    std::deque<Target> target_choices{};

    std::optional<Event> last_attempt{};
    std::optional<Event> last_complete{};
    bool last_complete_result{false};
    bool displayed_room_investigated{false};

    void attempt(Event event) noexcept(false) override {
        last_attempt = event;
    }

    void complete(Event event, bool result) noexcept(false) override {
        last_complete = event;
        last_complete_result = result;
    }

    void display(Actions const&) override {
    }
    void display(Damages const&) override {
    }
    void display(Directions const&) override {
    }
    void display(Items const&) override {
    }
    void display(Targets const&) override {
    }

    void display(Animate const&, std::string) override {
    }
    void display(Room const& room) override {
        displayed_room_investigated = room.is_investigated();
    }
    void display(Storage const&, std::string) override {
    }
    void display(Map const&) override {
    }
    void display(Player const&) override {
    }
    void display(Monster const&) override {
    }

    Action choose(Actions const&) override {
        auto value = action_choices.front();
        action_choices.pop_front();
        return value;
    }

    Damage choose(Damages const&) override {
        auto value = damage_choices.front();
        damage_choices.pop_front();
        return value;
    }

    Direction choose(Directions const&) override {
        auto value = direction_choices.front();
        direction_choices.pop_front();
        return value;
    }

    Item choose(Items const&) override {
        auto value = item_choices.front();
        item_choices.pop_front();
        return value;
    }

    Target choose(Targets const&) override {
        auto value = target_choices.front();
        target_choices.pop_front();
        return value;
    }

    void greeting(void) override {
    }

    void completed(void) override {
    }

    void goodbye(void) override {
    }
};

TEST(DoorgameValidationTest, EnumValidationDetectsKnownAndUnknownValues) {
    EXPECT_TRUE(is_valid(Action::Move));
    EXPECT_TRUE(is_valid(Direction::North));
    EXPECT_TRUE(is_valid(Item::Torch));
    EXPECT_TRUE(is_valid(Damage::Moderate));
    EXPECT_TRUE(is_valid(Target::Monster));
    EXPECT_TRUE(is_valid(Health::Dead));
    EXPECT_TRUE(is_valid(Health::Normal));

    EXPECT_FALSE(is_valid(static_cast<Action>('x')));
    EXPECT_FALSE(is_valid(static_cast<Direction>('x')));
    EXPECT_FALSE(is_valid(static_cast<Item>('x')));
    EXPECT_FALSE(is_valid(static_cast<Damage>(777)));
    EXPECT_FALSE(is_valid(static_cast<Target>('x')));
    EXPECT_FALSE(is_valid(static_cast<Health>(999)));
}

TEST(DoorgameStorageTest, AddRemoveRespectsCapacity) {
    Storage storage{2};

    EXPECT_TRUE(storage.add(Item::Torch));
    EXPECT_TRUE(storage.add(Item::Key));
    EXPECT_FALSE(storage.add(Item::Book));

    EXPECT_TRUE(storage.remove(Item::Torch));
    EXPECT_FALSE(storage.remove(Item::Potion));
    EXPECT_TRUE(storage.add(Item::Book));
}

TEST(DoorgameRoomTest, AdjacencyAndDirectionsAreTracked) {
    Room room{7};

    EXPECT_EQ(room.get_adjacent(Direction::Here), 7U);
    EXPECT_EQ(room.get_directions().size(), 1U);

    room.set_adjacent(8, Direction::North);
    room.set_adjacent(3, Direction::West);

    EXPECT_EQ(room.get_adjacent(Direction::North), 8U);
    EXPECT_EQ(room.get_adjacent(Direction::West), 3U);
    EXPECT_EQ(room.get_adjacent(Direction::South), 7U);

    auto directions = room.get_directions();
    EXPECT_TRUE(std::find(directions.begin(), directions.end(), Direction::Here) != directions.end());
    EXPECT_TRUE(std::find(directions.begin(), directions.end(), Direction::North) != directions.end());
    EXPECT_TRUE(std::find(directions.begin(), directions.end(), Direction::West) != directions.end());
    EXPECT_FALSE(std::find(directions.begin(), directions.end(), Direction::East) != directions.end());
}

TEST(DoorgameMapTest, LoadAndMoveFollowDoors) {
    Map map{2, 0, 1};
    Doors doors{{TwoWayDoor{0, Direction::North, 1, Direction::South}}};
    Stuff stuff{{ItemLocation{Item::Torch, 0}}};

    EXPECT_TRUE(map.load(doors, stuff));

    Player player{0};
    size_t adjacent = 0;
    EXPECT_TRUE(map.get_adjacent(player, Direction::North, adjacent));
    EXPECT_EQ(adjacent, 1U);

    EXPECT_TRUE(map.move(player, Direction::North));
    EXPECT_EQ(player.location(), 1U);
    EXPECT_TRUE(map.is_done(player));
}

TEST(DoorgameGameTest, AskEventForMoveUsesChosenDirection) {
    MockView view{};
    view.action_choices.push_back(Action::Move);
    view.direction_choices.push_back(Direction::North);

    Doors doors{{TwoWayDoor{0, Direction::North, 1, Direction::South}}};
    Stuff stuff{};
    MonsterList monsters{};
    Game game{view, 0, 1, 2, doors, stuff, monsters};

    Event event = game.ask_event();

    EXPECT_EQ(std::get<0>(event), Target::Player);
    EXPECT_EQ(std::get<1>(event), Action::Move);
    EXPECT_EQ(std::get<2>(event), Target::Player);
    EXPECT_EQ(std::get<Direction>(std::get<3>(event)), Direction::North);
}

TEST(DoorgameGameTest, ProcessLookHereInvestigatesAndDisplaysRoom) {
    MockView view{};
    Doors doors{};
    Stuff stuff{};
    MonsterList monsters{};
    Game game{view, 0, 1, 2, doors, stuff, monsters};

    Event event{Target::Player, Action::Look, Target::Room, Parameter{Direction::Here}};
    game.process(event);

    ASSERT_TRUE(view.last_attempt.has_value());
    ASSERT_TRUE(view.last_complete.has_value());
    EXPECT_EQ(view.last_attempt.value(), event);
    EXPECT_EQ(view.last_complete.value(), event);
    EXPECT_TRUE(view.last_complete_result);
    EXPECT_TRUE(view.displayed_room_investigated);
}

TEST(DoorgameGameTest, PickupConsumesItemAndSecondPickupFails) {
    MockView view{};
    Doors doors{};
    Stuff stuff{{ItemLocation{Item::Torch, 0}}};
    MonsterList monsters{};
    Game game{view, 0, 1, 2, doors, stuff, monsters};

    Event pickup{Target::Player, Action::Pickup, Target::Item, Parameter{Item::Torch}};
    game.process(pickup);
    EXPECT_TRUE(view.last_complete_result);

    game.process(pickup);
    EXPECT_FALSE(view.last_complete_result);
}

TEST(DoorgameGameTest, AttackMonsterAppliesDamageWithoutRetaliation) {
    MockView view{};
    Doors doors{};
    Stuff stuff{};
    MonsterList monsters{0};
    TestGame game{view, 0, 1, 2, doors, stuff, monsters};

    Event attack{Target::Player, Action::Attack, Target::Monster, Parameter{Damage::Cut}};
    game.process(attack);

    EXPECT_TRUE(view.last_complete_result);
    EXPECT_TRUE(game.monster_alive(0));
    EXPECT_EQ(static_cast<size_t>(game.monster_health(0)), static_cast<size_t>(Health::Normal) - static_cast<size_t>(Damage::Cut));
    EXPECT_EQ(game.player_health(), Health::Normal);
}

TEST(DoorgameGameTest, FatalAttackKillsMonsterAndHidesMonsterTarget) {
    MockView view{};
    Doors doors{};
    Stuff stuff{};
    MonsterList monsters{0};
    TestGame game{view, 0, 1, 2, doors, stuff, monsters};

    auto targets_before = game.get_targets();
    EXPECT_TRUE(std::find(targets_before.begin(), targets_before.end(), Target::Monster) != targets_before.end());

    Event attack{Target::Player, Action::Attack, Target::Monster, Parameter{Damage::Fatal}};
    game.process(attack);

    EXPECT_TRUE(view.last_complete_result);
    EXPECT_FALSE(game.monster_alive(0));
    EXPECT_EQ(game.monster_health(0), Health::Dead);
    EXPECT_EQ(game.player_health(), Health::Normal);

    auto targets_after = game.get_targets();
    EXPECT_FALSE(std::find(targets_after.begin(), targets_after.end(), Target::Monster) != targets_after.end());
}

}  // namespace
}  // namespace doorgame
