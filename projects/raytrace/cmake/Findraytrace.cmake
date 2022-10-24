message(STATUS ">>> Found Raytrace!")
find_path(Raytrace_INCLUDE_DIR include/noise/noise.hpp)
find_library(Raytrace_LIBRARY hobbies-raytrace)
mark_as_advanced(Raytrace_INCLUDE_DIR Raytrace_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Raytrace
    REQUIRED_VARS Raytrace_LIBRARY Raytrace_INCLUDE_DIR
)

if (Raytrace_FOUND AND NOT TARGET Raytrace::hobbies-raytrace)
    add_library(Raytrace::hobbies-raytrace UNKNOWN IMPORTED)
    set_target_properties(Raytrace::hobbies-raytrace PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES CXX
        IMPORTED_LOCATION "${Raytrace_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Raytrace_INCLUDE_DIR}"
    )
endif()