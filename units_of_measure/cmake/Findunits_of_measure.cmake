message(STATUS ">>> Found Units of Measure!")
find_path(UnitsOfMeasure_INCLUDE_DIR include/iso/iso.hpp)
find_library(UnitsOfMeasure_LIBRARY hobbies-uom)
mark_as_advanced(UnitsOfMeasure_INCLUDE_DIR UnitsOfMeasure_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(UnitsOfMeasure
    REQUIRED_VARS UnitsOfMeasure_LIBRARY UnitsOfMeasure_INCLUDE_DIR
)

if (UnitsOfMeasure_FOUND AND NOT TARGET UnitsOfMeasure::hobbies-uom)
    add_library(UnitsOfMeasure::hobbies-uom UNKNOWN IMPORTED)
    set_target_properties(UnitsOfMeasure::hobbies-uom PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES CXX
        IMPORTED_LOCATION "${UnitsOfMeasure_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${UnitsOfMeasure_INCLUDE_DIR}"
    )
endif()