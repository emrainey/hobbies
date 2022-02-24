message(FATAL_ERROR ">>> Found Basal!")
find_path(Basal_INCLUDE_DIR include/basal/basal.hpp)
find_library(Basal_LIBRARY hobbies-basal)
mark_as_advanced(Basal_INCLUDE_DIR Basal_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Basal
    REQUIRED_VARS Basal_LIBRARY Basal_INCLUDE_DIR
)

if (Basal_FOUND AND NOT TARGET Basal::hobbies-basal)
    add_library(Basal::hobbies-basal UNKNOWN IMPORTED)
    set_target_properties(Basal::hobbies-basal PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES CXX
        IMPORTED_LOCATION "${Basal_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Basal_INCLUDE_DIR}"
    )
endif()