cmake_minimum_required(VERSION 3.13)
project(linalg CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
find_package(GTest REQUIRED)
find_package(basal REQUIRED)
find_package(units_of_measure REQUIRED)
find_package(fourcc REQUIRED)
enable_testing()

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
add_definitions(-march=native -mtune=native)
else()
message(ERROR "No other native format is yet supported!")
endif()

add_library(emrainey-linalg SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/source/matrix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/solvers.cpp
)
target_link_libraries(emrainey-linalg PUBLIC
    Basal::emrainey-basal UnitsOfMeasure::emrainey-uom)
target_include_directories(emrainey-linalg PUBLIC
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
install(TARGETS emrainey-linalg EXPORT LinalgTargets
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

if (${GTest_FOUND})
    add_executable(gtest_linalg
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_solvers.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_matrix.cpp
    )
    target_include_directories(gtest_linalg SYSTEM PUBLIC ${GTEST_INCLUDE_DIRS})
    target_link_libraries(gtest_linalg ${GTEST_BOTH_LIBRARIES} emrainey-linalg)
    add_test(NAME gtest_linalg COMMAND gtest_linalg)
endif()

#####################
# Build Documentation
#####################

if (${Doxygen_FOUND})
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(emrainey-linalg-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/linalg
        COMMENT "Linalg Doxygen")
endif()