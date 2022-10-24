message(STATUS ">>> Found Noise!")
find_path(Noise_INCLUDE_DIR include/noise/noise.hpp)
find_library(Noise_LIBRARY hobbies-noise)
mark_as_advanced(Noise_INCLUDE_DIR Noise_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Noise
    REQUIRED_VARS Noise_LIBRARY Noise_INCLUDE_DIR
)

if (Noise_FOUND AND NOT TARGET Noise::hobbies-noise)
    add_library(Noise::hobbies-noise UNKNOWN IMPORTED)
    set_target_properties(Noise::hobbies-noise PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES CXX
        IMPORTED_LOCATION "${Noise_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Noise_INCLUDE_DIR}"
    )
endif()