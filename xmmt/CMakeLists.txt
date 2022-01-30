cmake_minimum_required(VERSION 3.13)
project(xmmt
    VERSION 0.5
    LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(basal REQUIRED)
enable_testing()

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
add_definitions(-march=native -mtune=native)
else()
message(ERROR "No other native format is yet supported!")
endif()

add_library(emrainey-xmmt INTERFACE)
target_include_directories(emrainey-xmmt INTERFACE
    # Generated Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    # Build Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    # Install Tree
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(emrainey-xmmt INTERFACE Basal::emrainey-basal)

#####################
# EXPORTS
#####################

# Declare installs and exports (this isn't working yet)
install(TARGETS emrainey-xmmt EXPORT XmmtTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
install(EXPORT XmmtTargets
    FILE XmmtTargets.cmake
    NAMESPACE Xmmt::
    DESTINATION share/cmake/xmmt
)
install(DIRECTORY include/xmmt
    DESTINATION include
    PATTERN "*.hpp"
    PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
)
install(FILES "cmake/Findxmmt.cmake" "cmake/xmmt-config.cmake"
    DESTINATION share/cmake/xmmt
)
#####################
# Googletests
#####################

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    find_package(GTest REQUIRED)
    if (${GTest_FOUND})
        add_executable(gtest_xmmt ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_xmmt.cpp)
        target_include_directories(gtest_xmmt SYSTEM PUBLIC ${GTEST_INCLUDE_DIRS})
        target_link_libraries(gtest_xmmt ${GTEST_BOTH_LIBRARIES} emrainey-xmmt)
        add_test(NAME gtest_xmmt COMMAND gtest_xmmt)
    endif()
endif()

#####################
# Build Documentation
#####################

find_package(Doxygen)
if (${Doxygen_FOUND})
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(emrainey-xmmt-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/xmmt
        COMMENT "XMMT Doxygen")
endif()