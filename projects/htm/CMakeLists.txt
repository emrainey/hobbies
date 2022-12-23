cmake_minimum_required(VERSION 3.20.0)
project(htm LANGUAGES CXX VERSION 0.1.0)  # This must match the conanfile.py!

find_package(fourcc REQUIRED)

# Build Everything with C++17 and full native optimizations
set(CMAKE_CXX_STANDARD 17)
enable_testing()

option(BUILD_SHARED_LIBS "Builds libraries as dynamic objects" OFF)

add_library(hobbies-htm
    ${CMAKE_CURRENT_SOURCE_DIR}/source/encoders.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/frand.cpp
)
target_link_libraries(hobbies-htm
    PRIVATE
        Basal::enabled-debugging
        Basal::enabled-profiling
        Basal::enabled-warnings
        Basal::native-optimized
)
target_include_directories(hobbies-htm
    PUBLIC
        # Generated Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        # Build Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # Install Tree
        $<INSTALL_INTERFACE:include>
)
set_target_properties(hobbies-htm PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
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