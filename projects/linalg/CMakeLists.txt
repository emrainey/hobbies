cmake_minimum_required(VERSION 3.20.0)
project(linalg LANGUAGES CXX VERSION 0.7.0)  # This must match the conanfile.py!

find_package(basal 0.5.0 REQUIRED)
find_package(units_of_measure REQUIRED)
find_package(fourcc REQUIRED)

# Build Everything with C++17 and full native optimizations
set(CMAKE_CXX_STANDARD 17)
enable_testing()

option(BUILD_SHARED_LIBS "Builds libraries as dynamic objects" OFF)
option(USE_PRECISION_AS_FLOAT "Use float (ON) as the precision or double (OFF)" OFF)

add_library(hobbies-linalg
    ${CMAKE_CURRENT_SOURCE_DIR}/source/matrix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/solvers.cpp
)
target_link_libraries(hobbies-linalg
    PUBLIC
        Basal::hobbies-basal
        UnitsOfMeasure::hobbies-uom
    PRIVATE
        Basal::enabled-warnings
        Basal::enabled-debugging
        Basal::enabled-profiling
        Basal::native-optimized
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
target_compile_definitions(hobbies-linalg PUBLIC $<$<BOOL:${USE_PRECISION_AS_FLOAT}>:USE_PRECISION_AS_FLOAT>)

#####################
# EXPORTS
#####################

# Declare installs and exports
install(TARGETS hobbies-linalg EXPORT LinalgTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
install(EXPORT LinalgTargets
    FILE ${PROJECT_NAME}-Targets.cmake
    NAMESPACE Linalg::
    DESTINATION share/cmake/${PROJECT_NAME}
)
install(DIRECTORY include/${PROJECT_NAME}
    DESTINATION include
    PATTERN "*.hpp"
    PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
)
install(FILES "cmake/Find${PROJECT_NAME}.cmake" "cmake/${PROJECT_NAME}-config.cmake"
    DESTINATION share/cmake/${PROJECT_NAME}
)
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
    target_link_libraries(gtest_linalg PRIVATE hobbies-linalg GTest::gtest GTest::gtest_main Threads::Threads)
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