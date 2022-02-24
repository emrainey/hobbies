message(STATUS ">>> Found HTM!")
find_path(HTM_INCLUDE_DIR include/htm/htm.hpp)
find_library(HTM_LIBRARY hobbies-htm)
mark_as_advanced(HTM_INCLUDE_DIR HTM_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HTM
    REQUIRED_VARS HTM_LIBRARY HTM_INCLUDE_DIR
)

if (HTM_FOUND AND NOT TARGET HTM::hobbies-htm)
    add_library(HTM::hobbies-htm UNKNOWN IMPORTED)
    set_target_properties(HTM::hobbies-htm PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES CXX
        IMPORTED_LOCATION "${HTM_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${HTM_INCLUDE_DIR}"
    )
endif()