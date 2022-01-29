cmake_minimum_required(VERSION 3.20.0)
project(fourcc
    VERSION 0.7
    LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(GTest REQUIRED)
find_package(basal REQUIRED)
enable_testing()

#print_package_variables(basal)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
add_definitions(-march=native -mtune=native)
else()
message(ERROR "No other native format is yet supported!")
endif()


add_library(emrainey-fourcc SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/source/image.cpp
)
target_include_directories(emrainey-fourcc
    PUBLIC
        # Generated Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        # Build Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # Install Tree
        $<INSTALL_INTERFACE:include>
        ${basal_INCLUDE_DIRS}
)
target_link_libraries(emrainey-fourcc PUBLIC Basal::emrainey-basal)

#####################
# EXPORTS
#####################

# Declare installs and exports (this isn't working yet)
install(TARGETS emrainey-fourcc EXPORT FourccTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
install(EXPORT FourccTargets
    FILE FourccTargets.cmake
    NAMESPACE Fourcc::
    DESTINATION share/cmake/fourcc
)
install(DIRECTORY include/fourcc
    DESTINATION include
    PATTERN "*.hpp"
    PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
)
install(FILES "cmake/Findfourcc.cmake" "cmake/fourccConfig.cmake"
    DESTINATION share/cmake/fourcc
)
#####################
# Unit Tests
#####################
if (${GTest_FOUND})
    add_executable(gtest_fourcc
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_fourcc.cpp
    )
    target_include_directories(gtest_fourcc SYSTEM PUBLIC ${GTEST_INCLUDE_DIRS})
    target_link_libraries(gtest_fourcc ${GTEST_BOTH_LIBRARIES} emrainey-fourcc)
    add_test(NAME gtest_fourcc COMMAND gtest_fourcc)
endif()

#####################
# Build Documentation
#####################

find_package(Doxygen REQUIRED)
if (${Doxygen_FOUND})
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(emrainey-fourcc-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/fourcc
        COMMENT "Fourcc Doxygen")
endif()