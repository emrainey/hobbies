# === Project ===
cmake_minimum_required(VERSION 3.31.0)
project(raytrace LANGUAGES CXX VERSION 0.7.0)  # This must match the conanfile.py!

# === Dependencies ===
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(OpenMP QUIET) # this could fail, but we have a backup
find_package(OpenCV)
find_package(Threads)
find_package(Curses)
find_package(SDL2)
find_package(GTest)
find_package(Doxygen)

# === Targets ===
if (NOT OpenMP_FOUND)
    if (EXISTS $ENV{HOMEBREW_ROOT}/opt/libomp)
        set(OpenMP_ROOT $ENV{HOMEBREW_ROOT}/opt/libomp)
        # Homebrew's libomp is not in the default search path
        add_library(OpenMP::OpenMP_CXX INTERFACE IMPORTED)
        target_compile_options(OpenMP::OpenMP_CXX INTERFACE $<IF:$<COMPILE_LANG_AND_ID:CXX,AppleClang>,,-fopenmp>)
        target_link_libraries(OpenMP::OpenMP_CXX INTERFACE omp)
        target_link_directories(OpenMP::OpenMP_CXX INTERFACE ${OpenMP_ROOT}/lib/)
        target_include_directories(OpenMP::OpenMP_CXX INTERFACE ${OpenMP_ROOT}/include/)
        set(OpenMP_FOUND TRUE)
        message(STATUS "Found OpenMP: ${OpenMP_ROOT}")
    endif()
endif()

