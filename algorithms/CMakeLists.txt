set(GDXALGO_PUBLIC_HEADERS
    include/gdx/algo/addpoints.h
    include/gdx/algo/accuflux.h
    include/gdx/algo/aggregatemultiresolution.h
    include/gdx/algo/aggregateandspreadmultiresolution.h
    include/gdx/algo/algorithm.h
    include/gdx/algo/arealweighteddistribution.h
    include/gdx/algo/blurfilter.h
    include/gdx/algo/bufferstyle.h
    include/gdx/algo/cast.h
    include/gdx/algo/category.h
    include/gdx/algo/choropleth.h
    include/gdx/algo/clip.h
    include/gdx/algo/clusterid.h
    include/gdx/algo/clustersize.h
    include/gdx/algo/clusterutils.h
    include/gdx/algo/conversion.h
    include/gdx/algo/colormap.h
    include/gdx/algo/conditionals.h
    include/gdx/algo/dasline.h
    include/gdx/algo/dasmap.h
    include/gdx/algo/dasmaplineairdistribution.h
    include/gdx/algo/dasmapmultiresolution.h
    include/gdx/algo/distance.h
    include/gdx/algo/distribute.h
    include/gdx/algo/filter.h
    include/gdx/algo/logical.h
    include/gdx/algo/majorityfilter.h
    include/gdx/algo/masking.h
    include/gdx/algo/mathoperations.h
    include/gdx/algo/maximum.h
    include/gdx/algo/minimum.h
    include/gdx/algo/multiresolution.h
    include/gdx/algo/nodata.h
    include/gdx/algo/normalise.h
    include/gdx/algo/propdist.h
    include/gdx/algo/random.h
    include/gdx/algo/rasterize.h
    include/gdx/algo/rasterizelineantialiased.h
    include/gdx/algo/reclass.h
    include/gdx/algo/shape.h
    include/gdx/algo/statistics.h
    include/gdx/algo/categoryio.h
    include/gdx/algo/sum.h
    include/gdx/algo/suminbuffer.h
    include/gdx/algo/tablerow.h
    include/gdx/algo/voronoi.h
    include/gdx/algo/weightedproduct.h
    include/gdx/algo/weightedpropdist.h
    include/gdx/algo/weighteddistribution.h
    include/gdx/algo/weightedsum.h
)

add_library(gdxalgo
    ${GDXALGO_PUBLIC_HEADERS}
    accuflux.cpp
    reclass.cpp
    rasterizelineantialiased.cpp
    shape.cpp
    suminbuffer.cpp
    categoryio.cpp
)

add_library(Gdx::gdxalgo ALIAS gdxalgo)

target_include_directories(gdxalgo PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(gdxalgo PUBLIC
    Gdx::gdxcommon
    $<TARGET_NAME_IF_EXISTS:OpenMP::OpenMP_CXX>
)
set_target_properties(gdxalgo PROPERTIES PUBLIC_HEADER "${GDXALGO_PUBLIC_HEADERS}")


if (GDX_INSTALL_DEVELOPMENT_FILES)
    install(TARGETS gdxalgo EXPORT gdx-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gdx/algo
    )
endif ()

if (GDX_ENABLE_TESTS)
    add_subdirectory(test)
endif ()
