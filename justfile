set export
# detect the vcpkg triplet based on the system information
VCPKG_DEFAULT_TRIPLET := if os_family() == "windows" {
  "x64-windows-static-vs2022"
  } else if os() == "macos" {
    if arch() == "aarch64" {
      "arm64-osx"
    } else { "x64-osx" }
  } else {
    "x64-linux"
  }
PYTHON_EXE := if os_family() == "windows" {
    "python.exe"
  } else {
    "bin/python3"
  }

cmake_preset := if os_family() == "windows" {
    "windows"
} else if os() == "macos" {
    if arch() == "aarch64" {
        "mac-arm"
    } else { "mac-intel" }
} else {
    "linux"
}

RUST_TRIPLET := VCPKG_DEFAULT_TRIPLET + "-release"
VCPKG_DEFAULT_HOST_TRIPLET := VCPKG_DEFAULT_TRIPLET

cpp_vcpkg_root := env('VCPKG_ROOT', "../vcpkg")
rust_vcpkg_root := join(justfile_directory(), "target", "vcpkg")

export VCPKG_OVERLAY_TRIPLETS := join(justfile_directory(), "cpp", "deps", "infra", "vcpkg_overlay", "triplets")
export VCPKG_OVERLAY_PORTS := join(justfile_directory(), "cpp", "deps", "infra", "vcpkg_overlay", "ports")

cargo-config-gen:
  mkdir -p .cargo
  cp infra-rs/.cargo/config.toml.in .cargo/config.toml
  sd @CARGO_VCPKG_TRIPLET@ {{VCPKG_DEFAULT_TRIPLET}}-release .cargo/config.toml
  sd @PYTHON_EXE@ {{PYTHON_EXE}} .cargo/config.toml

bootstrap $VCPKG_ROOT=rust_vcpkg_root $VCPKG_DEFAULT_HOST_TRIPLET=RUST_TRIPLET : cargo-config-gen
  echo "Bootstrapping vcpkg:{{RUST_TRIPLET}}..."
  cargo vcpkg -v build
  -cp target/vcpkg/installed/x64-windows-static-vs2022-release/lib/gdal.lib target/vcpkg/installed/x64-windows-static-vs2022-release/lib/gdal_i.lib
  fd --base-directory target/vcpkg/installed -g gdal.pc --exec sd -F -- '-l-framework' '-framework'
  -mkdir -p target/data && mkdir -p target/debug && mkdir -p target/release
  -mkdir -p ./python/geodynamix.data/data/share/geodynamix/
  fd -g proj.db ./target/vcpkg/installed --exec cp "{}" ./target/data/
  cp ./target/data/proj.db ./target/debug/
  cp ./target/data/proj.db ./target/release/
  cp ./target/data/proj.db ./python/geodynamix.data/data/share/geodynamix/
  pixi install

doc:
  cargo doc --workspace --exclude='infra-rs' --exclude='vector_derive' --no-deps --all-features --open

docdeps:
  cargo doc --workspace --exclude='infra-rs' --exclude='vector_derive' --all-features --open

build_debug:
  cargo build --workspace

build_release:
  cargo build --workspace --release

test_debug:
    cargo nextest run --profile ci --workspace --features=static_build

wheel_develop:
    pixi run maturin develop

wheel:
    pixi run maturin build

test: wheel_develop
    pixi run test

cpp_bootstrap $VCPKG_ROOT=cpp_vcpkg_root:
    '{{cpp_vcpkg_root}}/vcpkg' install --x-manifest-root='{{join(justfile_directory(), "cpp")}}' --allow-unsupported --triplet {{VCPKG_DEFAULT_TRIPLET}}

cpp_configure $VCPKG_ROOT=cpp_vcpkg_root: cpp_bootstrap
    cmake --preset {{cmake_preset}}

cpp_build_debug: cpp_configure
    cmake --build ./build --config Debug

cpp_build_release: cpp_configure
    cmake --build ./build --config Release

cpp_build: cpp_build_release

cpp_test_debug: cpp_build
    ctest --test-dir ./build --output-on-failure -C Debug

cpp_test_release: cpp_build
    ctest --test-dir ./build --output-on-failure -C Release

cpp_test: cpp_test_release