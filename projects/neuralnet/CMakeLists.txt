cmake_minimum_required(VERSION 3.31.0)
project(neuralnet LANGUAGES CXX VERSION 0.7.0)  # This must match the conanfile.py!

find_package(OpenCV REQUIRED)

add_library(hobbies-neuralnet
    ${CMAKE_CURRENT_SOURCE_DIR}/source/hidden.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/inner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/layer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/matrix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mnist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/network.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/output.cpp
)
target_link_libraries(hobbies-neuralnet
    PUBLIC
        hobbies-basal
        hobbies-linalg
        hobbies-linalg-utils
        ${OpenCV_LIBS}
    PRIVATE
        enabled-warnings
        enabled-debugging
        $<$<BOOL:${USE_PROFILING}>:enabled-profiling>
        $<$<BOOL:${USE_COVERAGE}>:enabled-coverage>
        native-optimized
)
target_include_directories(hobbies-neuralnet
    PUBLIC
       # Generated Tree
       $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
       # Build Tree
       $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
       # Install Tree
       $<INSTALL_INTERFACE:include>
)
set_target_properties(hobbies-neuralnet PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

#####################
# Demo
#####################

add_executable(demo_neuralnet ${CMAKE_CURRENT_SOURCE_DIR}/demo/neuralnet_counter.cpp)
target_link_libraries(demo_neuralnet hobbies-neuralnet ${OpenCV_LIBS})

add_executable(neuralnet ${CMAKE_CURRENT_SOURCE_DIR}/source/neuralnet.cpp)
target_link_libraries(neuralnet hobbies-neuralnet ${OpenCV_LIBS})

#####################
# Build Documentation
#####################

find_package(Doxygen)
if (Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(hobbies-neuralnet-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/neuralnet
        COMMENT "Neural Network Doxygen")
endif()


#####################
# EXPORTS
#####################

# Declare installs and exports
# install(TARGETS hobbies-neuralnet EXPORT NeuralnetTargets
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
#     RUNTIME DESTINATION bin
#     INCLUDES DESTINATION include
# )
# install(EXPORT NeuralnetTargets
#     FILE ${PROJECT_NAME}-Targets.cmake
#     NAMESPACE Neuralnet::
#     DESTINATION share/cmake/${PROJECT_NAME}
# )
# install(DIRECTORY include/${PROJECT_NAME}
#     DESTINATION include
#     PATTERN "*.hpp"
#     PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
# )
# install(FILES "cmake/Find${PROJECT_NAME}.cmake" "cmake/${PROJECT_NAME}-config.cmake"
#     DESTINATION share/cmake/${PROJECT_NAME}
# )