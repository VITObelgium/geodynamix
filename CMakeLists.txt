cmake_minimum_required(VERSION 3.12)

set(PACKAGE_VERSION_COMMITHASH "dev" CACHE STRING "git commit hash")
if (PACKAGE_VERSION_COMMITHASH STREQUAL "dev")
    set(GDX_DEV_BUILD 1)
else ()
    set(GDX_DEV_BUILD 0)
endif ()

enable_testing()

include(CTestUseLaunchers)
include(CMakeDependentOption)
include(CMakePackageConfigHelpers)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/gdx)

string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" GDX_IS_TOPLEVEL)

option(GDX_ENABLE_SIMD "Build with simd support for DenseRaster (default = sse instructions)" ON)
cmake_dependent_option(GDX_AVX2 "Build with avx2 simd instructions support" OFF "GDX_ENABLE_SIMD" OFF)
option(GDX_ENABLE_OPENMP "Build without openmp acceleration support" OFF)
option(GDX_ENABLE_TOOLS "Build the gdx tools" ON)
option(GDX_ENABLE_TESTS "Build the c++ unit tests" OFF)
option(GDX_ENABLE_TEST_UTILS "Build the unit test utilities" ${GDX_ENABLE_TESTS})
option(GDX_ENABLE_BENCHMARKS "Build the micro benchmarks" OFF)
option(GDX_PYTHON_BINDINGS "Build python bindings" OFF)
option(GDX_INSOURCE_DEPS "Use submodules for internal dependencies" ON)
option(GDX_INSTALL_DEVELOPMENT_FILES "Install the gdx development files (headers/libs)" ${GDX_IS_TOPLEVEL})

