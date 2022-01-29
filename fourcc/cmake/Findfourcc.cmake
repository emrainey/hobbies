message(STATUS ">>> Found FOURCC!")
find_path(Fourcc_INCLUDE_DIR include/fourcc/fourcc.hpp)
find_library(Fourcc_LIBRARY emrainey-fourcc)
mark_as_advanced(Fourcc_INCLUDE_DIR Fourcc_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Fourcc
    REQUIRED_VARS Fourcc_LIBRARY Fourcc_INCLUDE_DIR
)

if (Fourcc_FOUND AND NOT TARGET Fourcc::emrainey-fourcc)
    add_library(Fourcc::emrainey-fourcc UNKNOWN IMPORTED)
    set_target_properties(Fourcc::emrainey-fourcc PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES CXX
        IMPORTED_LOCATION "${Fourcc_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Fourcc_INCLUDE_DIR}"
    )
endif()