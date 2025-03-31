cmake_minimum_required(VERSION 3.31.0)
project(audio LANGUAGES CXX VERSION 0.7.0)  # This must match the conanfile.py!

add_library(hobbies-audio
    ${CMAKE_CURRENT_SOURCE_DIR}/source/audio.cpp
)
target_include_directories(hobbies-audio
    PUBLIC
        # Generated Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        # Build Tree
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # Install Tree
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(hobbies-audio
    PUBLIC
        hobbies-basal
        hobbies-uom
    PRIVATE
        enabled-warnings
        enabled-debugging
        $<$<BOOL:${USE_PROFILING}>:enabled-profiling>
        $<$<BOOL:${USE_COVERAGE}>:enabled-coverage>
        native-optimized
)
set_target_properties(hobbies-audio PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)


#####################
# EXPORTS
#####################

# Declare installs and exports
# install(TARGETS hobbies-audio EXPORT FourccTargets
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
#     RUNTIME DESTINATION bin
#     INCLUDES DESTINATION include
# )
# install(EXPORT FourccTargets
#     FILE ${PROJECT_NAME}-Targets.cmake
#     NAMESPACE audio::
#     DESTINATION share/cmake/${PROJECT_NAME}
#     PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
# )
# install(DIRECTORY include/${PROJECT_NAME}
#     DESTINATION include
#     PATTERN "*.hpp"
#     PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
# )
# install(FILES "cmake/Find${PROJECT_NAME}.cmake" "cmake/${PROJECT_NAME}-config.cmake"
#     DESTINATION share/cmake/${PROJECT_NAME}
#     PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
# )
#####################
# Unit Tests
#####################

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
find_package(GTest)
if (Threads_FOUND AND GTest_FOUND)
    add_executable(gtest_audio
        ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_audio.cpp
    )
    target_link_libraries(gtest_audio PRIVATE hobbies-audio enabled-debugging GTest::gtest GTest::gtest_main Threads::Threads)
    add_test(NAME gtest_audio COMMAND gtest_audio)
endif()

#####################
# Build Documentation
#####################

find_package(Doxygen)
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-audio-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/audio
        COMMENT "audio Doxygen")
endif()