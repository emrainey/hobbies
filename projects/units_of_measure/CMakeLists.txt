# === Project ===
cmake_minimum_required(VERSION 3.31.0)
project(units_of_measure LANGUAGES CXX VERSION 0.7.0)  # This must match the conanfile.py!

# === Dependencies ===
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
find_package(GTest)
find_package(Doxygen
#    REQUIRED dot
#    OPTIONAL_COMPONENTS mscgen dia
)

# === Targets ===
add_library(hobbies-uom
    # IEC
    "source/iec/prefix.cpp"
    # ISO
    "source/iso/operators.cpp"
    "source/iso/convert.cpp"
    "source/iso/electrical.cpp"
    "source/iso/iso.cpp"
    ## Units
    "source/iso/amperes.cpp"
    "source/iso/degrees.cpp"
    "source/iso/grams.cpp"
    "source/iso/hertz.cpp"
    "source/iso/joules.cpp"
    "source/iso/meters.cpp"
    "source/iso/newtons.cpp"
    "source/iso/ohms.cpp"
    "source/iso/pascals.cpp"
    "source/iso/radians.cpp"
    "source/iso/seconds.cpp"
    "source/iso/turns.cpp"
    "source/iso/volts.cpp"
    "source/iso/watts.cpp"
    # SI
    "source/si/prefix.cpp"
)
target_link_libraries(hobbies-uom
    PRIVATE
        enabled-warnings
        enabled-debugging
        $<$<BOOL:${USE_PROFILING}>:enabled-profiling>
        $<$<BOOL:${USE_COVERAGE}>:enabled-coverage>
        native-optimized
    PUBLIC
        hobbies-basal
)
target_include_directories(hobbies-uom PUBLIC
    # Generated Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    # Source Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    # Install Tree
    $<INSTALL_INTERFACE:include>
)
set_target_properties(hobbies-uom PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)
install(TARGETS hobbies-uom
    EXPORT uom-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/units_of_measure
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

# === Unit Testing ===
if (BUILD_UNIT_TESTS AND Threads_FOUND AND GTest_FOUND)
    add_executable(gtest_iso ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_iso.cpp)
    # target_include_directories(gtest_iso PUBLIC ${GTEST_INCLUDE_DIRS} ${GTest_INCLUDE_DIRS})
    target_link_libraries(gtest_iso PRIVATE hobbies-uom GTest::gtest GTest::gmock GTest::gtest_main Threads::Threads)
    add_test(NAME gtest_iso COMMAND gtest_iso)
endif()

# === Doxygen ===
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-uom-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        COMMENT "Units of Measure Doxygen")
endif()
