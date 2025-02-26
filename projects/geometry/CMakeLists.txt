cmake_minimum_required(VERSION 3.31.0)
project(geometry LANGUAGES CXX VERSION 0.7.0)  # This must match the conanfile.py!

add_library(hobbies-geometry
    ${CMAKE_CURRENT_SOURCE_DIR}/source/extra_math.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/intersection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/line.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/plane.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/point.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/ray.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/sphere.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/vector.cpp
)
target_link_libraries(hobbies-geometry
    PUBLIC
        hobbies-basal
        hobbies-uom
        hobbies-linalg
    PRIVATE
        enabled-warnings
        enabled-debugging
        $<$<BOOL:${USE_PROFILING}>:enabled-profiling>
        $<$<BOOL:${USE_COVERAGE}>:enabled-coverage>
        native-optimized
)
target_include_directories(hobbies-geometry PUBLIC
    # Generated Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    # Build Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    # Install Tree
    $<INSTALL_INTERFACE:include>
)
set_target_properties(hobbies-geometry PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

#####################
# EXPORTS
#####################

# Declare installs and exports
# install(TARGETS hobbies-geometry EXPORT GeometryTargets
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
#     RUNTIME DESTINATION bin
#     INCLUDES DESTINATION include
# )
# install(EXPORT GeometryTargets
#     FILE ${PROJECT_NAME}-Targets.cmake
#     NAMESPACE Geometry::
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
#####################
# Googletests
#####################

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
find_package(GTest)
if (Threads_FOUND AND GTest_FOUND)
    add_executable(gtest_geometry
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_point.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_extra.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_intersections.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_line.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_plane.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_ray.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_sphere.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_vector.cpp
    )
    target_include_directories(gtest_geometry SYSTEM PUBLIC ${GTEST_INCLUDE_DIRS})
    target_link_libraries(gtest_geometry PRIVATE hobbies-geometry enabled-debugging GTest::gtest GTest::gmock GTest::gtest_main Threads::Threads)
    add_test(NAME gtest_geometry COMMAND gtest_geometry)
    # set_tests_properties(gtest_geometry
    #     PROPERTIES
    #         ENVIRONMENT LLVM_PROFILE_FILE=${CMAKE_BINARY_DIR}/coverage/gtest_geometry.profraw
    #         ENVIRONMENT GCOV_PREFIX=${CMAKE_BINARY_DIR}/coverage
    #         # ENVIRONMENT GCOV_PREFIX_STRIP=${CMAKE_SOURCE_DIR}
    # )
    coverage_target(gtest_geometry)
endif()

#####################
# Build Documentation
#####################
find_package(Doxygen)
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-geometry-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/geometry
        COMMENT "Geometry Doxygen")
endif()