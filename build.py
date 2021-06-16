#!/usr/bin/env python3
import os
import sys
import sysconfig
import argparse
import platform

from deps.vcpkg.scripts.buildtools import vcpkg

if __name__ == "__main__":
    try:
        parser = argparse.ArgumentParser(
            description="Build gdx.", parents=[vcpkg.build_argparser()]
        )
        parser.add_argument(
            "--no-avx2", dest="no_avx2", action="store_true", help="build with avx2 instructions"
        )

        args = parser.parse_args()

        sys_platform = sysconfig.get_platform()

        triplet = args.triplet
        if sys_platform == "win-amd64":
            triplet = "x64-windows-static-vs2019"
        elif not triplet:
            triplet = vcpkg.prompt_for_triplet()

        cmake_args = [
            "-DINFRA_EMBED_GDAL_DATA=ON",
            "-DGDX_ENABLE_TESTS=ON",
            "-DGDX_PYTHON_BINDINGS=ON",
            "-DGDX_ENABLE_TOOLS=ON",
            "-DGDX_ENABLE_BENCHMARKS=ON",
            "-DGDX_ENABLE_SIMD=ON",
        ]

        if not args.no_avx2:
            cmake_args.append("-DGDX_AVX2=ON")

        build_dir = "gdx"

        if args.parent:
            del vcpkg
            sys.path.insert(0, os.path.join("..", "vcpkg-ports", "scripts"))
            from buildtools import vcpkg

        if args.build_dist:
            vcpkg.build_project_release(
                os.path.abspath(args.source_dir),
                triplet=triplet,
                cmake_args=cmake_args,
                build_name=build_dir,
            )
        else:
            vcpkg.build_project(
                os.path.abspath(args.source_dir),
                triplet=triplet,
                cmake_args=cmake_args,
                build_name=build_dir,
            )
    except KeyboardInterrupt:
        print("\nInterrupted")
        sys.exit(-1)
    except RuntimeError as e:
        print(e)
        sys.exit(-1)
