if(GDX_IS_TOPLEVEL)
    if(GDX_INSOURCE_DEPS)
        # gdx is the toplevel project so include the infra dependency
        set(INFRA_LOGGING ON)
        set(INFRA_GDAL ON)
        set(INFRA_EMBED_GDAL_DATA OFF CACHE BOOL "")
        set(INFRA_ENABLE_TESTS ${GDX_ENABLE_TESTS})
        set(INFRA_ENABLE_TEST_UTILS ${GDX_ENABLE_TEST_UTILS})
        set(INFRA_INSTALL_DEVELOPMENT_FILES ${GDX_INSTALL_DEVELOPMENT_FILES})
        add_subdirectory(deps/infra)
    else()
        find_package(Infra CONFIG REQUIRED)

        if(GDX_ENABLE_TEST_UTILS)
            find_package(Infra CONFIG COMPONENTS testutil REQUIRED)
        endif()
    endif()
endif()

if(GDX_ENABLE_TEST_UTILS)
    add_subdirectory(testutil)
endif()

add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(algorithms)

if(GDX_PYTHON_BINDINGS)
    find_package(Python3 COMPONENTS Interpreter Development.Module)
    add_subdirectory(python)
    add_subdirectory(test)
endif()

if(GDX_ENABLE_TOOLS)
    add_subdirectory(tools)
endif()

if(GDX_ENABLE_BENCHMARKS)
    find_package(benchmark REQUIRED)
    add_subdirectory(benchmarks)
endif()

if(EMSCRIPTEN)
    add_subdirectory(js)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gdxconfig.h.in ${CMAKE_BINARY_DIR}/gdx/config.h)

if(WIN32)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.rc.in ${CMAKE_CURRENT_BINARY_DIR}/version.rc @ONLY)
endif()

if(GDX_IS_TOPLEVEL AND GDX_INSTALL_DEVELOPMENT_FILES)
    install(FILES ${CMAKE_BINARY_DIR}/gdx/config.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gdx
    )

    install(EXPORT geodynamix-targets
        FILE GeodynamixTargets.cmake
        NAMESPACE geodynamix::
        DESTINATION ${INSTALL_CONFIGDIR}
    )
endif()

if(GDX_IS_TOPLEVEL)
    add_subdirectory(dist)
endif()
