set export := true

export GIT_COMMIT_HASH := `git rev-parse HEAD`

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
