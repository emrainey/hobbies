cmake_minimum_required(VERSION 3.20.0)
project(neuralnet CXX)

find_package(OpenCV REQUIRED)
find_package(basal REQUIRED)
find_package(linalg REQUIRED)
find_package(linalg-utils REQUIRED)

# Build Everything with C++17 and full native optimizations
set(CMAKE_CXX_STANDARD 17)
enable_testing()

add_library(emrainey-neuralnet SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/source/hidden.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/inner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/layer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/matrix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mnist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/network.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/output.cpp
)
target_link_libraries(emrainey-neuralnet Basal::emrainey-basal Linalg::emrainey-linalg LinalgUtils::emrainey-linalg-utils ${OpenCV_LIBS})
target_include_directories(emrainey-neuralnet
    PUBLIC
       # Generated Tree
       $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
       # Build Tree
       $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
       # Install Tree
       $<INSTALL_INTERFACE:include>
)

#####################
# Demo
#####################

add_executable(demo_neuralnet ${CMAKE_CURRENT_SOURCE_DIR}/demo/neuralnet_counter.cpp)
target_link_libraries(demo_neuralnet emrainey-neuralnet ${OpenCV_LIBS})

add_executable(neuralnet ${CMAKE_CURRENT_SOURCE_DIR}/source/neuralnet.cpp)
target_link_libraries(neuralnet emrainey-neuralnet ${OpenCV_LIBS})

#####################
# Build Documentation
#####################

find_package(Doxygen)
if (${Doxygen_FOUND})
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    doxygen_add_docs(emrainey-neuralnet-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/neuralnet
        COMMENT "Neural Network Doxygen")
endif()


#####################
# EXPORTS
#####################

# Declare installs and exports
install(TARGETS emrainey-neuralnet EXPORT NeuralnetTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
install(EXPORT NeuralnetTargets
    FILE NeuralnetTargets.cmake
    NAMESPACE Neuralnet::
    DESTINATION share/cmake/neuralnet
)
install(DIRECTORY include/neuralnet
    DESTINATION include
    PATTERN "*.hpp"
    PERMISSIONS WORLD_READ GROUP_READ OWNER_READ
)
install(FILES "cmake/Findneuralnet.cmake" "cmake/neuralnet-config.cmake"
    DESTINATION share/cmake/neuralnet
)