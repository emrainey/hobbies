message(STATUS ">>> Found Geometry!")
find_path(Geometry_INCLUDE_DIR include/geometry/geometry.hpp)
find_library(Geometry_LIBRARY emrainey-geometry)
mark_as_advanced(Geometry_INCLUDE_DIR Geometry_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Geometry
    REQUIRED_VARS Geometry_LIBRARY Geometry_INCLUDE_DIR
)

if (Geometry_FOUND AND NOT TARGET Geometry::emrainey-geometry)
    add_library(Geometry::emrainey-geometry UNKNOWN IMPORTED)
    set_target_properties(Geometry::emrainey-geometry PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES CXX
        IMPORTED_LOCATION "${Geometry_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Geometry_INCLUDE_DIR}"
    )
endif()