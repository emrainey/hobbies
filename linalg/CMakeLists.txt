cmake_minimum_required(VERSION 3.20.0)
project(linalg CXX)

find_package(basal REQUIRED)
find_package(units_of_measure REQUIRED)
find_package(fourcc REQUIRED)

# Build Everything with C++17 and full native optimizations
set(CMAKE_CXX_STANDARD 17)
enable_testing()

option(BUILD_SHARED_LIBS "Builds libraries as dynamic objects" OFF)

add_library(hobbies-linalg
    ${CMAKE_CURRENT_SOURCE_DIR}/source/matrix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/solvers.cpp
)
target_link_libraries(hobbies-linalg PUBLIC
    Basal::hobbies-basal UnitsOfMeasure::hobbies-uom)
target_include_directories(hobbies-linalg PUBLIC
    # Generated Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    # Build Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    # Install Tree
    $<INSTALL_INTERFACE:include>
)

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
    FILE LinalgTargets.cmake
    NAMESPACE Linalg::
    DESTINATION share/cmake/linalg
)
install(DIRECTORY include/linalg
    DESTINATION include
    PATTERN "*.hpp"
    PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
)
install(FILES "cmake/Findlinalg.cmake" "cmake/linalg-config.cmake"
    DESTINATION share/cmake/linalg
)
#####################
# Googletests
#####################

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
if (GTest_FOUND)
    add_executable(gtest_linalg
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_solvers.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_matrix.cpp
    )
    target_include_directories(gtest_linalg SYSTEM PUBLIC ${GTEST_INCLUDE_DIRS})
    target_link_libraries(gtest_linalg PRIVATE ${GTEST_BOTH_LIBRARIES} hobbies-linalg Threads::Threads)
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