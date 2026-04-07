set export := true

export GIT_COMMIT_HASH := `git rev-parse HEAD`

import 'deps/infra/vcpkg_overlay/vcpkg.just'

project_name := "waterbodemverkenner"
export VCPKG_FORCE_DOWNLOADED_BINARIES := "1"

[windows]
configure_vs $VCPKG_ROOT=vcpkg_root: bootstrap
    cmake --preset x64-windows-static-vs

[windows]
build_vs: configure_vs
    cmake --build ./build/gdx-vs --config Release

[windows]
bootstrap triplet=VCPKG_DEFAULT_TRIPLET $VCPKG_ROOT=vcpkg_root:
    - mkdir -p '{{ join(justfile_directory(), "build", "vcpkgs") }}'
    '{{ vcpkg_root }}/vcpkg' install --allow-unsupported --triplet {{ triplet }} --x-install-root=./build/vcpkgs/{{ triplet }}

[windows]
configure triplet=VCPKG_DEFAULT_TRIPLET $VCPKG_ROOT=vcpkg_root:
    cmake --preset {{ triplet }}

[windows]
build_debug triplet=VCPKG_DEFAULT_TRIPLET: (configure triplet)
    cmake --build --preset {{ triplet }}-debug

[windows]
build_release triplet=VCPKG_DEFAULT_TRIPLET: (configure triplet)
    cmake --build --preset {{ triplet }}-release

[windows]
build triplet=VCPKG_DEFAULT_TRIPLET: (build_release triplet)

[private]
build_dist triplet=VCPKG_DEFAULT_TRIPLET $VCPKG_ROOT=vcpkg_root: git_status_clean
    rm -rf ./build/gdx-{{ triplet }}-dist
    rm -rf ./build/dist-{{ triplet }}-local
    cmake --preset {{ triplet }}-dist --install-prefix='{{ join(justfile_directory(), "build", "dist-" + triplet + "-local") }}'
    cmake --build --preset {{ triplet }}-dist --target package
    cmake --install ./build/gdx-{{ triplet }}-dist --strip --verbose

[unix]
configure:
    cmake  --preset nix

[unix]
build_debug: configure
    cmake --build --preset nix-debug

[unix]
build_release: configure
    cmake --build --preset nix-release

[unix]
build: build_release

[unix]
rebuild:
    cmake --build --preset nix-release --clean-first

# update_devlatest: (build_dist 'x64-linux-cluster')
#     cp -v ./build/x64-linux-cluster-dist/Release/emapcli /projects/E-MAP/03_Software/snapshots/devlatest/

[unix]
test_debug: build
    ctest --verbose --preset nix-debug --output-on-failure

[unix]
test_release: build
    ctest --verbose --preset nix-release --output-on-failure

[unix]
test: test_release

[unix]
run_debug: build_debug
    ./build/nix/bin/Debug/gdx

[unix]
run_release: build_release
    ./build/nix/bin/Release/gdx

[unix]
run: run_release

[unix]
update:
    nix flake update

[unix]
updatedeps:
    nix flake update --update-input pkgs-mod

[windows]
pixi_build:
    rm -rf ./.pixi/build
    rm -rf ./build/conda
    pixi build --output-dir=./build/conda

# Build conda package using pixi in a clean environment (no nix/devenv pollution)
[unix]
pixi_build:
    #!/usr/bin/env sh
    echo "Building pixi package in clean environment (excluding nix paths)..."
    # Remove existing build artifacts safely
    if [ -d .pixi/build/work ]; then \
        find .pixi/build/work -mindepth 1 -maxdepth 1 -name "urbclim-*" -exec rm -rf {} + 2>/dev/null || true; \
    fi
    # Run pixi build with minimal environment to avoid nix store pollution
    # We need to keep enough PATH to find pixi and basic tools
    env -i \
        HOME="$HOME" \
        USER="$USER" \
        TERM="${TERM:-xterm}" \
        GIT_COMMIT_HASH="$GIT_COMMIT_HASH" \
        PATH="/run/current-system/sw/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:$HOME/.local/bin:$HOME/.pixi/bin" \
        sh -c 'pixi build --output-dir=./build/conda'
