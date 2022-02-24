cmake_minimum_required(VERSION 3.20.0)
project(raytrace CXX)

find_package(OpenCV REQUIRED)
find_package(OpenMP REQUIRED)
# find_package(ncurses REQUIRED) # should be installed in the system
find_package(basal REQUIRED)
find_package(units_of_measure REQUIRED)
find_package(linalg REQUIRED)
find_package(linalg-utils REQUIRED)
find_package(geometry REQUIRED)
find_package(noise REQUIRED)
find_package(xmmt REQUIRED)

# Build Everything with C++17 and full native optimizations
set(CMAKE_CXX_STANDARD 17)
enable_testing()

add_library(hobbies-raytrace SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/source/camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/color.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/scene.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mapping.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lights/beam.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lights/bulb.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lights/speck.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lights/spot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/lights/light.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mediums/medium.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mediums/functions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mediums/dielectric.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mediums/conductor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mediums/opaque.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mediums/transparent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mediums/metal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mediums/plain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mediums/dots.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mediums/checkerboard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mediums/grid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mediums/marble.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mediums/random_noise.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/cone.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/cuboid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/cylinder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/ellipsoid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/ellipticalcone.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/ellipticalcylinder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/hyperboloid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/overlap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/paraboloid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/plane.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/pyramid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/quadratic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/ring.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/sphere.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/square.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/torus.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/triangle.cpp
)
target_link_libraries(hobbies-raytrace
    PUBLIC Linalg::hobbies-linalg Fourcc::hobbies-fourcc Noise::hobbies-noise Geometry::hobbies-geometry Xmmt::hobbies-xmmt Basal::native-optimized
    PRIVATE OpenMP::OpenMP_CXX
)
target_include_directories(hobbies-raytrace
    PUBLIC
       # Generated Tree
       $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
       # Build Tree
       $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
       # Install Tree
       $<INSTALL_INTERFACE:include>
)

################
# DEMOS
################

if (OpenCV_FOUND)
add_executable(demo_sphere_surfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/main_sphere_surfaces.cpp
)
target_link_libraries(demo_sphere_surfaces PRIVATE ${OpenCV_LIBS} hobbies-raytrace LinalgUtils::hobbies-linalg-utils Threads::Threads)

add_executable(demo_raytrace_loader
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/main_opencv.cpp
)
target_link_libraries(demo_raytrace_loader ${OpenCV_LIBS} hobbies-raytrace LinalgUtils::hobbies-linalg-utils Threads::Threads)

add_executable(demo_random_tuner
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/main_random_tuner.cpp
)
target_link_libraries(demo_random_tuner PRIVATE ${OpenCV_LIBS} hobbies-raytrace LinalgUtils::hobbies-linalg-utils Threads::Threads)
endif()

find_package(Threads REQUIRED)
find_package(Curses)
if (CURSES_FOUND)
    add_executable(demo_curses
        ${CMAKE_CURRENT_SOURCE_DIR}/demo/main_console.cpp
    )
    target_include_directories(demo_curses PRIVATE ${CURSES_INCLUDE_DIRS})
    target_link_libraries(demo_curses PRIVATE hobbies-raytrace ${CURSES_LIBRARIES} Threads::Threads)
    target_compile_options(demo_curses PRIVATE ${CURSES_CFLAGS})
endif()

#################################
# WORLDS
#################################
set(DEMOS world_example world_spheres world_cornell
          world_desktoy world_monochrome world_snowman
          world_lenses world_planet world_spheres2 world_example2)

foreach(demo IN LISTS DEMOS)
    add_library(${demo} SHARED ${CMAKE_CURRENT_SOURCE_DIR}/demo/${demo}.cpp)
    target_link_libraries(${demo} PRIVATE hobbies-raytrace)
endforeach()

#####################
# Googletests
#####################

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
if (GTest_FOUND)
    add_executable(gtest_raytrace
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_camera.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_color.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_cone.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_cuboid.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_cylinder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_entity.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_image.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_laws.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_light.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_mapping.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_perf.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_plane.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_renders.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_ring.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_scene.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_sphere.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_surface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_torus.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_triangle.cpp
    )
    target_include_directories(gtest_raytrace SYSTEM PUBLIC ${GTEST_INCLUDE_DIRS})
    target_link_libraries(gtest_raytrace PRIVATE ${GTEST_BOTH_LIBRARIES} hobbies-raytrace Threads::Threads)
    add_test(NAME gtest_raytrace COMMAND gtest_raytrace)
endif()

#####################
# Build Documentation
#####################

find_package(Doxygen)
if (${Doxygen_FOUND})
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-raytrace-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/raytrace
        COMMENT "Raytracing Doxygen")
endif()


#####################
# EXPORTS
#####################

# Declare installs and exports
install(TARGETS hobbies-raytrace EXPORT RaytraceTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
install(EXPORT RaytraceTargets
    FILE RaytraceTargets.cmake
    NAMESPACE Raytrace::
    DESTINATION share/cmake/raytrace
)
install(DIRECTORY include/raytrace
    DESTINATION include
    PATTERN "*.hpp"
    PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
)
install(FILES "cmake/Findraytrace.cmake" "cmake/raytrace-config.cmake"
    DESTINATION share/cmake/raytrace
)