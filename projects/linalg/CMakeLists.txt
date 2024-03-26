cmake_minimum_required(VERSION 3.20.0)
project(linalg LANGUAGES CXX VERSION 0.7.0)  # This must match the conanfile.py!

add_library(hobbies-linalg
    ${CMAKE_CURRENT_SOURCE_DIR}/source/matrix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/solvers.cpp
)
target_link_libraries(hobbies-linalg
    PUBLIC
        hobbies-basal
        # hobbies-geometry
        hobbies-uom
    PRIVATE
        enabled-warnings
        enabled-debugging
        enabled-profiling
        native-optimized
)
target_include_directories(hobbies-linalg PUBLIC
    # Generated Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    # Build Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    # Install Tree
    $<INSTALL_INTERFACE:include>
)
set_target_properties(hobbies-linalg PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

#####################
# EXPORTS
#####################

# # Declare installs and exports
# install(TARGETS hobbies-linalg EXPORT LinalgTargets
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
#     RUNTIME DESTINATION bin
#     INCLUDES DESTINATION include
# )
# install(EXPORT LinalgTargets
#     FILE ${PROJECT_NAME}-Targets.cmake
#     NAMESPACE Linalg::
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
    add_executable(gtest_linalg
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_solvers.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_matrix.cpp
    )
    target_link_libraries(gtest_linalg PRIVATE hobbies-linalg enabled-debugging GTest::gtest GTest::gtest_main Threads::Threads)
    add_test(NAME gtest_linalg COMMAND gtest_linalg)
endif()

#####################
# Build Documentation
#####################

find_package(Doxygen)
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-linalg-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/linalg
        COMMENT "Linalg Doxygen")
endif()