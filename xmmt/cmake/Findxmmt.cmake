message(STATUS ">>> Found Xmmt!")
find_path(Xmmt_INCLUDE_DIR include/xmmt/xmmt.hpp)
find_library(Xmmt_LIBRARY emrainey-xmmt)
mark_as_advanced(Xmmt_INCLUDE_DIR Xmmt_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Xmmt
    REQUIRED_VARS Xmmt_LIBRARY Xmmt_INCLUDE_DIR
)

if (Xmmt_FOUND AND NOT TARGET Xmmt::emrainey-xmmt)
    add_library(Xmmt::emrainey-xmmt UNKNOWN IMPORTED)
    set_target_properties(Xmmt::emrainey-xmmt PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES CXX
        IMPORTED_LOCATION "${Xmmt_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Xmmt_INCLUDE_DIR}"
    )
endif()