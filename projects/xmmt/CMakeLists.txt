cmake_minimum_required(VERSION 3.20.0)
project(xmmt LANGUAGES CXX VERSION 0.7.0)  # This must match the conanfile.py!

find_package(basal 0.5.0 REQUIRED)

# Build Everything with C++17
set(CMAKE_CXX_STANDARD 17)
enable_testing()

add_library(hobbies-xmmt INTERFACE)
target_include_directories(hobbies-xmmt INTERFACE
    # Generated Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    # Build Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    # Install Tree
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(hobbies-xmmt INTERFACE Basal::hobbies-basal Basal::native-optimized)

#####################
# EXPORTS
#####################

# Declare installs and exports
install(TARGETS hobbies-xmmt EXPORT XmmtTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
install(EXPORT XmmtTargets
    FILE ${PROJECT_NAME}-Targets.cmake
    NAMESPACE Xmmt::
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

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads)
    find_package(GTest)
    if (Threads_FOUND AND GTest_FOUND)
        add_executable(gtest_xmmt
            ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_xmmt.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_xmat.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_pixel.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_point.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_vector.cpp
        )
        target_include_directories(gtest_xmmt SYSTEM PUBLIC ${GTEST_INCLUDE_DIRS})
        target_link_libraries(gtest_xmmt PRIVATE ${GTEST_BOTH_LIBRARIES} hobbies-xmmt Basal::enabled-debugging Threads::Threads)
        add_test(NAME gtest_xmmt COMMAND gtest_xmmt)
    endif()
endif()

#####################
# Build Documentation
#####################

find_package(Doxygen)
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-xmmt-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/xmmt
        COMMENT "XMMT Doxygen")
endif()