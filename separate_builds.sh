#!/bin/zsh
set -x
set -e

# Which packages to build and their versions (must keep this manually synced with conanfile.py's)
declare -A PKGS
PKGS=(["basal"]="0.5.0" \
      ["units_of_measure"]="0.7.0" \
      ["fourcc"]="0.7.0" \
      ["linalg"]="0.7.0" \
      ["geometry"]="0.7.0" \
      ["linalg-utils"]="0.7.0" \
      ["linalg-algo"]="0.7.0" \
      ["xmmt"]="0.7.0" \
      ["noise"]="0.7.0" \
      ["raytrace"]="0.7.0" \
      ["neuralnet"]="0.7.0" \
      ["htm"]="0.1.0" \
      ["doorgame"]="0.1.0" \
      ["pyhobbies"]="0.7.0" \
)
# build in which order
ORDER=(basal units_of_measure fourcc linalg geometry linalg-utils linalg-algo xmmt noise raytrace neuralnet htm doorgame pyhobbies)

WORKSPACE_ROOT=`pwd`
PROJECT_ROOT=${WORKSPACE_ROOT}/projects
BUILD_ROOT=${WORKSPACE_ROOT}/build
PACKAGE_ROOT=${WORKSPACE_ROOT}/package
INSTALL_ROOT=${WORKSPACE_ROOT}/install

CLEAN=0
JOBS=16
VERBOSE=OFF
RUN_TESTS=0
USE_CONAN=1
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
        -cmake)
            USE_CONAN=0
        ;;
        -dy)
            USE_SHARED_LIBS=ON
        ;;
        -del)
            for pkg ver in ${(kv)PKGS}; do
                conan remove --force ${pkg}/${ver}
            done
            exit 0
        ;;
        -rmv)
            for pkg ver in ${(kv)PKGS}; do
                conan editable remove ${pkg}/${ver}
            done
            exit 0
        ;;
        -edit)
            for pkg ver in ${(kv)PKGS}; do
                conan editable add --layout layout.ini projects/${pkg} ${pkg}/${ver}
            done
            exit 0
        ;;
        *)
        ;;
    esac
    shift
done


if [[ ${CLEAN} -eq 1 ]]; then
    rm -rf ${INSTALL_ROOT}
fi
mkdir -p ${INSTALL_ROOT}
export CONAN_TRACE_FILE=${INSTALL_ROOT}/conan_trace.log

# Cycle over each package and build it
for pkg in ${ORDER[@]}; do
    ver=${PKGS[${pkg}]}
    echo "Package=${pkg} Version=${ver}"
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
        # conan export ${project_path} ?
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

        editable_already=`conan editable list | grep ${project_path} | wc -l | awk '{print $1}'`
        if [[ ${editable_already} -eq 0 ]]; then
            # this exports the package into the local conan cache
            conan export-pkg ${project_path} --force -pf ${package_path} --profile default
        fi
    fi
done

# Install the Hobbies Python Module via CMake build to the User's Python Install
CMAKE_PREFIX_PATH=${INSTALL_ROOT} CMAKE_INSTALL_PREFIX=${INSTALL_ROOT} python3 -m pip install --user -e projects/pyhobbies