add_library(hobbies-raytrace
    ${CMAKE_CURRENT_SOURCE_DIR}/source/animator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/bounds.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/color.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mapping.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/scene.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/stereocamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/tree.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mediums/stripes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mediums/happy_face.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/face.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/group.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/hyperboloid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/model.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/overlap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/paraboloid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/plane.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/pyramid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/quadratic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/ring.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/sphere.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/square.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/torus.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/polygon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/objects/wall.cpp
)
target_link_libraries(hobbies-raytrace
    PUBLIC
        hobbies-linalg
        hobbies-fourcc
        hobbies-noise
        hobbies-geometry
        $<$<BOOL:${USE_XMMT}>:hobbies-xmmt>
        $<$<BOOL:${OpenMP_FOUND}>:OpenMP::OpenMP_CXX>
    PRIVATE
        enabled-warnings
        enabled-debugging
        $<$<BOOL:${USE_PROFILING}>:enabled-profiling>
        $<$<BOOL:${USE_COVERAGE}>:enabled-coverage>
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

# === Demos ===
if (OpenCV_FOUND AND Threads_FOUND)
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

add_executable(demo_functions
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/main_functions.cpp
)
target_link_libraries(demo_functions PRIVATE ${OpenCV_LIBS} hobbies-raytrace hobbies-linalg-utils Threads::Threads)
endif()

if (Threads_FOUND AND CURSES_FOUND)
    add_executable(demo_curses
        ${CMAKE_CURRENT_SOURCE_DIR}/demo/main_console.cpp
    )
    # target_include_directories(demo_curses PRIVATE ${CURSES_INCLUDE_DIRS})
    target_link_libraries(demo_curses PRIVATE hobbies-raytrace ${CURSES_LIBRARIES} Threads::Threads)
    target_compile_options(demo_curses PRIVATE ${CURSES_CFLAGS})
endif()

if (SDL2_FOUND AND Threads_FOUND)
    add_executable(demo_sdl2
        ${CMAKE_CURRENT_SOURCE_DIR}/demo/main_sdl2.cpp
    )
    # target_include_directories(demo_sdl2 PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(demo_sdl2 PRIVATE hobbies-raytrace ${SDL2_LIBRARIES} Threads::Threads)
endif()

# === Worlds ===
list(APPEND DEMOS world_art1)
list(APPEND DEMOS world_cornell)
list(APPEND DEMOS world_desktoy)
list(APPEND DEMOS world_example)
list(APPEND DEMOS world_glass)
list(APPEND DEMOS world_gap)
list(APPEND DEMOS world_hexagon)
list(APPEND DEMOS world_lenses)
list(APPEND DEMOS world_mats)
list(APPEND DEMOS world_mirrors)
list(APPEND DEMOS world_monochrome world_monochrome2 world_monochrome3 world_monochrome4)
list(APPEND DEMOS world_outrun)
list(APPEND DEMOS world_planet)
list(APPEND DEMOS world_snowman)
list(APPEND DEMOS world_spheres world_spheres2 world_spheres3 world_spheres4)

foreach(demo IN LISTS DEMOS)
    add_library(${demo} SHARED ${CMAKE_CURRENT_SOURCE_DIR}/demo/${demo}.cpp)
    target_link_libraries(${demo} PRIVATE hobbies-raytrace)
    add_test(NAME test-${demo} COMMAND demo_sdl2 -m ${demo} -d QVGA -a 128 -s 0.125)
endforeach()

# === Googletests ===
if (BUILD_UNIT_TESTS AND Threads_FOUND AND GTest_FOUND)
    add_executable(gtest_raytrace
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_animator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_bounds.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_camera.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_color.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_cone.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_cuboid.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_cylinder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_entity.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_face.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_group.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_image.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_laws.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_lens.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_light.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_mapping.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_overlap.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_perf.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_plane.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_renders.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_ring.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_scene.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_sphere.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_square.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_stereocamera.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_surface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_torus.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_tree.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_polygon.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_objparser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_wall.cpp
    )
    target_link_libraries(gtest_raytrace PRIVATE hobbies-raytrace enabled-debugging GTest::gtest GTest::gmock GTest::gtest_main Threads::Threads)
    # CMake Test Plugin
    add_test(NAME gtest_raytrace COMMAND gtest_raytrace
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    set_tests_properties(gtest_raytrace
        PROPERTIES
            ENVIRONMENT LLVM_PROFILE_FILE=${CMAKE_BINARY_DIR}/coverage/gtest_raytrace.profraw
            ENVIRONMENT GCOV_PREFIX=${CMAKE_BINARY_DIR}/coverage
            # ENVIRONMENT GCOV_PREFIX_STRIP=${CMAKE_SOURCE_DIR}
    )

    coverage_target(gtest_raytrace)
endif()

# === Doxygen ===
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-raytrace-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/raytrace
        COMMENT "Raytracing Doxygen")
endif()

# === Profiling Helpers ===
function(profile_target ARG_DEMO_TARGET ARG_WORLD_TARGET)
    # PROFILING
    set(_TESTING ${CMAKE_SOURCE_DIR}/testing)
    set(_RAWPROF ${ARG_DEMO_TARGET}-${ARG_WORLD_TARGET}.profraw)
    set(DYLD_INSERT_LIBRARIES $ENV{HOMEBREW_ROOT}/Cellar/gperftools/2.16/lib/libprofiler.dylib)
    set(_PDF  ${ARG_DEMO_TARGET}-${ARG_WORLD_TARGET}-perf.pdf)
    # add_custom_target(raytrace-pprof-${ARG_DEMO_TARGET}-${ARG_WORLD_TARGET}
    #     DEPENDS ${ARG_DEMO_TARGET} ${ARG_WORLD_TARGET}
    #     COMMENT "Profiling Raytracing with pprof using ${ARG_WORLD_TARGET} and settings"
    #     WORKING_DIRECTORY ${_TESTING}
    #     COMMAND touch ${_RAWPROF}
    #     COMMAND rm ${_RAWPROF}
    #     COMMAND touch ${_PDF}
    #     COMMAND rm ${_PDF}
    #     # LLVM profiling is assumed
    #     COMMAND PATH=${CMAKE_CURRENT_BINARY_DIR}:$ENV{PATH} DYLD_LIBRARY_PATH=${DYLD_INSERT_LIBRARIES}:${CMAKE_CURRENT_BINARY_DIR} CPUPROFILE_FREQUENCY=1000 LLVM_PROFILE_FILE=${_RAWPROF} ${ARG_DEMO_TARGET} -m ${ARG_WORLD_TARGET}
    #     COMMAND pprof --pdf ${ARG_DEMO_TARGET} ${_RAWPROF} > ${_PDF}
    #     # ^ this has to be last to work
    #     BYPRODUCTS ${_RAWPROF} ${_PDF}
    # )
    set(_TXT_MANGLED ${CMAKE_CURRENT_BINARY_DIR}/${ARG_DEMO_TARGET}-${ARG_WORLD_TARGET}-profiling-mangled.txt)
    set(_TXT ${CMAKE_CURRENT_BINARY_DIR}/${ARG_DEMO_TARGET}-${ARG_WORLD_TARGET}-profiling.txt)
    set(CLT /Library/Developer/CommandLineTools/usr/bin/)
    add_custom_target(raytrace-llvmprof-${ARG_DEMO_TARGET}-${ARG_WORLD_TARGET}
        DEPENDS ${ARG_DEMO_TARGET} ${ARG_WORLD_TARGET}
        COMMENT "Profiling via llvm-profdata Raytracing with ${ARG_WORLD_TARGET} and settings"
        WORKING_DIRECTORY ${_TESTING}
        COMMAND touch ${_RAWPROF}
        COMMAND rm ${_RAWPROF}
        COMMAND DYLD_LIBRARY_PATH=${DYLD_INSERT_LIBRARIES}:${CMAKE_CURRENT_BINARY_DIR} LLVM_PROFILE_FILE=${_RAWPROF} ${CMAKE_CURRENT_BINARY_DIR}/${ARG_DEMO_TARGET} -m ${ARG_WORLD_TARGET}
        COMMAND ${CLT}/llvm-profdata show --detailed-summary --topn=100 --output=${_TXT_MANGLED} ${_RAWPROF}
        COMMAND cat ${_TXT_MANGLED} | c++filt -n  > ${_TXT}
        # ^ this has to be last to work
        BYPRODUCTS ${_TXT_MANGLED} ${_TXT} ${_RAWPROF}
    )
endfunction()

foreach(demo IN LISTS DEMOS)
profile_target(demo_sdl2 ${demo})
# profile_target(demo_curses ${demo})
endforeach()