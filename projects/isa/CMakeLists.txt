# === Project ===
cmake_minimum_required(VERSION 3.31.0)
project(ecs LANGUAGES CXX VERSION 0.1.0)  # This must match the conanfile.py!

# === Dependencies ===
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(SDL2)
find_package(Doxygen)
find_package(Threads)
find_package(GTest)

# === Targets ===
add_library(hobbies-isa
    ${CMAKE_CURRENT_SOURCE_DIR}/source/bus.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/cpu.cpp
)
target_link_libraries(hobbies-isa
    PUBLIC
        hobbies-basal
    PRIVATE
        enabled-debugging
        $<$<BOOL:${USE_PROFILING}>:enabled-profiling>
        $<$<BOOL:${USE_COVERAGE}>:enabled-coverage>
        enabled-warnings
        native-optimized
)
target_include_directories(hobbies-isa
    PUBLIC
        # Generated Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        # Build Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # Install Tree
        $<INSTALL_INTERFACE:include>
)
set_target_properties(hobbies-isa PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

add_library(hobbies-isa-demo
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/demo_memory_view.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/demo_register_view.cpp
)
target_link_libraries(hobbies-isa-demo
    PUBLIC
        hobbies-isa
    PRIVATE
        enabled-debugging
        $<$<BOOL:${USE_PROFILING}>:enabled-profiling>
        $<$<BOOL:${USE_COVERAGE}>:enabled-coverage>
        enabled-warnings
        native-optimized
)
target_include_directories(hobbies-isa-demo
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/demo>
        $<INSTALL_INTERFACE:include/isa/demo>
)
set_target_properties(hobbies-isa-demo PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS hobbies-isa hobbies-isa-demo
    EXPORT isa-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/isa/
    DESTINATION include/isa
    FILES_MATCHING PATTERN "*.hpp"
)
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/demo_memory_view.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/demo_register_view.hpp
    DESTINATION include/isa/demo
)

# === Demos ===
if (SDL2_FOUND)
add_executable(demo_isa
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/main.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/demo/program1.cpp
)
target_link_libraries(demo_isa PUBLIC ${SDL2_LIBRARIES} hobbies-isa-demo ftxui::screen ftxui::dom ftxui::component)
install(TARGETS demo_isa
    EXPORT isa-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
endif(SDL2_FOUND)

# === Unit Tests ===
if (BUILD_UNIT_TESTS AND Threads_FOUND AND GTest_FOUND)
    add_executable(gtest_isa
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_bus.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_breakpoint.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_cpu_persistence.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_memory_view.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_register_view.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_cache.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_instruction.cpp
    )
    target_link_libraries(gtest_isa PRIVATE hobbies-isa-demo enabled-debugging GTest::gtest GTest::gmock GTest::gtest_main Threads::Threads)
    add_test(NAME gtest_isa COMMAND gtest_isa)
endif()

# === Doxygen ===
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-isa-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/isa
        COMMENT "ISA Doxygen")
endif()