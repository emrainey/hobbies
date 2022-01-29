cmake_minimum_required(VERSION 3.20.0)
project(basal CXX)
set(CMAKE_CXX_STANDARD 17)

add_library(emrainey-basal SHARED)
target_sources(emrainey-basal
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source/debuggable.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/exception.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/listable.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/basal.cpp
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/basal/debuggable.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/basal/exception.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/basal/ieee754.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/basal/listable.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/basal/options.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/basal/basal.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/basal/printable.hpp
)
target_include_directories(emrainey-basal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

install(TARGETS emrainey-basal DESTINATION lib)
install(DIRECTORY include/basal DESTINATION include)

# Unit Tests
find_package(GTest REQUIRED)
if (${GTest_FOUND})
    add_executable(test_basal
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_basal.cpp
    )
    target_include_directories(test_basal PRIVATE ${GTEST_INCLUDE_DIRS})
    target_link_libraries(test_basal emrainey-basal ${GTEST_BOTH_LIBRARIES})
    add_test(NAME test_basal COMMAND test_basal)
endif()

# Documentation
find_package(Doxygen REQUIRED)
set(DOXYGEN_GENERATE_HTML YES)
set(DOXYGEN_GENERATE_MAN NO)
doxygen_add_docs(emrainey-basal-docs
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    COMMENT "Basal Doxygen")
