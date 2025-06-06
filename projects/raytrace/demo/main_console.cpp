///
/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Wraps a rendered scene with some NCurses magic
/// @copyright Copyright (c) 2022
///

/// This allows rendering directly to the Terminal via RGB codes.
/// It's not the worst, but I haven't been able to make it a "live" view, as I'd like.
#undef RENDER_TO_CONSOLE

#include <ncurses.h>

#include <basal/module.hpp>
#include <basal/options.hpp>
#include <chrono>
#include <functional>
#include <iomanip>
#include <raytrace/raytrace.hpp>
#include <thread>

#include "world.hpp"

using namespace std::placeholders;
using namespace basal::literals;

struct Parameters {
    std::string dim_name;
    size_t subsamples;
    size_t reflections;
    precision fov;
    std::string module;
    size_t mask_threshold;
};

enum class State : char {
    MENU = 'm',
    DRAW = 'd'
};

class Console {
public:
    Console() : m_width{0}, m_height{0}, m_buffer{0} {
        // initialize
        initscr();
        raw();  // we'll get the characters for Ctrl+C too
        start_color();
        keypad(stdscr, TRUE);  // use Fn keys
        noecho();              // don't echo input
        // get screen details
        getmaxyx(stdscr, m_height, m_width);
        wborder(stdscr, '|', '|', '-', '-', '+', '+', '+', '+');

        init_pair(INFO_PAIR, COLOR_WHITE, COLOR_BLACK);
        init_pair(PROGRESS_PAIR, COLOR_RED, COLOR_BLACK);
    }
    ~Console() {
        endwin();
    }

    /// Clears the screen and redraws the window edge
    void clear() {
        attron(COLOR_PAIR(INFO_PAIR));
        for (size_t y = 0; y < m_height; y++) {
            for (size_t x = 0; x < m_width; x++) {
                move(y, x);
                addch(' ');
            }
        }
        attroff(COLOR_PAIR(INFO_PAIR));
        wborder(stdscr, '|', '|', '-', '-', '+', '+', '+', '+');
        refresh();
    }

    void progress(size_t row, precision percent) {
        size_t lb = 1, rb = 1;
        size_t span = m_width - lb - rb;
        attron(COLOR_PAIR(PROGRESS_PAIR));
        move(row, lb);
        addch('[');
        for (size_t x = lb + 1; x < (m_width - rb); x++) {
            precision here = 100.0_p * (x - lb) / span;
            move(row, x);
            if (here <= percent) {
                addch('*');
            } else {
                addch('_');
            }
        }
        move(row, m_width - rb - 1);
        addch(']');
        attroff(COLOR_PAIR(PROGRESS_PAIR));
        // refresh();
    }

    void refresh() {
        ::refresh();
    }

    void print(size_t y, size_t x, char const fmt[], ...) {
        va_list list;
        va_start(list, fmt);
        vsnprintf(m_buffer, sizeof(m_buffer), fmt, list);
        va_end(list);
        move(y, x);
        printw(m_buffer);
        // refresh();
    }

    void wait_for_input(std::function<void(int)> func) {
        int input = getch();
        if (func) {
            func(input);
        }
    }

    int get_width() const {
        return m_width;
    }

    int get_height() const {
        return m_height;
    }

protected:
    int m_width;
    int m_height;
    char m_buffer[1024];
    static int const INFO_PAIR = 1;
    static int const PROGRESS_PAIR = 2;
};

