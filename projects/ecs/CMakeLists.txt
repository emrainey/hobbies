# === Project ===
cmake_minimum_required(VERSION 3.31.0)
project(ecs LANGUAGES CXX VERSION 0.1.0)  # This must match the conanfile.py!

# === Dependencies ===
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(SDL2)
find_package(Doxygen)

# === Targets ===
# add_library(hobbies-ecs
#     ${CMAKE_CURRENT_SOURCE_DIR}/source/.cpp
# )
# target_link_libraries(hobbies-ecs
#     PUBLIC
#         hobbies-basal
#     PRIVATE
#         enabled-debugging
#         $<$<BOOL:${USE_PROFILING}>:enabled-profiling>
#         $<$<BOOL:${USE_COVERAGE}>:enabled-coverage>
#         enabled-warnings
#         native-optimized
# )
# target_include_directories(hobbies-ecs
#     PUBLIC
#         # Generated Tree
#         $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
#         # Build Tree
#         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#         # Install Tree
#         $<INSTALL_INTERFACE:include>
# )
# set_target_properties(hobbies-ecs PROPERTIES
#     VERSION ${PROJECT_VERSION}
#     SOVERSION ${PROJECT_VERSION_MAJOR}
#     POSITION_INDEPENDENT_CODE ON
# )

# === Demos ===
if (SDL2_FOUND)
add_executable(demo_ecs ${CMAKE_CURRENT_SOURCE_DIR}/demo/main.cpp)
target_link_libraries(demo_ecs PUBLIC ${SDL2_LIBRARIES}) # hobbies-ecs
endif(SDL2_FOUND)

# === Doxygen ===
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-ecs-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ecs
        COMMENT "ECS Doxygen")
endif()