#!/usr/bin/env python3
import os
import sys
import platform
import argparse

from deps.vcpkg.scripts.buildtools import vcpkg

if __name__ == "__main__":
    try:
        parser = argparse.ArgumentParser(
            description="Bootstrap gdx-core.", parents=[vcpkg.bootstrap_argparser()]
        )

        args = parser.parse_args()
        triplet = (
            "x64-windows-static-vs2022"
            if platform.system() == "Windows"
            else args.triplet
        )

        overlay_ports = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "deps", "overlay-ports")
        )

        if args.parent:
            del vcpkg
            sys.path.insert(0, os.path.join("..", "vcpkg-ports", "scripts"))
            from buildtools import vcpkg
        elif platform == "win-amd64":
            if os.path.isdir("c:/DEV/bld"):
                build_root = (
                    "c:/DEV/bld"  # avoid long path issues by using a short build path
                )

        if args.clean:
            vcpkg.clean(triplet=triplet)
        else:
            vcpkg.bootstrap(
                ports_dir=os.path.join(".", "deps"),
                triplet=triplet,
                build_root=build_root,
                overlay_ports=overlay_ports,
            )
    except KeyboardInterrupt:
        print("\nInterrupted")
        sys.exit(-1)
    except RuntimeError as e:
        print(e)
        sys.exit(-1)
