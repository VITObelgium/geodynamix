if (GDX_ENABLE_SIMD)
    # do no use the provided Vc cmake module
    # it will just autodetect the host machine and enable everything that is supported
    # this is not the behaviour we want

    include(FindPackageHandleStandardArgs)

    find_path(Vc_INCLUDE_DIR
        NAMES Vc/Vc
        HINTS ${Vc_ROOT_DIR}/include ${Vc_INCLUDEDIR}
    )

    find_library(Vc_LIBRARY NAMES Vc HINTS ${Vc_ROOT_DIR}/lib)
    find_library(Vc_LIBRARY_DEBUG NAMES Vcd HINTS ${Vc_ROOT_DIR}/lib)

    find_package_handle_standard_args(Vc
        FOUND_VAR Vc_FOUND
        REQUIRED_VARS Vc_INCLUDE_DIR Vc_LIBRARY
    )

    mark_as_advanced(
        Vc_ROOT_DIR
        Vc_INCLUDE_DIR
        Vc_LIBRARY
        Vc_LIBRARY_DEBUG
    )

    if(Vc_FOUND AND NOT TARGET Vc::Vc)
        add_library(Vc::Vc STATIC IMPORTED)
        set_target_properties(Vc::Vc PROPERTIES
            IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
            INTERFACE_INCLUDE_DIRECTORIES "${Vc_INCLUDE_DIR}"
            IMPORTED_LOCATION ${Vc_LIBRARY}
            IMPORTED_LOCATION_RELWITHDEBINFO ${Vc_LIBRARY}
        )

        if(Vc_LIBRARY_DEBUG)
            set_target_properties(Vc::Vc PROPERTIES
                IMPORTED_LOCATION_DEBUG "${Vc_LIBRARY_DEBUG}"
            )
        endif()

        if (GDX_AVX2)
            set_target_properties(Vc::Vc PROPERTIES
                INTERFACE_COMPILE_DEFINITIONS Vc_IMPL=AVX2
            )
            if (MSVC)
                set_target_properties(Vc::Vc PROPERTIES
                    INTERFACE_COMPILE_OPTIONS /arch:AVX2
                )
            else ()
                set_target_properties(Vc::Vc PROPERTIES
                    INTERFACE_COMPILE_OPTIONS "-mavx2;-ffp-contract=fast"
                )
            endif ()
        else ()
            if (MSVC)
                set_target_properties(Vc::Vc PROPERTIES
                    INTERFACE_COMPILE_DEFINITIONS Vc_IMPL=SSE2
                )
            elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
                set_target_properties(Vc::Vc PROPERTIES
                    INTERFACE_COMPILE_DEFINITIONS Vc_IMPL=Scalar
                )
            else ()
                set_target_properties(Vc::Vc PROPERTIES
                    INTERFACE_COMPILE_OPTIONS -msse4.2
                    INTERFACE_COMPILE_DEFINITIONS Vc_IMPL=SSE4_2
                )
            endif ()
        endif ()
    endif()
endif ()

set(GDXCORE_PUBLIC_HEADERS
    include/gdx/eigenio.h
    include/gdx/line.h
    include/gdx/log.h
    include/gdx/point.h
    include/gdx/exception.h
    include/gdx/maskedraster.h
    include/gdx/maskedrasterio.h
    include/gdx/maskedrasteriterator.h
    include/gdx/raster.h
    include/gdx/rasterarea.h
    include/gdx/rasterfwd.h
    include/gdx/rasterdiff.h
    include/gdx/rastercompare.h
    include/gdx/rasteriterator.h
    include/gdx/rastercelliterator.h
    include/gdx/rasterchecks.h
    include/gdx/sparseraster.h
    include/gdx/sparserasterio.h
    include/gdx/sparserasteriterator.h
    include/gdx/rasterutils-private.h
    include/gdx/cpupredicates-private.h
    include/gdx/nodatapredicates-private.h
    include/gdx/eigeniterationsupport-private.h
    include/gdx/rasterspan.h
    include/gdx/rasterspanio.h
)

if (GDX_ENABLE_SIMD)
    list(APPEND GDXCORE_PUBLIC_HEADERS
        include/gdx/denseraster.h
        include/gdx/denserasterio.h
        include/gdx/simd.h
    )
endif ()

add_library(gdxcore
    ${GDXCORE_PUBLIC_HEADERS}
    raster.cpp
)

add_library(geodynamix::gdxcore ALIAS gdxcore)

target_include_directories(gdxcore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

target_link_libraries(gdxcore PUBLIC
    Infra::infra
    geodynamix::gdxcommon
    geodynamix::gdxalgo
    Eigen3::Eigen
    $<TARGET_NAME_IF_EXISTS:Vc::Vc>
    $<TARGET_NAME_IF_EXISTS:OpenMP::OpenMP_CXX>
)
set_target_properties(gdxcore PROPERTIES PUBLIC_HEADER "${GDXCORE_PUBLIC_HEADERS}")

if (TARGET OpenMP::OpenMP_CXX)
    target_link_libraries(gdxcore PRIVATE OpenMP::OpenMP_CXX)
endif ()

if (GDX_INSTALL_DEVELOPMENT_FILES)
    install(TARGETS gdxcore EXPORT geodynamix-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gdx
    )
endif ()

if (MSVC)
    # add debugger visualisers for improved debugging info
    target_sources(gdxcore PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../GSL.natvis
        ${CMAKE_CURRENT_SOURCE_DIR}/../Eigen.natvis
    )
endif ()

if (GDX_ENABLE_TESTS)
    add_subdirectory(test)
endif ()
