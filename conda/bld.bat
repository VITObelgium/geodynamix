SET script_dir=%~dp0
SET build_dir=%source_dir%/build
SET thirdparty_dir=%source_dir%/thirdparty
SET thirdparty_install_dir=%thirdparty_dir%/local

mkdir %SRC_DIR%\build
mkdir %SRC_DIR%\thirdparty

cd %SRC_DIR%/build

cmake ^
    -G Ninja ^
    %CMAKE_ARGS% ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DPACKAGE_VERSION_COMMITHASH=%GIT_FULL_HASH% ^
    -DCMAKE_INSTALL_PREFIX:PATH="%PREFIX%" ^
    -DCMAKE_PREFIX_PATH:PATH="%CMAKE_PREFIX_PATH%" ^
    -DCMAKE_MODULE_PATH:PATH="%RECIPE_DIR%\cmake" ^
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
    -S .. -B .
if %ERRORLEVEL% NEQ 0 exit %ERRORLEVEL%
cmake --build . --target install
