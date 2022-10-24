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

PROJECT_ROOT=`pwd`/projects
BUILD_ROOT=`pwd`/build
PACKAGE_ROOT=`pwd`/package
INSTALL_ROOT=`pwd`/install

if [[ ${CLEAN} -eq 1 ]]; then
    rm -rf ${INSTALL_ROOT}
fi
mkdir -p ${INSTALL_ROOT}
export CONAN_TRACE_FILE=${INSTALL_ROOT}/conan_trace.log

# Which packages to build in which order
if [[ -z ${PKGS[@]} ]]; then
PKGS=(basal units_of_measure fourcc linalg geometry linalg-utils \
    linalg-algo xmmt noise raytrace neuralnet htm doorgame pyhobbies)
fi

# Cycle over each package and build it
for pkg in "${PKGS[@]}"; do

    project_path=${PROJECT_ROOT}/$pkg
    build_path=${BUILD_ROOT}/$pkg
    install_path=${INSTALL_ROOT}
    package_path=${PACKAGE_ROOT}/$pkg

    if [[ ${CLEAN} -eq 1 ]]; then
        rm -rf ${build_path}
        rm -rf ${package_path}
        # can't delete project_path
    fi
    mkdir -p ${build_path} ${install_path} ${package_path}

    if [[ ${USE_CONAN} -eq 0 ]]; then
        cmake -B ${build_path} -S ${project_path} \
            -DBUILD_SHARED_LIBS=${USE_SHARED_LIBS} \
            -DCMAKE_INSTALL_PREFIX:PATH=${INSTALL_ROOT} \
            -DCMAKE_PREFIX_PATH:PATH=${INSTALL_ROOT} \
            -DCMAKE_VERBOSE_MAKEFILE:BOOL=${VERBOSE} \
            -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
            -DCMAKE_BUILD_TYPE:STRING=Release
        cmake --build ${build_path} -j${JOBS} --target all
        if [[ ${RUN_TESTS} -eq 1 ]]; then
            cmake --build ${build_path} --target test
        fi
        cmake --build ${build_path} --target install
    else
        if [[ ${CLEAN} -eq 1 ]]; then
            (cd ${PROJECT_ROOT} && conan remove --force $pkg)
        fi
        shared=""
        if [[ ${USE_SHARED_LIBS} == ON ]] && [[ ! ${pkg} == "xmmt" ]]; then
            shared="--options shared=True"
        fi
        # this installs the dependencies into a local folder
        conan install ${project_path} -if ${build_path} ${shared}
        # this runs CMake config
        conan build ${project_path} --configure -bf ${build_path}
        # this actually builds
        conan build ${project_path} --build     -bf ${build_path}
        if [[ ${RUN_TESTS} -eq 1 ]]; then
            # this runs tests
            conan build ${project_path} --test  -bf ${build_path}
        fi
        # special CMake step to install into a common folder
        cmake --install ${build_path} --prefix ${install_path}
        # this installs from the build folder into the packageing folder
        conan build ${project_path} --install   -bf ${build_path} -pf ${package_path}
        # this creates a conan package in the package folder
        conan package ${project_path} -bf ${build_path} -pf ${package_path}
        # conan export ${project_path}
        # this exports the package into the local conan cache
        conan export-pkg ${project_path} --force -pf ${package_path} --profile default
    fi
done

# Install the Hobbies Python Module via CMake build to the User's Python Install
CMAKE_PREFIX_PATH=${INSTALL_ROOT} CMAKE_INSTALL_PREFIX=${INSTALL_ROOT} python3 -m pip install --user -e projects/pyhobbies
