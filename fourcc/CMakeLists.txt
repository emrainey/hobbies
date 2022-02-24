cmake_minimum_required(VERSION 3.20.0)
project(fourcc CXX)

find_package(basal REQUIRED)

# Build Everything with C++17
set(CMAKE_CXX_STANDARD 17)
enable_testing()

add_library(hobbies-fourcc SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/source/image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/convolve.cpp
)
target_include_directories(hobbies-fourcc
    PUBLIC
        # Generated Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        # Build Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # Install Tree
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(hobbies-fourcc PUBLIC Basal::hobbies-basal Basal::native-optimized)

#####################
# EXPORTS
#####################

# Declare installs and exports
install(TARGETS hobbies-fourcc EXPORT FourccTargets
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
install(FILES "cmake/Findfourcc.cmake" "cmake/fourcc-config.cmake"
    DESTINATION share/cmake/fourcc
)
#####################
# Unit Tests
#####################

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(GTest)
if (GTest_FOUND)
    add_executable(gtest_fourcc
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_fourcc.cpp
    )
    target_include_directories(gtest_fourcc SYSTEM PUBLIC ${GTEST_INCLUDE_DIRS} ${GTest_INCLUDE_DIRS})
    target_link_libraries(gtest_fourcc PRIVATE hobbies-fourcc ${GTEST_BOTH_LIBRARIES} ${GTest_LIBS} Threads::Threads)
    add_test(NAME gtest_fourcc COMMAND gtest_fourcc)
endif()

#####################
# Build Documentation
#####################

find_package(Doxygen)
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-fourcc-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/fourcc
        COMMENT "Fourcc Doxygen")
endif()