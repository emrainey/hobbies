message(STATUS ">>> Found Neuralnet!")
find_path(Neuralnet_INCLUDE_DIR include/noise/noise.hpp)
find_library(Neuralnet_LIBRARY emrainey-neuralnet)
mark_as_advanced(Neuralnet_INCLUDE_DIR Neuralnet_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Neuralnet
    REQUIRED_VARS Neuralnet_LIBRARY Neuralnet_INCLUDE_DIR
)

if (Neuralnet_FOUND AND NOT TARGET Neuralnet::emrainey-neuralnet)
    add_library(Neuralnet::emrainey-neuralnet UNKNOWN IMPORTED)
    set_target_properties(Neuralnet::emrainey-neuralnet PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES CXX
        IMPORTED_LOCATION "${Neuralnet_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Neuralnet_INCLUDE_DIR}"
    )
endif()