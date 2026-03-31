///
/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Renders a raytraced scene directly into the terminal using FTXUI.
/// @copyright Copyright (c) 2026
///

#include <basal/module.hpp>
#include <basal/options.hpp>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <functional>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <optional>
#include <raytrace/raytrace.hpp>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/loop.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/box.hpp>
#include <ftxui/screen/color.hpp>

#include "world.hpp"

using namespace std::placeholders;
using namespace basal::literals;
using namespace geometry::operators;

namespace {

struct Parameters {
    size_t subsamples;
    size_t reflections;
    precision fov;
    std::string module;
    size_t mask_threshold;
};

struct FrameSnapshot {
    std::vector<fourcc::rgb8> pixels;
    size_t width{0};
    size_t height{0};
    bool has_image{false};
    bool rendering{false};
    size_t completed_rows{0};
    size_t total_rows{0};
    std::string started_at;
    double render_seconds{0.0};
    std::string status{"Idle"};
};

struct CameraPose {
    raytrace::point from;
    precision yaw{0.0_p};
    precision pitch{0.0_p};
    precision view_distance{1.0_p};
};

constexpr int kMinimumViewportWidth = 16;
constexpr int kMinimumViewportHeight = 8;
constexpr precision kMoveUnit = 2.5_p;
constexpr precision kTurnUnit = 7.5_p;
constexpr precision kPitchLimit = 80.0_p;
constexpr char kPixelGlyph[] = "▀";

std::string FormatPoint(raytrace::point const& point) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2) << "{" << point.x() << ", " << point.y() << ", " << point.z() << "}";
    return stream.str();
}

std::string FormatDouble(double value, int precision_digits = 3) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(precision_digits) << value;
    return stream.str();
}

std::string FormatProgress(size_t completed, size_t total) {
    std::ostringstream stream;
    if (total == 0U) {
        stream << "0.000";
    } else {
        stream << std::fixed << std::setprecision(3) << (100.0 * static_cast<double>(completed) / total);
    }
    return stream.str() + "%";
}

ftxui::Color::Palette16 QuantizeColor(fourcc::rgb8 const& color) {
    const int red = color.components.r;
    const int green = color.components.g;
    const int blue = color.components.b;
    const int max_value = std::max({red, green, blue});
    const int min_value = std::min({red, green, blue});
    const int spread = max_value - min_value;

    if (max_value < 18) {
        return ftxui::Color::Black;
    }
    if (spread < 18) {
        if (max_value < 64) {
            return ftxui::Color::GrayDark;
        }
        if (max_value < 160) {
            return ftxui::Color::GrayLight;
        }
        return ftxui::Color::White;
    }

    const bool bright = max_value >= 160;
    const int yellowish = (red + green) / 2 - blue;
    const int cyanish = (green + blue) / 2 - red;
    const int magentaish = (red + blue) / 2 - green;

    if (yellowish > 28) {
        return bright ? ftxui::Color::YellowLight : ftxui::Color::Yellow;
    }
    if (cyanish > 28) {
        return bright ? ftxui::Color::CyanLight : ftxui::Color::Cyan;
    }
    if (magentaish > 28) {
        return bright ? ftxui::Color::MagentaLight : ftxui::Color::Magenta;
    }

    if (red >= green && red >= blue) {
        return bright ? ftxui::Color::RedLight : ftxui::Color::Red;
    }
    if (green >= red && green >= blue) {
        return bright ? ftxui::Color::GreenLight : ftxui::Color::Green;
    }
    return bright ? ftxui::Color::BlueLight : ftxui::Color::Blue;
}

raytrace::vector ComputeForward(precision yaw_degrees, precision pitch_degrees) {
    const precision yaw = yaw_degrees * iso::pi / 180.0_p;
    const precision pitch = pitch_degrees * iso::pi / 180.0_p;
    const precision cos_pitch = std::cos(pitch);
    raytrace::vector forward{cos_pitch * std::cos(yaw), cos_pitch * std::sin(yaw), std::sin(pitch)};
    forward.normalize();
    return forward;
}

