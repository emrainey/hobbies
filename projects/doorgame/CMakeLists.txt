# === Project ===
cmake_minimum_required(VERSION 3.31.0)
project(doorgame LANGUAGES CXX VERSION 0.1.0)  # This must match the conanfile.py!

# === Dependencies ===
find_package(Curses)
find_package(Doxygen)

# === Targets ===
add_library(hobbies-doorgame
    ${CMAKE_CURRENT_SOURCE_DIR}/source/animate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/actions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/damages.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/directions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/game.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/health.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/items.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/map.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/monster.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/parameters.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/player.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/room.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/storage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/targets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/streamview.cpp
)
target_link_libraries(hobbies-doorgame
    PUBLIC
        hobbies-basal
    PRIVATE
        enabled-debugging
        $<$<BOOL:${USE_PROFILING}>:enabled-profiling>
        $<$<BOOL:${USE_COVERAGE}>:enabled-coverage>
        enabled-warnings
        native-optimized
)
target_include_directories(hobbies-doorgame
    PUBLIC
        # Generated Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        # Build Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # Install Tree
        $<INSTALL_INTERFACE:include>
)
set_target_properties(hobbies-doorgame PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)
install(TARGETS hobbies-doorgame
    EXPORT doorgame-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/doorgame
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

# === Demos ===
if (Curses_FOUND)
add_executable(demo_doorgame ${CMAKE_CURRENT_SOURCE_DIR}/demo/main.cpp)
target_link_libraries(demo_doorgame PUBLIC hobbies-doorgame  ${CURSES_LIBRARIES})
target_compile_definitions(demo_doorgame PUBLIC ${CURSES_CFLAGS})
install(TARGETS hobbies-doorgame
    EXPORT doorgame-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
endif(Curses_FOUND)

# === Doxygen ===
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-doorgame-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/doorgame
        COMMENT "DoorGame Doxygen")
endif()