message(STATUS ">>> Found Linalg!")
find_path(Linalg_INCLUDE_DIR include/linalg/linalg.hpp)
find_library(Linalg_LIBRARY emrainey-linalg)
mark_as_advanced(Linalg_INCLUDE_DIR Linalg_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Linalg
    REQUIRED_VARS Linalg_LIBRARY Linalg_INCLUDE_DIR
)

if (Linalg_FOUND AND NOT TARGET Linalg::emrainey-linalg)
    add_library(Linalg::emrainey-linalg UNKNOWN IMPORTED)
    set_target_properties(Linalg::emrainey-linalg PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES CXX
        IMPORTED_LOCATION "${Linalg_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Linalg_INCLUDE_DIR}"
    )
endif()