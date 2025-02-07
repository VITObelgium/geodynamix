set export
# detect the vcpkg triplet based on the system information
VCPKG_DEFAULT_TRIPLET := if os_family() == "windows" {
  "x64-windows-static-release"
  } else if os() == "macos" {
    if arch() == "aarch64" {
      "arm64-osx-release"
    } else { "x64-osx-release" }
  } else {
    "x64-linux-release"
  }
PYTHON_EXE := if os_family() == "windows" {
    "python.exe"
  } else {
    "bin/python3"
  }
VCPKG_DEFAULT_HOST_TRIPLET := VCPKG_DEFAULT_TRIPLET

cargo-config-gen:
  mkdir -p .cargo
  cp infra-rs/.cargo/config.toml.in .cargo/config.toml
  sd @CARGO_VCPKG_TRIPLET@ {{VCPKG_DEFAULT_TRIPLET}} .cargo/config.toml
  sd @PYTHON_EXE@ {{PYTHON_EXE}} .cargo/config.toml
  
bootstrap: cargo-config-gen
  echo "Bootstrapping vcpkg:{{VCPKG_DEFAULT_TRIPLET}}..."
  cargo vcpkg -v build
  -cp target/vcpkg/installed/x64-windows-static/lib/gdal.lib target/vcpkg/installed/x64-windows-static/lib/gdal_i.lib
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

build: build_release

test:
  pixi run maturin develop; pixi run test

