#!/bin/bash
set -x
set -e

CLEAN=0
JOBS=16
VERBOSE=OFF
RUN_TESTS=0
USE_CONAN=0
USE_SHARED_LIBS=OFF
while [[ $# -gt 0 ]]; do
    case $1 in
        -rm)
            CLEAN=1
        ;;
        -s)
            JOBS=1
        ;;
        -v)
            VERBOSE=ON
        ;;
        -t)
            RUN_TESTS=1
        ;;
        -c)
            USE_CONAN=1
        ;;
        -dy)
            USE_SHARED_LIBS=ON
            ;;
        *)
        ;;
    esac
    shift
done

# The local install location to test the build
INSTALL_ROOT=`pwd`/install
if [[ ${CLEAN} -eq 1 ]]; then
    rm -rf ${INSTALL_ROOT}
fi
mkdir -p ${INSTALL_ROOT}

# Which packages to build in which order
PKGS=(basal units_of_measure fourcc linalg geometry linalg-utils \
    linalg-algo xmmt noise raytrace neuralnet htm doorgame)

# Cycle over each package and build it
for pkg in "${PKGS[@]}"; do
    if [[ ${USE_CONAN} -eq 0 ]]; then
        if [[ ${CLEAN} -eq 1 ]]; then
            rm -rf $pkg/build
        fi
        mkdir -p $pkg/build
        cmake -B $pkg/build -S $pkg \
            -DBUILD_SHARED_LIBS=${USE_SHARED_LIBS} \
            -DCMAKE_INSTALL_PREFIX:PATH=${INSTALL_ROOT} \
            -DCMAKE_PREFIX_PATH:PATH=${INSTALL_ROOT} \
            -DCMAKE_VERBOSE_MAKEFILE:BOOL=${VERBOSE} \
            -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
            -DCMAKE_BUILD_TYPE:STRING=Release
            #-DCMAKE_C_COMPILER:FILEPATH=/usr/bin/clang \
            #-DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/clang++
        cmake --build $pkg/build -j${JOBS}
        if [[ ${RUN_TESTS} -eq 1 ]]; then
            cmake --build $pkg/build --target test
        fi
        cmake --build $pkg/build --target install
    else
        if [[ ${CLEAN} -eq 1 ]]; then
            conan remove --force $pkg
            rm -rf .conan/$pkg
        fi

        build_path=.conan/$pkg/build
        package_path=.conan/$pkg/package

        mkdir -p ${install_path} ${build_path} ${package_path}

        shared=""
        if [[ ${USE_SHARED_LIBS} == ON ]] && [[ ! ${pkg} == "xmmt" ]]; then
            shared="--options shared=True"
        fi

        # this installs the dependencies into a local folder
        conan install $pkg -if ${build_path} ${shared}
        # this runs CMake config
        conan build $pkg --configure -bf ${build_path}
        # this actually builds
        conan build $pkg --build     -bf ${build_path}
        if [[ ${RUN_TESTS} -eq 1 ]]; then
            # this runs tests
            conan build $pkg --test  -bf ${build_path}
        fi
        # this installs from the build folder into the packageing folder
        conan build $pkg --install   -bf ${build_path} -pf ${package_path}
        # this creates a conan package in the package folder
        conan package $pkg -bf ${build_path} -pf ${package_path}
        # conan export $pkg
        # this exports the package into the local conan cache
        conan export-pkg $pkg --force -pf ${package_path} --profile default
    fi
done