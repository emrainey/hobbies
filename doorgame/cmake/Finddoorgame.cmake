message(STATUS ">>> Found DoorGame!")
find_path(DG_INCLUDE_DIR include/doorgame/doorgame.hpp)
find_library(DG_LIBRARY hobbies-doorgame)
mark_as_advanced(DG_INCLUDE_DIR DG_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DG
    REQUIRED_VARS DG_LIBRARY DG_INCLUDE_DIR
)

if (DG_FOUND AND NOT TARGET DoorGame::hobbies-doorgame)
    add_library(DoorGame::hobbies-doorgame UNKNOWN IMPORTED)
    set_target_properties(DoorGame::hobbies-doorgame PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES CXX
        IMPORTED_LOCATION "${DG_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${DG_INCLUDE_DIR}"
    )
endif()