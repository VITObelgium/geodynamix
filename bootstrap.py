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
            "x64-windows-static-vs2019" if platform.system() == "Windows" else args.triplet
        )

        if args.parent:
            del vcpkg
            sys.path.insert(0, os.path.join("..", "vcpkg-ports", "scripts"))
            from buildtools import vcpkg

        if args.clean:
            vcpkg.clean(triplet=triplet)
        else:
            vcpkg.bootstrap(ports_dir=os.path.join(".", "deps"), triplet=triplet)
    except KeyboardInterrupt:
        print("\nInterrupted")
        sys.exit(-1)
    except RuntimeError as e:
        print(e)
        sys.exit(-1)
