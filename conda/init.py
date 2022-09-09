# __init__ for geodynamix.

from sys import platform
if platform == 'win32':
    import os
    if 'USE_PATH_FOR_GDAL_PYTHON' in os.environ and 'PATH' in os.environ:
        for p in os.environ['PATH'].split(';'):
            if p:
                try:
                    os.add_dll_directory(p)
                except (FileNotFoundError, OSError):
                    continue
    elif 'PATH' in os.environ:
        import glob
        for p in os.environ['PATH'].split(';'):
            if glob.glob(os.path.join(p, 'gdal*.dll')) or glob.glob(os.path.join(p, 'libgdal*.dll')):
                try:
                    os.add_dll_directory(p)
                except (FileNotFoundError, OSError):
                    continue

from .geodynamix import *