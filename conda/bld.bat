SET script_dir=%~dp0
SET build_dir=%source_dir%/build
SET thirdparty_dir=%source_dir%/thirdparty
SET thirdparty_install_dir=%thirdparty_dir%/local

mkdir %SRC_DIR%\build
mkdir %SRC_DIR%\thirdparty

cd %SRC_DIR%\thirdparty

git clone --branch 7.1.3 --depth 1 https://github.com/fmtlib/fmt.git
git clone --branch v1.8.5 --depth 1 https://github.com/gabime/spdlog.git
git clone --branch v3.1.0 --depth 1 https://github.com/Microsoft/GSL.git
git clone --branch v3.0.1 --depth 1 https://github.com/HowardHinnant/date

REM Anaconda pkgs/main package 'eigen' for win-64 does not include Eigen3Config.cmake, so we build Eigen ourselves.
git clone --branch 3.3.9 --depth 1 https://gitlab.com/libeigen/eigen.git eigen

cd %SRC_DIR%/thirdparty/date
cmake -G Ninja ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DBUILD_TZ_LIB=ON ^
    -DUSE_SYSTEM_TZ_DB=ON ^
    -DENABLE_DATE_TESTING=OFF ^
    -DCMAKE_INSTALL_PREFIX=../local ^
    -S . -B .
if %ERRORLEVEL% NEQ 0 exit %ERRORLEVEL%
cmake --build . --target install
if %ERRORLEVEL% NEQ 0 exit %ERRORLEVEL%

cd %SRC_DIR%/thirdparty/fmt
cmake -G Ninja ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DFMT_TEST=OFF ^
    -DFMT_DOC=OFF ^
    -DCMAKE_INSTALL_PREFIX=../local ^
    -S . -B .
if %ERRORLEVEL% NEQ 0 exit %ERRORLEVEL%
cmake --build . --target install
if %ERRORLEVEL% NEQ 0 exit %ERRORLEVEL%

cd %SRC_DIR%/thirdparty/spdlog
cmake -G Ninja ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DSPDLOG_BUILD_BENCH=OFF ^
    -DSPDLOG_BUILD_EXAMPLE=OFF ^
    -DSPDLOG_BUILD_TESTS=OFF ^
    -DSPDLOG_FMT_EXTERNAL=ON ^
    -DCMAKE_INSTALL_PREFIX=../local ^
    -S . -B .
if %ERRORLEVEL% NEQ 0 exit %ERRORLEVEL%
cmake --build . --target install
if %ERRORLEVEL% NEQ 0 exit %ERRORLEVEL%

cd %SRC_DIR%/thirdparty/GSL
cmake -G Ninja ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DGSL_TEST=OFF ^
    -DCMAKE_INSTALL_PREFIX=../local ^
    -S . -B .
if %ERRORLEVEL% NEQ 0 exit %ERRORLEVEL%
cmake --build . --target install
if %ERRORLEVEL% NEQ 0 exit %ERRORLEVEL%

mkdir %SRC_DIR%\thirdparty\eigen\build
cd %SRC_DIR%\thirdparty\eigen\build
cmake -G Ninja ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_INSTALL_PREFIX=../local ^
    -S .. -B .
if %ERRORLEVEL% NEQ 0 exit %ERRORLEVEL%
cmake --build . --target install
if %ERRORLEVEL% NEQ 0 exit %ERRORLEVEL%

cd %SRC_DIR%/build

cmake ^
    -G Ninja ^
    %CMAKE_ARGS% ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DPACKAGE_VERSION_COMMITHASH=%GIT_FULL_HASH% ^
    -DCMAKE_INSTALL_PREFIX:PATH="%PREFIX%" ^
    -DCMAKE_PREFIX_PATH:PATH="%CMAKE_PREFIX_PATH%" ^
    -DCMAKE_FIND_ROOT_PATH:PATH="%SRC_DIR%\thirdparty\local" ^
    -DCMAKE_MODULE_PATH:PATH="%RECIPE_DIR%\cmake" ^
    -Ddate_DIR="%SRC_DIR%\thirdparty\local\CMake" ^
    -Dfmt_DIR:PATH="%SRC_DIR%\thirdparty\local\lib\cmake\fmt" ^
    -Dspdlog_DIR:PATH="%SRC_DIR%\thirdparty\local\lib\cmake\spdlog" ^
    -DGDX_ENABLE_OPENMP=OFF ^
    -DGDX_AVX2=OFF ^
    -DGDX_ENABLE_SIMD=OFF ^
    -DGDX_ENABLE_TOOLS=OFF ^
    -DGDX_ENABLE_TESTS=OFF ^
    -DGDX_ENABLE_TEST_UTILS=OFF ^
    -DGDX_PYTHON_BINDINGS=ON ^
    -DINFRA_ENABLE_TESTS=OFF ^
    -DINFRA_EMBED_GDAL_DATA=OFF ^
    -DGDX_INSTALL_DEVELOPMENT_FILES=OFF ^
    -DPython3_ROOT_DIR="%PREFIX%" ^
    -DPython3_FIND_VIRTUALENV=ONLY ^
    ..
if %ERRORLEVEL% NEQ 0 exit %ERRORLEVEL%
cmake --build . --target install
