#include "doorgame/configuration.hpp"
#include "doorgame/streamview.hpp"

#include <basal/basal.hpp>

namespace doorgame {

std::ostream& operator<<(std::ostream& os, Action a) {
    // cSpell: disable
    switch (a) {
        case Action::Nothing:
            os << "[n]othing";
            break;
        case Action::Move:
            os << "[m]ove";
            break;
        case Action::Look:
            os << "[l]ook";
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
    // cSpell: enable
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
    // cSpell: disable
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
    // cSpell: enable
    return os;
}

std::ostream& operator<<(std::ostream& os, Item i) {
    // cSpell: disable
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
        case Item::Sword:
            os << "[s]word";
            break;
        case Item::Potion:
            os << "[p]otion";
            break;
        case Item::Bomb:
            os << "bo[m]b";
            break;
        default:
            os << "Unknown item (" << (char)i << ") ";
            break;
    }
    // cSpell: enable
    return os;
}

std::ostream& operator<<(std::ostream& os, Target target) {
    // cSpell: disable
    switch (target) {
        case Target::None:
            os << "[n]one";
            break;
        case Target::Player:
            os << "[p]layer";
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
    // cSpell: enable
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
    } else if (std::holds_alternative<Damage>(p)) {
        Damage d = std::get<3>(p);
        os << d;
    } else if (std::holds_alternative<Health>(p)) {
        Health h = std::get<4>(p);
        os << h;
    }
    return os;
}

StreamView::StreamView() {
}

void StreamView::attempt(Event event) {
    Target subject = std::get<0>(event);
    Action action = std::get<1>(event);
    Target object = std::get<2>(event);
    Parameter param = std::get<3>(event);
    if constexpr (debug::input) {
        std::cout << "[DEBUG] " << subject << " chose " << action << " to " << object << " with/by " << param
                  << std::endl;
    }
    switch (action) {
        case Action::Attack: {
            if (subject == Target::Player) {
                if (object == Target::Monster) {
                    // Damage dmg = std::get<3>(param);
                    std::cout << "You try to attack the " << object << "." << std::endl;
                }
            }
            break;
        }
        case Action::Use: {
            if (subject == Target::Player) {
                if (object == Target::Item) {
                    if (std::holds_alternative<Item>(param)) {
                        Item item = std::get<2>(param);  // it has to be an Item
                        std::cout << "You tries to use " << item << "." << std::endl;
                    } else {
                        // assert();
                    }
                }
            }
            break;
        }
        case Action::Look: {
            if (object == Target::Player) {
                std::cout << "You look into your inventory and assess yourself." << std::endl;
            }
            break;
        }
        default:
            break;
    }
}

void StreamView::complete(Event event, bool result) noexcept(false) {
    Target subject = std::get<0>(event);
    Action action = std::get<1>(event);
    Target object = std::get<2>(event);
    Parameter param = std::get<3>(event);

    switch (subject) {
        case Target::Player: {
            switch (action) {
                case Action::Attack: {
                    if (object == Target::None) {
                        std::cout << "You think twice and do not attack the heavy weight of tension in the air."
                                  << std::endl;
                    } else if (object == Target::Player) {
                        std::cout << "You have hurt yourself in your confusion!" << std::endl;
                    } else if (object == Target::Room) {
                        std::cout << "You flail about as if a spider was crawling on your neck." << std::endl;
                    } else if (object == Target::Monster) {
                        //
                    }
                    break;
                }
                case Action::Pickup: {
                    Item item = std::get<2>(param);
                    std::cout << "You picked up " << item << std::endl;
                    break;
                }
                case Action::Use: {
                    if (object == Target::Item) {
                        Item item = std::get<2>(param);
                        if (item == Item::Torch) {
                            std::cout << "You have used a lit torch and can see the room better" << std::endl;
                        } else if (item == Item::Book) {
                            std::cout << "You have read a book about the amazing world of tax derivatives during the "
                                         "middle-post modern era. You awake covered in sweat."
                                      << std::endl;
                        }
                    }
                    break;
                }
                case Action::Look: {
                    if (object == Target::Item) {
                        Item item = std::get<2>(param);
                        if (item == Item::Nothing) {
                            std::cout << "You stare at your empty palms. Is this what madness is?" << std::endl;
                        } else if (item == Item::Book) {
                            std::cout << "The spine of the book is cracked and aged." << std::endl;
                        } else {
                            std::cout << "It looks like any other " << item << " to you." << std::endl;
                        }
                    }
                    if (object == Target::Room) {
                        Direction dir = std::get<1>(param);
                        if (result) {
                            if (dir != Direction::Here) {
                                std::cout << "You looked to the room " << dir << " of here." << std::endl;
                            } else {
                                std::cout << "You look around in the room." << std::endl;
                            }
                        } else {
                            std::cout << "You can not look around " << dir << " of here." << std::endl;
                        }
                    }
                    break;
                }
                case Action::Move: {
                    if (object == Target::Player) {
                        Direction dir = std::get<1>(param);
                        if (result) {
                            if (dir == Direction::Here) {
                                std::cout << "You wisely decide not to move from this room." << std::endl;
                            } else {
                                std::cout << "You moved " << dir << "." << std::endl;
                            }
                        } else {
                            std::cout << "You can not move in that direction." << std::endl;
                        }
                    }
                    break;
                }
                case Action::Nothing:
                    std::cout << "You stare at the walls, slowly drooling, in delight." << std::endl;
                    break;
                case Action::Quit:
                    // premature end
                    std::cout << "The darkness encases you. Your silent gasp is the last thing you hear." << std::endl;
                    break;
            }
            break;
        }
        case Target::Item:
        case Target::Room:
        case Target::Monster:
            // FIXME Monsters can Pickup/Use/Attack/Move
        default:
            break;
    }
}
void StreamView::display(Actions const& actions) {
    static int rando = 0;
    static int const mod = 7;
    switch (rando % mod) {
        case 0:
            std::cout << "You feel an imperative to leave this place." << std::endl;
            break;
        case 1:
            std::cout << "You must find the exit." << std::endl;
            break;
        case 2:
            std::cout << "The hairs on the back of your neck tingle with dread." << std::endl;
            break;
        default:
            break;
    }
    // update random number
    rando = rand();
    std::cout << "Actions: ";
    for (auto a : actions) {
        std::cout << a << " ";
    }
}

void StreamView::display(Damages const& damages) {
    std::cout << "Damage:" << std::endl;
    for (auto d : damages) {
        std::cout << "\t" << d << std::endl;
    }
}

void StreamView::display(Directions const& directions) {
    std::cout << "Direction:" << std::endl;
    for (auto d : directions) {
        std::cout << "\t" << d << std::endl;
    }
}

void StreamView::display(Items const& items) {
    std::cout << "Items: " << std::endl;
    for (auto i : items) {
        std::cout << "\t" << i << std::endl;
    }
}

void StreamView::display(Targets const& targets) {
    std::cout << "Targets: " << std::endl;
    for (auto& target : targets) {
        std::cout << "\t" << target << std::endl;
    }
}

void StreamView::display(Animate const& object, std::string preface) {
    std::cout << preface << " Health: " << object.get_health() << ". Location is room " << object.location() << "."
              << std::endl;
}

void StreamView::display(Player const& player) {
    display(player.as_animate(), "Player");
    display(player.as_storage(), "Player");
}

void StreamView::display(Map const& map __attribute__((unused))) {
    return;
}

void StreamView::display(Monster const& monster) {
    display(monster.as_animate(), "Monster");
    display(monster.as_storage(), "Monster");
}

void StreamView::display(Storage const& storage, std::string preface) {
    std::cout << preface << " Inventory: ";
    for (auto& inv : storage.get_inventory()) {
        std::cout << inv << " ";
    }
    std::cout << std::endl;
}

void StreamView::display(Room const& room) {
    Directions directions = room.get_directions();
    std::cout << "The directions to travel in room " << room.get_id() << " are ";
    for (auto& dir : directions) {
        std::cout << dir << " ";
    }
    std::cout << "." << std::endl;
    if (room.is_investigated()) {
        display(room.as_storage(), "Room");
    }
}

Action StreamView::choose(Actions const& actions) {
    display(actions);
    std::cout << " ? " << std::flush;
    char c;
    std::cin >> c;
    Action a = static_cast<Action>(c);
    return (is_valid(a) ? a : Action::Nothing);
}

Damage StreamView::choose(Damages const& damages) {
    display(damages);
    std::string str_value;
    std::cin >> str_value;
    size_t value = std::stoul(str_value, nullptr, 10);
    Damage d = static_cast<Damage>(value);
    // FIXME round up or down to nearest enum?
    return (is_valid(d) ? d : Damage::None);
}

Direction StreamView::choose(Directions const& directions) {
    display(directions);
    char c;
    std::cin >> c;
    Direction d = static_cast<Direction>(c);
    return (is_valid(d) ? d : Direction::Here);
}

Item StreamView::choose(Items const& items) {
    display(items);
    char c;
    std::cin >> c;
    Item i = static_cast<Item>(c);
    return (is_valid(i) ? i : Item::Nothing);
}

Target StreamView::choose(Targets const& targets) {
    display(targets);
    char c;
    std::cin >> c;
    Target t = static_cast<Target>(c);
    return (is_valid(t) ? t : Target::None);
}

void StreamView::greeting(void) {
    std::cout << "▓█████▄  ▒█████   ▒█████   ██▀███    ▄████  ▄▄▄       ███▄ ▄███▓▓█████ " << std::endl;
    std::cout << "▒██▀ ██▌▒██▒  ██▒▒██▒  ██▒▓██ ▒ ██▒ ██▒ ▀█▒▒████▄    ▓██▒▀█▀ ██▒▓█   ▀ " << std::endl;
    std::cout << "░██   █▌▒██░  ██▒▒██░  ██▒▓██ ░▄█ ▒▒██░▄▄▄░▒██  ▀█▄  ▓██    ▓██░▒███   " << std::endl;
    std::cout << "░▓█▄   ▌▒██   ██░▒██   ██░▒██▀▀█▄  ░▓█  ██▓░██▄▄▄▄██ ▒██    ▒██ ▒▓█  ▄ " << std::endl;
    std::cout << "░▒████▓ ░ ████▓▒░░ ████▓▒░░██▓ ▒██▒░▒▓███▀▒ ▓█   ▓██▒▒██▒   ░██▒░▒████▒" << std::endl;
    std::cout << " ▒▒▓  ▒ ░ ▒░▒░▒░ ░ ▒░▒░▒░ ░ ▒▓ ░▒▓░ ░▒   ▒  ▒▒   ▓▒█░░ ▒░   ░  ░░░ ▒░ ░" << std::endl;
    std::cout << " ░ ▒  ▒   ░ ▒ ▒░   ░ ▒ ▒░   ░▒ ░ ▒░  ░   ░   ▒   ▒▒ ░░  ░      ░ ░ ░  ░" << std::endl;
    std::cout << " ░ ░  ░ ░ ░ ░ ▒  ░ ░ ░ ▒    ░░   ░ ░ ░   ░   ░   ▒   ░      ░      ░   " << std::endl;
    std::cout << "   ░        ░ ░      ░ ░     ░           ░       ░  ░       ░      ░  ░" << std::endl;
    std::cout << " ░                                                                     " << std::endl;
    std::cout << " AN ESCAPE FROM CARTESIAN TERROR" << std::endl;
}

void StreamView::completed(void) {
    std::cout << "You see the light of a new day. Your nightmare has ended." << std::endl;
}

void StreamView::goodbye(void) {
    std::cout << "" << std::endl;
}

}  // namespace doorgame