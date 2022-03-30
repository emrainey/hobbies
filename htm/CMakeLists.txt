cmake_minimum_required(VERSION 3.13)
project(htm CXX)

find_package(fourcc REQUIRED)

# Build Everything with C++17 and full native optimizations
set(CMAKE_CXX_STANDARD 17)
enable_testing()

option(BUILD_SHARED_LIBS "Builds libraries as dynamic objects" OFF)

add_library(hobbies-htm
    ${CMAKE_CURRENT_SOURCE_DIR}/source/encoders.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/frand.cpp
)
# target_link_libraries(hobbies-htm PUBLIC)
target_include_directories(hobbies-htm
    PUBLIC
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
install(TARGETS hobbies-htm EXPORT HTMTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
install(EXPORT HTMTargets
    FILE HTMTargets.cmake
    NAMESPACE HTM::
    DESTINATION share/cmake/htm
)
install(DIRECTORY include/htm
    DESTINATION include
    PATTERN "*.hpp"
    PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
)
install(FILES "cmake/Findhtm.cmake" "cmake/htm-config.cmake"
    DESTINATION share/cmake/htm
)
################
# DEMOS
################

add_executable(demo_sparse_word ${CMAKE_CURRENT_SOURCE_DIR}/demo/sparse_word.cpp)
target_link_libraries(demo_sparse_word hobbies-htm)

#####################
# Build Documentation
#####################
find_package(Doxygen)
if (${Doxygen_FOUND})
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-htm-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/htm
        COMMENT "HTM Doxygen")
endif()