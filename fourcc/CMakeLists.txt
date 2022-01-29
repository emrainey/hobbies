cmake_minimum_required(VERSION 3.20.0)
project(fourcc
    VERSION 0.7
    LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(GTest REQUIRED)
find_package(basal 0.7 REQUIRED)
enable_testing()

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
add_definitions(-march=native -mtune=native)
else()
message(ERROR "No other native format is yet supported!")
endif()


add_library(emrainey-fourcc SHARED)
target_sources(emrainey-fourcc
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source/image.cpp
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/fourcc/image.hpp
)
target_link_libraries(emrainey-fourcc PUBLIC emrainey-basal)
target_include_directories(emrainey-fourcc
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${basal_INCLUDE_DIRS}
)

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