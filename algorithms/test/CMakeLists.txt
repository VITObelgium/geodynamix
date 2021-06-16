add_executable(gdxalgotest
    accufluxtest.cpp
    algorithmtest.cpp    
    casttest.cpp
    cliptest.cpp
    categoryiotest.cpp
    choroplethtest.cpp
    clusteridtest.cpp
    clustersizetest.cpp
    conditionalstest.cpp
    distancetest.cpp
    distributetest.cpp
    filtertest.cpp
    logicaltest.cpp
    majoritytest.cpp
    minimumtest.cpp
    maskingtest.cpp
    mathoperationstest.cpp
    maximumtest.cpp
    nodatatest.cpp
    normalisetest.cpp
    propdisttest.cpp
    rasterizelineantialiasedtest.cpp
    reclasstest.cpp
    suminbuffertest.cpp
    sumwithintraveldistancetest.cpp
    sumtest.cpp
    tablerowtest.cpp
    voronoitest.cpp
    weighteddistributiontest.cpp
    weightedproducttest.cpp
    weightedpropdisttest.cpp
    weightedsumtest.cpp
)

if (GDX_ENABLE_SIMD)
    #rework these test to operate on dense AND masked rasters
    target_sources(gdxalgotest PRIVATE
        addpointstest.cpp
        aggregatemultiresolutiontest.cpp
        aggregateandspreadmultiresolutiontest.cpp
        arealweighteddistributiontest.cpp
        categorytest.cpp
        daslinetest.cpp
        dasmaptest.cpp
        dasmaplineairdistributiontest.cpp
        dasmapmultiresolutiontest.cpp
    )
endif ()

target_include_directories(gdxalgotest PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(gdxalgotest PRIVATE gdxtestutil gdxalgo gdxcore gdxtestutil Infra::infratestutilmain)

if (TARGET OpenMP::OpenMP_CXX)
    target_link_libraries(gdxalgotest PRIVATE OpenMP::OpenMP_CXX)
endif ()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/testconfig.h)

add_test(NAME gdxalgotest COMMAND gdxalgotest)