raytrace::vector ComputeRight(raytrace::vector const& forward) {
    raytrace::vector right = geometry::R3::cross(forward, geometry::R3::basis::Z);
    if (basal::nearly_zero(right.magnitude())) {
        right = geometry::R3::basis::Y;
    }
    right.normalize();
    return right;
}

CameraPose BuildInitialPose(raytrace::world const& world) {
    CameraPose pose;
    pose.from = world.looking_from();
    raytrace::vector look = world.looking_at() - world.looking_from();
    pose.view_distance = look.magnitude();
    if (basal::nearly_zero(pose.view_distance)) {
        pose.view_distance = 1.0_p;
    }
    look.normalize();
    pose.yaw = std::atan2(look.y(), look.x()) * 180.0_p / iso::pi;
    pose.pitch = std::asin(std::clamp(look.z(), -1.0_p, 1.0_p)) * 180.0_p / iso::pi;
    return pose;
}

raytrace::point ComputeLookAt(CameraPose const& pose) {
    return pose.from + (ComputeForward(pose.yaw, pose.pitch) * pose.view_distance);
}

}  // namespace

int main(int argc, char* argv[]) {
    Parameters params;
    bool verbose = false;

    basal::options::config opts[] = {
        {"-b", "--subsamples", (size_t)1, "Number of sub-samples"},
        {"-r", "--reflections", (size_t)4, "Reflection Depth"},
        {"-f", "--fov", 55.0_p, "Field of View in Degrees"},
        {"-v", "--verbose", false, "Enables showing the early debugging"},
        {"-m", "--module", std::string(""), "Module to load"},
        {"-a", "--aaa", (size_t)raytrace::image::AAA_MASK_DISABLED,
         "Adaptive Anti-Aliasing Threshold value (255 disables)"},
    };

    basal::options::process(basal::dimof(opts), opts, argc, argv);
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

    auto get_world = mod.get_symbol<raytrace::world_getter>("get_world");
    basal::exit_unless(get_world != nullptr, __FILE__, __LINE__, "Must find module to load");

    raytrace::world& world = *get_world();
    raytrace::scene scene;
    scene.set_background_mapper(std::bind(&raytrace::world::background, &world, _1));
    world.add_to(scene);
    scene.set_ambient_light(world.ambient());
    if (verbose) {
        scene.print(std::cout, world.window_name().c_str());
    }

    CameraPose pose = BuildInitialPose(world);

    std::mutex state_mutex;
    FrameSnapshot frame;
    bool render_requested = true;
    bool render_in_flight = false;
    bool shutdown_requested = false;
    bool bootstrap_refresh_requested = false;
    size_t viewport_width = 0;
    size_t viewport_height = 0;
    ftxui::Box viewport_box;

    auto apply_pose_to_world = [&]() {
        world.looking_from() = pose.from;
        world.looking_at() = ComputeLookAt(pose);
    };
    apply_pose_to_world();

    std::vector<std::string> control_lines = {
        "W/S: forward/backward",       "A/D: strafe left/right", "R/F: move up/down", "Q/E: turn left/right",
        "Arrow Up/Down: look up/down", "Space: re-render",       "Esc: quit",
    };

    size_t info_width = 0;
    auto update_info_width = [&]() {
        std::vector<std::string> labels = {
            "Module: ",        "World: ",        "Viewport: ",      "Status: ",         "Progress: ",
            "Render Time: ",   "FOV: ",          "Subsamples: ",    "Reflections: ",    "AAA Threshold: ",
            "Camera From: ",   "Camera At: ",    "Yaw/Pitch: ",     "Objects/Lights: ", "Rays Cast: ",
            "Intersections: ", "Bounced/Xmit: ", "Missed/Bounds: ", "Geometry: ",       "Controls:",
        };
        for (auto const& label : labels) {
            info_width = std::max(info_width, label.size());
        }
        for (auto const& line : control_lines) {
            info_width = std::max(info_width, line.size());
        }
        info_width += 4U;
    };
    update_info_width();

    ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();

    auto request_render = [&](size_t width, size_t height) {
        std::scoped_lock lock(state_mutex);
        if (width < static_cast<size_t>(kMinimumViewportWidth)
            || height < static_cast<size_t>(kMinimumViewportHeight)) {
            frame.status = "Viewport too small to render";
            frame.rendering = false;
            frame.total_rows = 0;
            frame.completed_rows = 0;
            return;
        }
        viewport_width = width;
        viewport_height = height;
        render_requested = true;
    };

    auto render_worker = std::thread([&]() {
        while (not shutdown_requested) {
            size_t next_width = 0;
            size_t next_height = 0;
            CameraPose next_pose;
            std::string started_at;
            {
                std::scoped_lock lock(state_mutex);
                if (render_in_flight || not render_requested) {
                    next_width = 0;
                } else {
                    if (viewport_width == 0 || viewport_height == 0) {
                        next_width = 0;
                        next_height = 0;
                        continue;
                    }
                    render_requested = false;
                    render_in_flight = true;
                    next_width = viewport_width;
                    next_height = viewport_height;
                    next_pose = pose;
                    frame.rendering = true;
                    frame.status = "Rendering";
                    frame.total_rows = next_height;
                    frame.completed_rows = 0;
                    frame.render_seconds = 0.0;
                    auto const now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                    std::tm local_time = *std::localtime(&now);
                    char buffer[128];
                    std::strftime(buffer, sizeof(buffer), "%F %T", &local_time);
                    started_at = buffer;
                    frame.started_at = started_at;
                }
            }

            if (shutdown_requested) {
                break;
            }

            if (next_width == 0 || next_height == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
                continue;
            }

            try {
                raytrace::camera camera(next_height, next_width, iso::degrees(params.fov));
                camera.move_to(next_pose.from, ComputeLookAt(next_pose));
                auto start = std::chrono::steady_clock::now();
                auto row_notifier = [&](size_t row_index, bool is_complete) {
                    std::scoped_lock lock(state_mutex);
                    if (row_index >= frame.total_rows) {
                        return;
                    }
                    if (is_complete) {
                        frame.completed_rows = std::min(frame.total_rows, frame.completed_rows + 1U);
                    }
                };
                scene.render(camera, std::string{}, params.subsamples, params.reflections, row_notifier,
                             static_cast<uint8_t>(params.mask_threshold), false, false);

                FrameSnapshot next_frame;
                next_frame.width = camera.capture.width;
                next_frame.height = camera.capture.height;
                next_frame.has_image = true;
                next_frame.rendering = false;
                next_frame.completed_rows = next_frame.height;
                next_frame.total_rows = next_frame.height;
                next_frame.started_at = started_at;
                next_frame.render_seconds
                    = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
                next_frame.status = "Ready";
                next_frame.pixels.reserve(next_frame.width * next_frame.height);
                for (size_t y = 0; y < next_frame.height; ++y) {
                    for (size_t x = 0; x < next_frame.width; ++x) {
                        raytrace::color value(
                            camera.capture.at(y, x).components.r, camera.capture.at(y, x).components.g,
                            camera.capture.at(y, x).components.b, camera.capture.at(y, x).components.i);
                        value.clamp();
                        value.ToEncoding(fourcc::Encoding::GammaCorrected);
                        next_frame.pixels.push_back(value.to_<fourcc::PixelFormat::RGB8>());
                    }
                }

                {
                    std::scoped_lock lock(state_mutex);
                    frame = std::move(next_frame);
                    render_in_flight = false;
                }
            } catch (basal::exception const& e) {
                std::scoped_lock lock(state_mutex);
                frame.rendering = false;
                frame.status = std::string{"Render error: "} + e.what();
                render_in_flight = false;
            } catch (std::exception const& e) {
                std::scoped_lock lock(state_mutex);
                frame.rendering = false;
                frame.status = std::string{"Render error: "} + e.what();
                render_in_flight = false;
            } catch (...) {
                std::scoped_lock lock(state_mutex);
                frame.rendering = false;
                frame.status = "Render error: unknown exception";
                render_in_flight = false;
            }

            screen.PostEvent(ftxui::Event::Custom);
        }
    });

    auto app = ftxui::Renderer([&]() {
        if (not bootstrap_refresh_requested) {
            bootstrap_refresh_requested = true;
            screen.PostEvent(ftxui::Event::Custom);
        }

        FrameSnapshot snapshot;
        CameraPose pose_snapshot;
        size_t current_viewport_width = 0;
        size_t current_viewport_height = 0;
        {
            std::scoped_lock lock(state_mutex);
            snapshot = frame;
            pose_snapshot = pose;
            current_viewport_width = viewport_width;
            current_viewport_height = viewport_height;
        }

        const int raw_width
            = viewport_box.x_max >= viewport_box.x_min ? (viewport_box.x_max - viewport_box.x_min + 1) : 0;
        const int raw_height
            = viewport_box.y_max >= viewport_box.y_min ? (viewport_box.y_max - viewport_box.y_min + 1) : 0;
        size_t desired_width = raw_width > 0 ? static_cast<size_t>(raw_width) : 0U;
        size_t desired_height = raw_height > 0 ? static_cast<size_t>(raw_height) * 2U : 0U;
        if (desired_width % 2U == 1U) {
            desired_width -= 1U;
        }
        if (desired_height % 2U == 1U) {
            desired_height -= 1U;
        }

        if ((desired_width != 0U && desired_height != 0U)
            && (desired_width != current_viewport_width || desired_height != current_viewport_height)) {
            request_render(desired_width, desired_height);
        }

        ftxui::Elements info_rows;
        auto add_info = [&](std::string const& label, std::string const& value) {
            info_rows.push_back(ftxui::text(label + value));
        };

        add_info("Module: ", params.module);
        add_info("World: ", world.window_name());
        add_info("Viewport: ", std::to_string(desired_width) + "x" + std::to_string(desired_height));
        add_info("Status: ", snapshot.status + (snapshot.rendering ? "..." : ""));
        add_info("Progress: ", FormatProgress(snapshot.completed_rows, snapshot.total_rows));
        add_info("Render Time: ", snapshot.has_image ? (FormatDouble(snapshot.render_seconds) + " s") : "n/a");
        add_info("FOV: ", FormatDouble(params.fov, 2));
        add_info("Subsamples: ", std::to_string(params.subsamples));
        add_info("Reflections: ", std::to_string(params.reflections));
        add_info("AAA Threshold: ", std::to_string(params.mask_threshold));
        add_info("Camera From: ", FormatPoint(pose_snapshot.from));
        add_info("Camera At: ", FormatPoint(ComputeLookAt(pose_snapshot)));
        add_info("Yaw/Pitch: ", FormatDouble(pose_snapshot.yaw, 1) + "/" + FormatDouble(pose_snapshot.pitch, 1));
        add_info("Objects/Lights: ",
                 std::to_string(scene.number_of_objects()) + "/" + std::to_string(scene.number_of_lights()));
        add_info("Rays Cast: ", std::to_string(raytrace::statistics::get().cast_rays_from_camera));
        add_info("Intersections: ", std::to_string(raytrace::statistics::get().intersections_with_objects));
        add_info("Bounced/Xmit: ", std::to_string(raytrace::statistics::get().bounced_rays) + "/"
                                       + std::to_string(raytrace::statistics::get().transmitted_rays));
        add_info("Missed/Bounds: ", std::to_string(raytrace::statistics::get().missed_rays) + "/"
                                        + std::to_string(raytrace::statistics::get().intersections_with_bounds));
        add_info("Geometry: ", std::to_string(geometry::statistics::get().dot_operations) + " dot, "
                                   + std::to_string(geometry::statistics::get().cross_products) + " cross");
        if (not snapshot.started_at.empty()) {
            add_info("Started: ", snapshot.started_at);
        }
        info_rows.push_back(ftxui::separator());
        info_rows.push_back(ftxui::text("Controls:"));
        for (auto const& line : control_lines) {
            info_rows.push_back(ftxui::text(line));
        }

        ftxui::Elements view_rows;
        if (snapshot.has_image && snapshot.width > 0U && snapshot.height > 1U) {
            for (size_t y = 0; y + 1U < snapshot.height; y += 2U) {
                ftxui::Elements columns;
                columns.reserve(snapshot.width);
                for (size_t x = 0; x < snapshot.width; ++x) {
                    auto const& top = snapshot.pixels[y * snapshot.width + x];
                    auto const& bottom = snapshot.pixels[(y + 1U) * snapshot.width + x];
                    columns.push_back(ftxui::text(kPixelGlyph) | ftxui::bgcolor(ftxui::Color(QuantizeColor(top)))
                                      | ftxui::color(ftxui::Color(QuantizeColor(bottom))));
                }
                view_rows.push_back(ftxui::hbox(std::move(columns)));
            }
        } else {
            view_rows.push_back(ftxui::text(snapshot.rendering ? "Rendering scene..." : "Waiting for first render"));
        }

        auto info_panel = ftxui::window(ftxui::text(" Info "), ftxui::vbox(std::move(info_rows)))
                          | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, static_cast<int>(info_width));
        auto view_panel = ftxui::window(ftxui::text(" View "),
                                        ftxui::vbox(std::move(view_rows)) | ftxui::reflect(viewport_box) | ftxui::flex)
                          | ftxui::flex;

        return ftxui::hbox({info_panel, view_panel | ftxui::flex}) | ftxui::flex;
    });

    app = ftxui::CatchEvent(app, [&](ftxui::Event event) {
        if (event == ftxui::Event::Escape || event == ftxui::Event::Character('x')) {
            shutdown_requested = true;
            screen.Exit();
            return true;
        }

        bool changed = false;
        auto forward = ComputeForward(pose.yaw, pose.pitch);
        auto right = ComputeRight(forward);

        if (event == ftxui::Event::Character('w') || event == ftxui::Event::Character('W')) {
            pose.from += forward * kMoveUnit;
            changed = true;
        } else if (event == ftxui::Event::Character('s') || event == ftxui::Event::Character('S')) {
            pose.from -= forward * kMoveUnit;
            changed = true;
        } else if (event == ftxui::Event::Character('a') || event == ftxui::Event::Character('A')) {
            pose.from -= right * kMoveUnit;
            changed = true;
        } else if (event == ftxui::Event::Character('d') || event == ftxui::Event::Character('D')) {
            pose.from += right * kMoveUnit;
            changed = true;
        } else if (event == ftxui::Event::Character('r') || event == ftxui::Event::Character('R')) {
            pose.from.z() += kMoveUnit;
            changed = true;
        } else if (event == ftxui::Event::Character('f') || event == ftxui::Event::Character('F')) {
            pose.from.z() -= kMoveUnit;
            changed = true;
        } else if (event == ftxui::Event::Character('q') || event == ftxui::Event::Character('Q')) {
            pose.yaw += kTurnUnit;
            changed = true;
        } else if (event == ftxui::Event::Character('e') || event == ftxui::Event::Character('E')) {
            pose.yaw -= kTurnUnit;
            changed = true;
        } else if (event == ftxui::Event::ArrowUp) {
            pose.pitch = std::min(kPitchLimit, pose.pitch + kTurnUnit);
            changed = true;
        } else if (event == ftxui::Event::ArrowDown) {
            pose.pitch = std::max(-kPitchLimit, pose.pitch - kTurnUnit);
            changed = true;
        } else if (event == ftxui::Event::Character(' ')) {
            changed = true;
        } else if (event == ftxui::Event::Custom) {
            return true;
        }

        if (changed) {
            std::scoped_lock lock(state_mutex);
            apply_pose_to_world();
            render_requested = true;
            return true;
        }
        return false;
    });

    screen.Loop(app);

    shutdown_requested = true;
    if (render_worker.joinable()) {
        render_worker.join();
    }

    apply_pose_to_world();
    std::cout << "Final camera for module '" << params.module << "'" << std::endl;
    std::cout << "  looking_from = " << FormatPoint(world.looking_from()) << std::endl;
    std::cout << "  looking_at   = " << FormatPoint(world.looking_at()) << std::endl;
    std::cout << "  yaw/pitch    = " << FormatDouble(pose.yaw, 3) << "/" << FormatDouble(pose.pitch, 3)
              << " deg" << std::endl;
    std::cout << "  fov          = " << FormatDouble(params.fov, 3) << " deg" << std::endl;

    return 0;
}