#include "doorgame/streamview.hpp"

#include <basal/basal.hpp>

namespace doorgame {

std::ostream& operator<<(std::ostream& os, Action a) {
    switch (a) {
        case Action::Nothing:
            os << "[n]othing";
            break;
        case Action::Move:
            os << "[m]ove";
            break;
        case Action::Investigate:
            os << "[i]nvestigate";
            break;
        case Action::Pickup:
            os << "[p]ickup";
            break;
        case Action::Attack:
            os << "[a]ttack";
            break;
        case Action::Use:
            os << "[u]se";
            break;
        case Action::Quit:
            os << "[q]uit";
            break;
        default:
            os << "Unknown Action " << (char)a;
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, Damage value) {
    switch (value) {
        case Damage::None:
            os << "None";
            break;
        case Damage::Sting:
            os << "Sting";
            break;
        case Damage::Bite:
            os << "Bite";
            break;
        case Damage::Cut:
            os << "Cut";
            break;
        case Damage::Laceration:
            os << "Laceration";
            break;
        case Damage::Moderate:
            os << "Moderate";
            break;
        case Damage::Heavy:
            os << "Heavy";
            break;
        case Damage::Fatal:
            os << "Fatal";
            break;
    }
    os << " (" << basal::to_underlying(value) << ") ";
    return os;
}

std::ostream& operator<<(std::ostream& os, Health value) {
    size_t h = basal::to_underlying(value);
    if (value == Health::Dead) {
        os << "Dead";
    } else if (basal::to_underlying(Health::Dead) < h and h <= basal::to_underlying(Health::Critical)) {
        os << "Critical";
    } else if (basal::to_underlying(Health::Critical) < h and h <= basal::to_underlying(Health::Injured)) {
        os << "Injured";
    } else if (basal::to_underlying(Health::Injured) < h and h <= basal::to_underlying(Health::Hurt)) {
        os << "Hurt";
    } else if (basal::to_underlying(Health::Hurt) < h and h <= basal::to_underlying(Health::Bruised)) {
        os << "Bruised";
    } else if (basal::to_underlying(Health::Bruised) < h and h <= basal::to_underlying(Health::Normal)) {
        os << "Normal";
    }
    os << " (" << h << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, Direction d) {
    switch (d) {
        case Direction::Here:
            os << "[h]ere";
            break;
        case Direction::North:
            os << "[n]orth";
            break;
        case Direction::South:
            os << "[s]outh";
            break;
        case Direction::East:
            os << "[e]ast";
            break;
        case Direction::West:
            os << "[w]est";
            break;
        default:
            os << "Unknown Direction (" << (char)d << ") ";
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, Item i) {
    switch (i) {
        case Item::Nothing:
            os << "[n]othing";
            break;
        case Item::Torch:
            os << "[t]orch";
            break;
        case Item::Key:
            os << "[k]ey";
            break;
        case Item::Book:
            os << "[b]ook";
            break;
        default:
            os << "Unknown item (" << (char)i << ") ";
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, Target target) {
    switch (target) {
        case Target::None:
            os << "[n]one";
            break;
        case Target::Self:
            os << "[s]elf";
            break;
        case Target::Room:
            os << "[r]oom";
            break;
        case Target::Monster:
            os << "[m]onster";
            break;
        case Target::Item:
            os << "[i]tem";
            break;
        default:
            os << "Unknown target type (" << basal::to_underlying(target) << ") ";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, Parameter p) {
    if (std::holds_alternative<std::monostate>(p)) {
        os << "(empty)";
    } else if (std::holds_alternative<Direction>(p)) {
        Direction d = std::get<1>(p);
        os << d;
    } else if (std::holds_alternative<Item>(p)) {
        Item i = std::get<2>(p);
        os << i;
    } else if (std::holds_alternative<Target>(p)) {
        Target t = std::get<3>(p);
        os << t;
    }
    return os;
}

StreamView::StreamView() {
}

void StreamView::attempt(Action action, Parameter param) {
    std::cout << "Chose " << action << " and " << param << std::endl;
    switch (action) {
        case Action::Attack: {
            // throw_exception_unless(std::holds_alternative<Target>(param), "Must have a Target");
            Target target = std::get<3>(param);
            std::cout << "You try to attack " << target << "." << std::endl;
            break;
        }
        case Action::Use: {
            // throw_exception_unless(std::holds_alternative<Item>(param), "Must have an Item");
            Item item = std::get<2>(param);  // it has to be an Item
            std::cout << "You tries to use " << item << "." << std::endl;
            break;
        }
        default:
            break;
    }
}

void StreamView::complete(Action action, Parameter param, bool result) noexcept(false) {
    switch (action) {
        case Action::Attack: {
            // throw_exception_unless(std::holds_alternative<Target>(param), "Must have a Target");
            Target target = std::get<3>(param);
            if (target == Target::None) {
                std::cout << "You think twice and do not attack the heavy weight of tension in the air." << std::endl;
            } else if (target == Target::Self) {
                std::cout << "You have hurt yourself in your confusion!" << std::endl;
            } else if (target == Target::Room) {
                std::cout << "You flail about as if a spider was crawling on your neck." << std::endl;
            } else if (target == Target::Monster) {
                //
            }
            break;
        }
        case Action::Pickup: {
            // throw_exception_unless(std::holds_alternative<Item>(param), "Must have an Item");
            Item item = std::get<2>(param);
            std::cout << "You picked up " << item << std::endl;
        }
        case Action::Use: {
            // throw_exception_unless(std::holds_alternative<Item>(param), "Must have an Item");
            Item item = std::get<2>(param);
            if (item == Item::Torch) {
                std::cout << "You have used a lit torch and can see the room better" << std::endl;
            } else if (item == Item::Book) {
                std::cout << "You have read a book about the amazing world of tax derivatives during the "
                             "middle-post modern era. You awake covered in sweat."
                          << std::endl;
            }
            break;
        }
        case Action::Investigate: {
            // throw_exception_unless(std::holds_alternative<Direction>(param), "Must have a Direction");
            Direction dir = std::get<1>(param);
            if (result) {
                std::cout << "You investigated " << dir << "." << std::endl;
            } else {
                std::cout << "You can not investigate " << dir << "." << std::endl;
            }
            break;
        }
        case Action::Move: {
            Direction dir = std::get<1>(param);
            if (result) {
                std::cout << "You moved " << dir << "." << std::endl;
            } else {
                std::cout << "You can not move " << dir << "." << std::endl;
            }
            break;
        }
        case Action::Nothing:
            std::cout << "You stare at the walls, slowly drooling, in delight." << std::endl;
            break;
        case Action::Quit:
            std::cout << "The darkness encases you. Your silent gasp is the last thing you hear." << std::endl;
            break;
    }
}
void StreamView::display(const Actions& actions) {
    std::cout << "Actions: ";
    for (auto a : actions) {
        std::cout << a << " ";
    }
}

void StreamView::display(const Damages& damages) {
    std::cout << "Damage:" << std::endl;
    for (auto d : damages) {
        std::cout << "\t" << d << std::endl;
    }
}

void StreamView::display(const Directions& directions) {
    std::cout << "Direction:" << std::endl;
    for (auto d : directions) {
        std::cout << "\t" << d << std::endl;
    }
}

void StreamView::display(const Items& items) {
    std::cout << "Items: " << std::endl;
    for (auto i : items) {
        std::cout << "\t" << i << std::endl;
    }
}

void StreamView::display(const Targets& targets) {
    std::cout << "Targets: " << std::endl;
    for (auto& target : targets) {
        std::cout << "\t" << target << std::endl;
    }
}

void StreamView::display(const Animate& object) {
    std::cout << "Health: " << object.get_health() << ". Location is room " << object.location() << "." << std::endl;
}

void StreamView::display(const Player& player) {
    std::cout << "Player:" << std::endl;
    display(player.as_animate());
    display(player.as_storage());
}

void StreamView::display(const Map& map) {
    return;
}

void StreamView::display(const Monster& monster) {
    std::cout << "Monster:" << std::endl;
    display(monster.as_animate());
    display(monster.as_storage());
}

void StreamView::display(const Storage& storage) {
    std::cout << "Inventory: ";
    for (auto& inv : storage.get_inventory()) {
        std::cout << inv << " ";
    }
    std::cout << std::endl;
}

void StreamView::display(const Room& room) {
    Directions directions = room.get_directions();
    std::cout << "The directions to travel in room " << room.get_id() << " are ";
    for (auto& dir : directions) {
        std::cout << dir << " ";
    }
    std::cout << "." << std::endl;
    if (room.is_investigated()) {
        display(room.as_storage());
    }
}

Action StreamView::choose(const Actions& actions) {
    display(actions);
    std::cout << " ? " << std::flush;
    char c;
    std::cin >> c;
    Action a = static_cast<Action>(c);
    return (is_valid(a) ? a : Action::Nothing);
}

Damage StreamView::choose(const Damages& damages) {
    display(damages);
    char c;
    std::cin >> c;
    Damage d = static_cast<Damage>(c);
    // FIXME round up or down to nearest enum?
    return (is_valid(d) ? d : Damage::None);
}

Direction StreamView::choose(const Directions& directions) {
    display(directions);
    char c;
    std::cin >> c;
    Direction d = static_cast<Direction>(c);
    return (is_valid(d) ? d : Direction::Here);
}

Item StreamView::choose(const Items& items) {
    display(items);
    char c;
    std::cin >> c;
    Item i = static_cast<Item>(c);
    return (is_valid(i) ? i : Item::Nothing);
}

Target StreamView::choose(const Targets& targets) {
    display(targets);
    char c;
    std::cin >> c;
    Target t = static_cast<Target>(c);
    return (is_valid(t) ? t : Target::None);
}

}  // namespace doorgame