A toolbox for spatial analysis.

## Building
#### Requirements
- cpp-infra (https://github.com/VITObelgium/cpp-infra)
- Eigen (https://eigen.tuxfamily.org/)
Optional
- Vc (https://github.com/VcDevel/Vc) [simd support]
- Doctest (https://github.com/onqtam/doctest) [unit tests]

The library is built using CMake:
```
cmake -G Ninja "/path/to/geodynamix"
```

#### Build options
The following CMake options are available for toggling functionality:
- `GDX_ENABLE_SIMD` Build with simd support for DenseRaster (default = sse instructions) (requires Vc)
- `GDX_AVX2` Build with avx2 simd instructions support
- `GDX_ENABLE_OPENMP` Build without openmp acceleration support
- `GDX_ENABLE_TOOLS` Build the geydynamix tools
- `GDX_ENABLE_TESTS` Build the c++ unit tests
- `GDX_ENABLE_TEST_UTILS` Build the unit test utilities (raster equality asserts, usefull for unit tests in depending projects)
- `GDX_ENABLE_BENCHMARKS` Build the micro benchmarks
- `GDX_PYTHON_BINDINGS` Build python bindings
- `GDX_INSOURCE_DEPS` Use submodules for internal dependencies
- `GDX_INSTALL_DEVELOPMENT_FILES` Install the geodynamix development files (headers/libs)

## Using geodynamix in your project
directly add the source directory in your cmake project
```
# first set the enabled components in the cache
set(GDX_PYTHON_BINDINGS OFF CACHE BOOL "" FORCE)
set(GDX_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(GDX_ENABLE_TEST_UTILS ON CACHE BOOL "" FORCE)
set(GDX_ENABLE_TOOLS OFF CACHE BOOL "" FORCE)
set(GDX_PYTHON_BINDINGS OFF CACHE BOOL "" FORCE)
set(GDX_ENABLE_SIMD ON CACHE BOOL "" FORCE)
set(GDX_AVX2 ON CACHE BOOL "" FORCE)
add_subdirectory(geodynamix)
```

or use the installed cmake module
```
find_package(Gdx CONFIG REQUIRED)
target_link_libraries(mytarget PRIVATE Gdx::gdxcore Gdx::gdxalgo)