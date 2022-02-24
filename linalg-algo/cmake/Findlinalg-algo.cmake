message(STATUS ">>> Found Linalg-Utils!")
find_path(LinalgAlgo_INCLUDE_DIR include/linalg/utils.hpp)
find_library(LinalgAlgo_LIBRARY hobbies-linalg-algo)
mark_as_advanced(LinalgAlgo_INCLUDE_DIR LinalgAlgo_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LinalgAlgo
    REQUIRED_VARS LinalgAlgo_LIBRARY LinalgAlgo_INCLUDE_DIR
)

if (Linalg_FOUND AND NOT TARGET LinalgAlgo::hobbies-linalg-algo)
    add_library(LinalgAlgo::hobbies-linalg-algo UNKNOWN IMPORTED)
    set_target_properties(LinalgAlgo::hobbies-linalg-algo PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES CXX
        IMPORTED_LOCATION "${LinalgAlgo_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${LinalgAlgo_INCLUDE_DIR}"
    )
endif()