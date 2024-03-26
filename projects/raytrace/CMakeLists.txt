cmake_minimum_required(VERSION 3.20.0)
project(raytrace LANGUAGES CXX VERSION 0.7.0)  # This must match the conanfile.py!

find_package(OpenMP)
if (NOT OpenMP_FOUND)
# Homebrew's libomp is not in the default search path
add_library(OpenMP::OpenMP_CXX INTERFACE IMPORTED)
target_compile_options(OpenMP::OpenMP_CXX INTERFACE -fopenmp)
target_link_libraries(OpenMP::OpenMP_CXX INTERFACE omp)
target_link_directories(OpenMP::OpenMP_CXX INTERFACE /opt/homebrew/Cellar/libomp/18.1.2/lib/)
endif()

add_library(hobbies-raytrace
    ${CMAKE_CURRENT_SOURCE_DIR}/source/camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/stereocamera.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mediums/perlin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mediums/turbsin.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/wall.cpp
)
target_link_libraries(hobbies-raytrace
    PUBLIC
        hobbies-linalg
        hobbies-fourcc
        hobbies-noise
        hobbies-geometry
        hobbies-xmmt
        OpenMP::OpenMP_CXX
    PRIVATE
        enabled-warnings
        enabled-debugging
        enabled-profiling
        native-optimized
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
set_target_properties(hobbies-raytrace PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

################
# DEMOS
################

find_package(OpenCV)
if (OpenCV_FOUND)
add_executable(demo_sphere_surfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/main_sphere_surfaces.cpp
)
target_link_libraries(demo_sphere_surfaces PRIVATE ${OpenCV_LIBS} hobbies-raytrace hobbies-linalg-utils Threads::Threads)

add_executable(demo_opencv_gui
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/main_opencv.cpp
)
target_link_libraries(demo_opencv_gui ${OpenCV_LIBS} hobbies-raytrace hobbies-linalg-utils Threads::Threads)

add_executable(demo_random_tuner
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/main_random_tuner.cpp
)
target_link_libraries(demo_random_tuner PRIVATE ${OpenCV_LIBS} hobbies-raytrace hobbies-linalg-utils Threads::Threads)
endif()

find_package(Threads)
find_package(Curses)
if (Threads_FOUND AND CURSES_FOUND)
    add_executable(demo_curses
        ${CMAKE_CURRENT_SOURCE_DIR}/demo/main_console.cpp
    )
    target_include_directories(demo_curses PRIVATE ${CURSES_INCLUDE_DIRS})
    target_link_libraries(demo_curses PRIVATE hobbies-raytrace ${CURSES_LIBRARIES} Threads::Threads)
    target_compile_options(demo_curses PRIVATE ${CURSES_CFLAGS})
endif()

find_package(Threads)
find_package(SDL2)
if (SDL2_FOUND)
    add_executable(demo_sdl2
        ${CMAKE_CURRENT_SOURCE_DIR}/demo/main_sdl2.cpp
    )
    target_include_directories(demo_sdl2 PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(demo_sdl2 PRIVATE hobbies-raytrace ${SDL2_LIBRARIES} Threads::Threads)
endif()

#################################
# WORLDS
#################################
list(APPEND DEMOS world_art1)
list(APPEND DEMOS world_cornell)
list(APPEND DEMOS world_desktoy)
list(APPEND DEMOS world_example)
list(APPEND DEMOS world_glass)
list(APPEND DEMOS world_lenses)
list(APPEND DEMOS world_mats)
list(APPEND DEMOS world_monochrome world_monochrome2)
list(APPEND DEMOS world_outrun)
list(APPEND DEMOS world_planet)
list(APPEND DEMOS world_snowman)
list(APPEND DEMOS world_spheres world_spheres2)

foreach(demo IN LISTS DEMOS)
    add_library(${demo} SHARED ${CMAKE_CURRENT_SOURCE_DIR}/demo/${demo}.cpp)
    target_link_libraries(${demo} PRIVATE hobbies-raytrace)
endforeach()

#####################
# Googletests
#####################

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
find_package(GTest)
if (Threads_FOUND AND GTest_FOUND)
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
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_stereocamera.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_surface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_torus.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_triangle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_objparser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_wall.cpp
    )
    target_link_libraries(gtest_raytrace PRIVATE hobbies-raytrace enabled-debugging GTest::gtest GTest::gmock GTest::gtest_main Threads::Threads)
    add_test(NAME gtest_raytrace COMMAND gtest_raytrace)
endif()

#####################
# Build Documentation
#####################

find_package(Doxygen)
if (Doxygen_FOUND)
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
# install(TARGETS hobbies-raytrace EXPORT RaytraceTargets
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
#     RUNTIME DESTINATION bin
#     INCLUDES DESTINATION include
# )
# install(EXPORT RaytraceTargets
#     FILE ${PROJECT_NAME}-Targets.cmake
#     NAMESPACE Raytrace::
#     DESTINATION share/cmake/${PROJECT_NAME}
# )
# install(DIRECTORY include/${PROJECT_NAME}
#     DESTINATION include
#     PATTERN "*.hpp"
#     PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
# )
# install(FILES "cmake/Find${PROJECT_NAME}.cmake" "cmake/${PROJECT_NAME}-config.cmake"
#     DESTINATION share/cmake/${PROJECT_NAME}
# )