cmake_minimum_required(VERSION 3.20.0)
project(units_of_measure
    VERSION 0.7
    LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_package(basal REQUIRED)
enable_testing()

set(UOM_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

# The generic interface which all of these generated libraries will depend on
add_library(uom-generator INTERFACE)
target_include_directories(uom-generator INTERFACE
    # Generated Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    # Source Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    # Install Tree
    $<INSTALL_INTERFACE:include>
)

# The library that all the generated files will be made a dependency of
add_library(uom-generated INTERFACE)

# Generates cpp and hpp files from jinja templates.
#
function(generate_source_code NAMESPACE CLASSNAME TYPENAME SCALETYPE SCALEVALUE POSTFIX)
    add_custom_command(
        MAIN_DEPENDENCY
            ${CMAKE_CURRENT_SOURCE_DIR}/templates/header.hpp.jinja
            ${CMAKE_CURRENT_SOURCE_DIR}/templates/body.cpp.jinja
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/scripts/process_templates.py
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/templates/header.hpp.jinja header.hpp.jinja
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/templates/body.cpp.jinja body.cpp.jinja
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/scripts/process_templates.py process_templates.py
        COMMAND ./process_templates.py ARGS --namespace ${NAMESPACE} --class ${CLASSNAME} --typename ${TYPENAME} --scale ${SCALEVALUE} --scaletype ${SCALETYPE} --postfix ${POSTFIX} --infile header.hpp.jinja --outfile ${UOM_GEN_DIR}/${NAMESPACE}/${CLASSNAME}.hpp
        COMMAND ./process_templates.py ARGS --namespace ${NAMESPACE} --class ${CLASSNAME} --typename ${TYPENAME} --scale ${SCALEVALUE} --scaletype ${SCALETYPE} --postfix ${POSTFIX} --infile body.cpp.jinja --outfile ${UOM_GEN_DIR}/${CLASSNAME}.cpp
        COMMENT "Generating the Jinja Templates for ${CLASSNAME}"
        OUTPUT ${UOM_GEN_DIR}/${NAMESPACE}/${CLASSNAME}.hpp ${UOM_GEN_DIR}/${CLASSNAME}.cpp
        VERBATIM
    )
    add_library(uom-${CLASSNAME} STATIC ${UOM_GEN_DIR}/${CLASSNAME}.cpp)
    target_link_libraries(uom-${CLASSNAME} uom-generator)

    list(APPEND UOM_LIBS uom-${CLASSNAME})
    set(UOM_LIBS ${UOM_LIBS} PARENT_SCOPE)

endfunction(generate_source_code)

# Examples of how to generate code using this script
generate_source_code(iso furlong long SI::prefix iso fl)
generate_source_code(iso microfortnight float SI::prefix micro uftnt)

add_library(emrainey-uom SHARED)
file(GLOB private_sources
    "source/iec/prefix.cpp"
    "source/iso/*.cpp"
    "source/si/*.cpp"
)
file(GLOB public_headers
    "include/iec/*.hpp"
    "include/iso/*.hpp"
    "include/si/*.hpp"
)
target_sources(emrainey-uom
    PRIVATE
        ${private_sources}
)
message(STATUS "List of UOM Libs: ${UOM_LIBS}")
target_link_libraries(emrainey-uom uom-generator ${UOM_LIBS})
target_include_directories(emrainey-uom PUBLIC
    # Generated Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    # Source Tree
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    # Install Tree
    $<INSTALL_INTERFACE:include>
    ${Basal_INCLUDE_DIRS}
)

install(TARGETS emrainey-uom DESTINATION lib)
install(DIRECTORY include/iec DESTINATION include)
install(DIRECTORY include/iso DESTINATION include)
install(DIRECTORY include/si DESTINATION include)

# Declare installs and exports (this isn't working yet)
install(TARGETS emrainey-uom uom-generator uom-furlong uom-microfortnight EXPORT UnitsOfMeasureTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
install(EXPORT UnitsOfMeasureTargets
    FILE UnitsOfMeasureTargets.cmake
    NAMESPACE UnitsOfMeasure::
    DESTINATION share/cmake/units_of_measure
)
install(FILES "cmake/Findunits_of_measure.cmake" "cmake/units_of_measure-config.cmake"
    DESTINATION share/cmake/units_of_measure
)
#####################################################
# Unit Testing
#####################################################
find_package(GTest REQUIRED)
if (${GTest_FOUND})
    add_executable(gtest_iso ${CMAKE_CURRENT_SOURCE_DIR}/test/gtest_iso.cpp)
    target_include_directories(gtest_iso PUBLIC ${GTEST_INCLUDE_DIRS})
    target_link_libraries(gtest_iso ${GTEST_BOTH_LIBRARIES} emrainey-uom)
    add_test(NAME gtest_iso COMMAND gtest_iso)
endif()

#####################################################
# Documentation
#####################################################
find_package(Doxygen
#    REQUIRED dot
#    OPTIONAL_COMPONENTS mscgen dia
)
if (${Doxygen_FOUND})
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)

    doxygen_add_docs(emrainey-uom-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        COMMENT "Units of Measure Doxygen")
endif()
