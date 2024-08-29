#!/bin/bash
set -e

mkdir ${SRC_DIR}/build
mkdir ${SRC_DIR}/thirdparty
mkdir -p ${SRC_DIR}/thirdparty/local/share/cmake

cd ${SRC_DIR}/thirdparty

git clone --branch v4.0.0 --depth 1 https://github.com/Microsoft/GSL.git
git clone --branch v3.0.1 --depth 1 https://github.com/HowardHinnant/date

cd ${SRC_DIR}/thirdparty/fmt
cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DFMT_TEST=OFF \
    -DFMT_DOC=OFF \
    -DCMAKE_INSTALL_PREFIX=../local \
    -S . -B .
cmake --build . --target install

cd ${SRC_DIR}/thirdparty/spdlog
cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DSPDLOG_BUILD_BENCH=OFF \
    -DSPDLOG_BUILD_EXAMPLE=OFF \
    -DSPDLOG_BUILD_TESTS=OFF \
    -DSPDLOG_FMT_EXTERNAL=ON \
    -DCMAKE_INSTALL_PREFIX=../local \
    -S . -B .
cmake --build . --target install

cd ${SRC_DIR}/thirdparty/GSL
cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DGSL_TEST=OFF \
    -DCMAKE_INSTALL_PREFIX=../local \
    -S . -B .
cmake --build . --target install

cd ${SRC_DIR}/thirdparty/date
cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TZ_LIB=ON \
    -DUSE_SYSTEM_TZ_DB=ON \
    -DENABLE_DATE_TESTING=OFF \
    -DCMAKE_INSTALL_PREFIX=../local \
    -S . -B .
cmake --build . --target install

cd ${SRC_DIR}/build
# unset the SYSCONFIGDATA_NAME otherwise pybind11 python module extension detection fails
unset _CONDA_PYTHON_SYSCONFIGDATA_NAME
cmake \
    -G Ninja \
    ${CMAKE_ARGS} \
    -DPACKAGE_VERSION_COMMITHASH=${GIT_FULL_HASH} \
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH} \
    -DCMAKE_FIND_ROOT_PATH:PATH="${SRC_DIR}\thirdparty\local;${PREFIX};${BUILD_PREFIX}/x86_64-conda-linux-gnu/sysroot" \
    -DCMAKE_MODULE_PATH="${RECIPE_DIR}/cmake" \
    -DGDX_DISABLE_OPENMP=OFF \
    -DGDX_ENABLE_TOOLS=OFF \
    -DGDX_ENABLE_TESTS=OFF \
    -DGDX_ENABLE_TEST_UTILS=OFF \
    -DGDX_PYTHON_BINDINGS=ON \
    -DINFRA_ENABLE_TESTS=OFF \
    -DINFRA_EMBED_GDAL_DATA=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DGDX_INSTALL_DEVELOPMENT_FILES=OFF \
    -DGDX_ENABLE_OPENMP=OFF \
    -DGDX_ENABLE_SIMD=OFF \
    -DGDX_ENABLE_TOOLS=OFF \
    -DGDX_ENABLE_TESTS=OFF \
    -DGDX_ENABLE_TEST_UTILS=OFF \
    -DPython3_EXECUTABLE="$PYTHON" \
    -DPython3_FIND_VIRTUALENV=ONLY \
    -S ${SRC_DIR} \
    -B .

cmake --build . --target install 
