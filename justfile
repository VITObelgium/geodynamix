cpp_vcpkg_root := env('VCPKG_ROOT', "../vcpkg")

build_debug:
    pixi run build

build_release:
    pixi run build --release

build: build_release

test:
    pixi run test

wheel_develop:
    pixi run maturin develop

pydoc: wheel_develop
    pixi run -e docs docs

wheel:
    pixi run maturin build

# test: wheel_develop
#     pixi run test

# cpp_bootstrap $VCPKG_ROOT=cpp_vcpkg_root:
#     '{{cpp_vcpkg_root}}/vcpkg' install --x-manifest-root='{{join(justfile_directory(), "cpp")}}' --allow-unsupported --triplet {{VCPKG_DEFAULT_TRIPLET}}

# cpp_configure $VCPKG_ROOT=cpp_vcpkg_root: cpp_bootstrap
#     cmake --preset {{cmake_preset}}

# cpp_build_debug: cpp_configure
#     cmake --build ./build --config Debug

# cpp_build_release: cpp_configure
#     cmake --build ./build --config Release

# cpp_build: cpp_build_release

# cpp_test_debug: cpp_build
#     ctest --test-dir ./build --output-on-failure -C Debug

# cpp_test_release: cpp_build
#     ctest --test-dir ./build --output-on-failure -C Release

# cpp_test: cpp_test_release