int main(int argc, char* argv[]) {
    Parameters params;
    bool verbose = false;
    bool should_quit = false;
    bool should_render = true;
    precision move_unit = 5.0_p;
    std::string module_name;
    State state = State::MENU;
    std::time_t start_time;
    std::chrono::duration<precision> diff;
    std::chrono::time_point start = std::chrono::steady_clock::now();
    char time_string[256];

    basal::options::config opts[] = {
        {"-d", "--dims", std::string("QVGA"), "Use text video format like VGA or 2K"},
        {"-b", "--subsamples", (size_t)1, "Number of sub-samples"},
        {"-r", "--reflections", (size_t)4, "Reflection Depth"},
        {"-f", "--fov", 55.0_p, "Field of View in Degrees"},
        {"-v", "--verbose", false, "Enables showing the early debugging"},
        {"-m", "--module", std::string(""), "Module to load"},
        {"-a", "--aaa", (size_t)raytrace::image::AAA_MASK_DISABLED,
         "Adaptive Anti-Aliasing Threshold value (255 disables)"},
    };

    basal::options::process(basal::dimof(opts), opts, argc, argv);
    basal::exit_unless(basal::options::find(opts, "--dims", params.dim_name), __FILE__, __LINE__,
                       "Must have a text value");
    basal::exit_unless(basal::options::find(opts, "--fov", params.fov), __FILE__, __LINE__, "Must have a FOV value");
    basal::exit_unless(basal::options::find(opts, "--verbose", verbose), __FILE__, __LINE__,
                       "Must be able to assign bool");
    basal::exit_unless(basal::options::find(opts, "--subsamples", params.subsamples), __FILE__, __LINE__,
                       "Must have some number of subsamples");
    basal::exit_unless(basal::options::find(opts, "--reflections", params.reflections), __FILE__, __LINE__,
                       "Must have some number of reflections");
    basal::exit_unless(basal::options::find(opts, "--module", params.module), __FILE__, __LINE__,
                       "Must choose a module to load");
    basal::exit_unless(basal::options::find(opts, "--aaa", params.mask_threshold), __FILE__, __LINE__,
                       "Must be get value");
    basal::options::print(basal::dimof(opts), opts);

    basal::module mod(params.module.c_str());
    basal::exit_unless(mod.is_loaded(), __FILE__, __LINE__, "Must have loaded module");

    // get the symbol to load wth
    auto get_world = mod.get_symbol<raytrace::world_getter>("get_world");
    basal::exit_unless(get_world != nullptr, __FILE__, __LINE__, "Must find module to load");

    // creates a local reference to the object
    raytrace::world& world = *get_world();
    auto [width, height] = fourcc::dimensions(params.dim_name);
    // Ncurses console
    Console console;
#if defined(RENDER_TO_CONSOLE)
    basal::exit_unless(console.get_width() % 2 == 0, __FILE__, __LINE__, "Must be an even console width");
#endif
    // The completion data will be stored in here, a bool per line.
#if defined(RENDER_TO_CONSOLE)
    std::vector<bool> completed(console.get_height() * 2);
#else
    std::vector<bool> completed(height);
#endif
    std::fill(completed.begin(), completed.end(), false);
    bool running = true;

    auto progress_bar = [&]() -> void {
        while (running) {
            size_t count = 0;
            std::for_each(completed.begin(), completed.end(), [&](bool p) -> bool {
                count += (p ? 1 : 0);
                return p;
            });
            precision percentage = 100.0_p * count / completed.size();
            bool done = (count == completed.size());
            if (state == State::MENU) {
                size_t h = console.get_height() - 2;  // account for border
                console.print(h - 1, 2, " >> PROGRESS [ %0.3lf %%]", done ? 100.0_p : percentage);
                console.progress(h, percentage);
                diff = std::chrono::steady_clock::now() - start;
                console.print(6, 2, "START TIME: %s, RENDERING TIME: %0.3lf secs", time_string, diff.count());

                console.print(9, 2, "GEOMETRY");
                console.print(10, 2, "  DOT: %zu", geometry::statistics::get().dot_operations);
                console.print(11, 2, "CROSS: %zu", geometry::statistics::get().cross_products);
                console.print(12, 2, " NORM: %zu", geometry::statistics::get().magnitudes);
                console.print(13, 2, "LINALG:");
                console.print(14, 2, "  MULT: %zu", linalg::statistics::get().matrix_multiply);
                console.print(15, 2, "  QUAD: %zu", linalg::statistics::get().quadratic_roots);
                console.print(16, 2, " CUBIC: %zu", linalg::statistics::get().cubic_roots);
                console.print(17, 2, " QUART: %zu", linalg::statistics::get().quartic_roots);
                size_t w = console.get_width() / 2;
                console.print(9, w, "RAYS");
                console.print(10, w, "  CAST: %zu", raytrace::statistics::get().cast_rays_from_camera);
                console.print(11, w, "  INTR: %zu", raytrace::statistics::get().intersections_with_objects);
                console.print(12, w, "BOUNCE: %zu", raytrace::statistics::get().bounced_rays);
                console.print(13, w, "  XMIT: %zu", raytrace::statistics::get().transmitted_rays);
                console.print(14, w, " SAVED: %zu", raytrace::statistics::get().saved_ray_traces);
                console.print(15, w, "SHADOW: %zu", raytrace::statistics::get().shadow_rays);
                console.print(16, w, "SAMPLE: %zu", raytrace::statistics::get().sampled_rays);
                console.print(17, w, " S.CLR: %zu", raytrace::statistics::get().color_sampled_rays);
                console.print(18, w, "ABSORB: %zu", raytrace::statistics::get().absorbed_rays);
                console.print(19, w, "MISSED: %zu", raytrace::statistics::get().missed_rays);
                console.print(20, w, "INSIDE: %zu", raytrace::statistics::get().inside_out_intersections);
                console.print(21, w, "BOUNDS: %zu", raytrace::statistics::get().intersections_with_bounds);
                console.refresh();
            }
            // sleep for a bit
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    };

    auto row_notifier = [&](size_t row_index, bool is_complete) -> void { completed[row_index] = is_complete; };

    raytrace::scene scene;
#if defined(RENDER_TO_CONSOLE)
    raytrace::camera view(console.get_height() * 2, console.get_width(), iso::degrees(params.fov));
#else
    raytrace::camera view(height, width, iso::degrees(params.fov));
#endif
    raytrace::vector looking = (world.looking_at() - world.looking_from()).normalized();
    raytrace::point image_plane_principal_point = world.looking_from() + looking;
    view.move_to(world.looking_from(), image_plane_principal_point);

    scene.set_background_mapper(std::bind(&raytrace::world::background, &world, std::placeholders::_1));
    world.add_to(scene);
    if (verbose) {
        scene.print(std::cout, world.window_name().c_str());
    }

    do {
        if (state == State::MENU) {
            console.print(1, 2, "RAYTRACE by Erik Rainey. Loaded module %s", params.module.c_str());
            console.print(2, 2, "CONSOLE {%dx%d}, IMAGE {%zux%zu} RGB8", console.get_width(), console.get_height(),
                          width, height);
            console.print(3, 2, "SUBSAMPLES: %zu, REFLECTIONS: %zu ANTI-ALIAS MASK: %zu", params.subsamples,
                          params.reflections, params.mask_threshold);
            console.print(4, 2, "CAMERA FROM {%0.3lf,%0.3lf,%0.3lf} AT {%0.3lf,%0.3lf,%0.3lf} FOV=%0.2lf",
                          world.looking_from().x, world.looking_from().y, world.looking_from().z, world.looking_at().x,
                          world.looking_at().y, world.looking_at().z, params.fov);
            console.print(5, 2, "#LIGHTS %zu #OBJECTS %zu", scene.number_of_lights(), scene.number_of_objects());
            // console.print(6, 2, "START TIME: %s, RENDERING TIME: %0.3lf secs", std::ctime(&start_time),
            // diff.count());
#if defined(RENDER_TO_CONSOLE)
            console.print(7, 2, "Commands: (r)ender, (m)enu, (d)raw, (q)uit");
#else
            console.print(7, 2, "Commands: (r)ender, (q)uit");
#endif
            console.refresh();
        }
#if defined(RENDER_TO_CONSOLE)
        else if (state == State::DRAW) {
            printf("\e[s\e[?25l");  // save cursor?
            printf("\e[u");         // clear screen?
            for (size_t y = 0; y < view.capture.height; y += 2) {
                for (size_t x = 0; x < view.capture.width; x++) {
                    fourcc::rgb8 top = view.capture.at(y + 0, x);
                    fourcc::rgb8 btm = view.capture.at(y + 1, x);
                    printf("\x1b[48;2;%d;%d;%dm\x1b[38;2;%d;%d;%dm\u2584", top.r, top.g, top.b, btm.r, btm.g, btm.b);
                }
            }
            printf("\e[?25h");
            console.refresh();
        }
#endif
        // wait for input
        console.wait_for_input([&](int input) {
            switch (input) {
                case KEY_F(1):
                    // show function menu instead?
                    break;
                case 'r': {
                    std::chrono::time_point start = std::chrono::steady_clock::now();
                    start_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                    std::strftime(time_string, dimof(time_string), "%A %c", std::localtime(&start_time));
                    console.print(6, 2, "START TIME: %s, RENDERING TIME: ??? secs", time_string);
                    std::thread bar_thread(progress_bar);  // thread starts
                    try {
                        scene.render(view, world.output_filename(), params.subsamples, params.reflections, row_notifier,
                                     params.mask_threshold);
                    } catch (basal::exception const& e) {
                        std::cout << "Caught basal::exception in scene.render()! " << std::endl;
                        std::cout << "What:" << e.what() << " Why:" << e.why() << " Where:" << e.where() << std::endl;
                    } catch (...) {
                        std::cout << "Caught unknown exception in scene.render()! " << std::endl;
                    }
                    diff = std::chrono::steady_clock::now() - start;
                    running = false;
                    bar_thread.join();  // thread stop
                    console.print(6, 2, "START TIME: %s, RENDERING TIME: %0.3lf secs", time_string, diff.count());
                    break;
                }
#if defined(RENDER_TO_CONSOLE)
                case 'd':
                    if (has_colors()) {
                        state = State::DRAW;
                        // console.clear();
                        endwin();
                    } else {
                        console.print(10, 0, "Console can not render image!");
                    }
                    break;
                case 'm':
                    state = State::MENU;
                    console.clear();
                    break;
#endif
                case 'q':
                    should_quit = true;
                    break;
                default:
                    break;
            }
        });
    } while (not should_quit);
    return 0;
}