if (GDX_IS_TOPLEVEL)
    option(ENABLE_ADDRESS_SANITIZER "Enable the address sanitizer" OFF)
    option(ENABLE_UB_SANITIZER "Enable the undefined behavior sanitizer" OFF)
    
    project(GeoDynamiX
        VERSION 0.12.0
        LANGUAGES CXX
    )
    set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

    include(GNUInstallDirs)

    message(STATUS "Gdx version: ${GDX_VERSION} (${PACKAGE_VERSION_COMMITHASH})")
    message(STATUS "Unit tests: ${GDX_ENABLE_TESTS}")
    message(STATUS "Address sanitizer: ${ENABLE_ADDRESS_SANITIZER}")
    message(STATUS "Undefined behavior sanitizer: ${ENABLE_UB_SANITIZER}")

    if (MSVC)
        add_compile_options(/Zc:twoPhase- /permissive- /bigobj /w34100 /w34101 /w35038 /w34127)
        # avoid windows specific warnings
        add_compile_definitions(
            _CRT_SECURE_NO_WARNINGS
            _SCL_SECURE_NO_WARNINGS
            _SILENCE_CXX17_NEGATORS_DEPRECATION_WARNING
            _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
            _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING
            _SILENCE_CXX17_UNCAUGHT_EXCEPTION_DEPRECATION_WARNING
        )

        # Fixes warning when linking against release c library in debug mode
        set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT /NODEFAULTLIB:LIBCMT")
        set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT /NODEFAULTLIB:LIBCMT")

        # Enable faster linking for debug builds
        set (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG:FASTLINK")
        set (CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /DEBUG:FASTLINK")
        set (CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} /DEBUG:FASTLINK")
    else ()
        add_compile_options(
            -Wall -Wextra -Wpedantic -Wfatal-errors -Wno-unknown-pragmas
            $<$<CXX_COMPILER_ID:GNU>:-Wno-maybe-uninitialized>
            $<$<CXX_COMPILER_ID:GNU>:-fdiagnostics-color=always>
            $<$<CXX_COMPILER_ID:Clang>:-fcolor-diagnostics>
        )
    endif ()

    if (NOT MSVC AND ENABLE_ADDRESS_SANITIZER)
        add_compile_options($<$<CONFIG:Debug>:-O1 -fno-optimize-sibling-calls -fno-omit-frame-pointer -fsanitize=address>)
        set (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fno-optimize-sibling-calls")
        set (CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fno-optimize-sibling-calls")
        set (CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fno-optimize-sibling-calls")

        if (${CMAKE_COMPILER_IS_GNUCXX})
            set (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -static-libasan")
            set (CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -static-libasan")
            set (CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} -static-libasan")
        endif ()
    endif ()

    if (NOT MSVC AND ENABLE_UB_SANITIZER)
        add_compile_options($<$<CONFIG:Debug>:-fsanitize=undefined>)
        set (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=undefined")
        set (CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fsanitize=undefined")
        set (CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} -fsanitize=undefined")
    endif ()

    write_basic_package_version_file("GdxConfigVersion.cmake"
        VERSION ${GDX_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/GdxConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/GdxConfig.cmake
        INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
    )

    if (GDX_INSTALL_DEVELOPMENT_FILES)
        install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/GdxConfig.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/GdxConfigVersion.cmake
            DESTINATION ${INSTALL_CONFIGDIR}
        )
    endif ()
endif ()

if (MINGW OR UNIX)
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif ()

if (MINGW)
    add_definitions("-DMINGW_HAS_SECURE_API")
endif ()

if (MSVC)
    set (LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
    set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
else ()
    set (LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
    set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
endif ()

find_package(Gsl REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

if (GDX_ENABLE_OPENMP)
    find_package(OpenMP)
endif ()

message(STATUS "Gdx OpenMP support: ${OpenMP_FOUND}")

if (GDX_ENABLE_TEST_UTILS OR GDX_ENABLE_TESTS)
    find_package(doctest CONFIG REQUIRED)
endif ()

if (GDX_IS_TOPLEVEL)
    if (GDX_INSOURCE_DEPS)
        # gdx is the toplevel project so include the infra dependency
        set(INFRA_LOGGING ON CACHE BOOL "" FORCE)
        set(INFRA_GDAL ON CACHE BOOL "" FORCE)
        set(INFRA_EMBED_GDAL_DATA ON CACHE BOOL "")
        set(INFRA_ENABLE_TESTS ${GDX_ENABLE_TESTS} CACHE BOOL "" FORCE)
        set(INFRA_ENABLE_TEST_UTILS ${GDX_ENABLE_TEST_UTILS} CACHE BOOL "" FORCE)
        set(INFRA_INSTALL_DEVELOPMENT_FILES ${GDX_INSTALL_DEVELOPMENT_FILES} CACHE BOOL "" FORCE)
        add_subdirectory(deps/infra)
    else ()
        find_package(Infra CONFIG REQUIRED)
        if (GDX_ENABLE_TEST_UTILS)
            find_package(Infra CONFIG COMPONENTS testutil REQUIRED)
        endif ()
    endif ()
endif ()

if (GDX_ENABLE_TEST_UTILS)
    add_subdirectory(testutil)
endif ()

add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(algorithms)

if (GDX_PYTHON_BINDINGS)
    add_subdirectory(python)
    add_subdirectory(test)
endif ()

if (GDX_ENABLE_TOOLS)
    add_subdirectory(tools)
endif ()

if (GDX_ENABLE_BENCHMARKS)
    find_package(benchmark REQUIRED)
    add_subdirectory(benchmarks)
endif ()


if (EMSCRIPTEN)
    add_subdirectory(js)
endif ()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gdxconfig.h.in ${CMAKE_BINARY_DIR}/gdx/config.h)
if (WIN32)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.rc.in ${CMAKE_CURRENT_BINARY_DIR}/version.rc @ONLY)
endif ()

if (GDX_IS_TOPLEVEL AND GDX_INSTALL_DEVELOPMENT_FILES)
    install(FILES ${CMAKE_BINARY_DIR}/gdx/config.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gdx
    )

    install(EXPORT gdx-targets
        FILE GdxTargets.cmake
        NAMESPACE Gdx::
        DESTINATION ${INSTALL_CONFIGDIR}
    )
endif ()
