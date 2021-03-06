message(STATUS ">>> Found Linalg-Utils!")
find_path(LinalgUtils_INCLUDE_DIR include/linalg/utils.hpp)
find_library(LinalgUtils_LIBRARY hobbies-linalg-utils)
mark_as_advanced(LinalgUtils_INCLUDE_DIR LinalgUtils_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LinalgUtils
    REQUIRED_VARS LinalgUtils_LIBRARY LinalgUtils_INCLUDE_DIR
)

if (Linalg_FOUND AND NOT TARGET LinalgUtils::hobbies-linalg-utils)
    add_library(LinalgUtils::hobbies-linalg-utils UNKNOWN IMPORTED)
    set_target_properties(LinalgUtils::hobbies-linalg-utils PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES CXX
        IMPORTED_LOCATION "${LinalgUtils_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${LinalgUtils_INCLUDE_DIR}"
    )
endif()