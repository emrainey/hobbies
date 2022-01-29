find_path(Basal_INCLUDE_DIR include/basal/basal.hpp)
find_library(Basal_LIBRARY emrainey-basal)
mark_as_advanced(Basal_INCLUDE_DIR Basal_LIBRARY

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Basal
    REQUIRED_VARS Basal_LIBRARY Basal_INCLUDE_DIR
)

if (Basal_FOUND AND NOT TARGET Basal::emrainey-basal)
    add_library(Basal::emrainey-basal UNKNOWN IMPORTED)
    set_target_properties(Basal::emrainey-basal PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES CXX
        IMPORTED_LOCATION "${Basal_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Basal_INCLUDE_DIR}"
    )
endif()