{
  pkgs,
  inputs,
  lib,
  config,
  ...
}:

let
  buildEnv = inputs.pkgs-mod.lib.mkBuildEnv pkgs.stdenv.hostPlatform.system;

  staticCC = buildEnv.pkgsStatic.stdenv.cc;
  staticTargetPrefix = staticCC.targetPrefix;

  projectPackagesFor =
    pkgSet: with pkgSet; [
      pkg-mod-cryptopp
      pkg-mod-gdal
      pkg-mod-geos
      pkg-mod-lyra
      pkg-mod-fmt
      pkg-mod-spdlog
      pkg-mod-eigen
      pkg-mod-vc
      pkg-mod-doctest
      pkg-mod-howard-hinnant-date
    ];

  staticProjectPackages = projectPackagesFor buildEnv.pkgsStatic;

  # Devenv adds `packages` as native build inputs, which causes Nix to splice
  # cross packages back to their native variants. A native build environment
  # preserves the intended Zig/glibc target outputs and their development files.
  staticProjectDependencies = pkgs.buildEnv {
    name = "geodynanmix-static-project-dependencies";
    paths = lib.closePropagation staticProjectPackages;
    extraOutputsToInstall = [
      "out"
      "dev"
      "lib"
    ];
    ignoreCollisions = true;
  };

  commonPackages =
    (with pkgs; [
      cmake
      cmakeCurses
      ninja
      pkg-config
      just
      flex
      bison
      mold
      sccache
      autoconf
      autoconf-archive
      automake
      libtool
      (python314.withPackages (ps: [ ps.numpy ]))
      python314Packages.pybind11
      trompeloeil
      nlohmann_json
      nixd
      nixfmt
      clang-tools
      neocmakelsp
    ])
    ++ lib.optionals pkgs.stdenv.isLinux [ pkgs.glib ]
    ++ lib.optionals (pkgs.stdenv.hostPlatform.system != "aarch64-darwin") [ pkgs.gdb ];

  localPackages = commonPackages ++ [ staticProjectDependencies ];

  ciPackages = commonPackages ++ [
    staticProjectDependencies
  ];

in
{
  cachix.pull = [ "geo-overlay" ];

  # Shell environment variables
  env = {
    LC_ALL = "C.UTF-8";
    LANG = "en_US.UTF-8";
    GDX_STATIC_DEPS = "${staticProjectDependencies}";
    GDX_STATIC_C_COMPILER = "${staticCC}/bin/${staticTargetPrefix}cc";
    GDX_STATIC_CXX_COMPILER = "${staticCC}/bin/${staticTargetPrefix}c++";
  }
  // lib.optionalAttrs pkgs.stdenv.isLinux {
    LOCALE_ARCHIVE = "${pkgs.glibcLocales}/lib/locale/locale-archive";
  };

  # Use the static pkg-mod dependency set for local and CI builds.
  packages = localPackages;

  # Keep the CI profile for its checks task.
  profiles.ci.module = {
    packages = lib.mkForce ciPackages;
    env.QT_PLUGIN_PATH = lib.mkForce "";

    tasks."ci:checks".exec = ''
      cmake --fresh --preset nix
      cmake --build --preset nix-release
      ctest --preset nix-release --exclude-regex '.*integrationtest' --test-output-size-failed 0
    '';
  };

  # The Zig compiler wrapper uses $TMPDIR/zig-cache for its global cache.
  enterShell = ''
    export TMPDIR="${config.devenv.root}/build/tmp"
    mkdir -p "$TMPDIR"
  '';
}
