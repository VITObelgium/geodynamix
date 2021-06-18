set(GDXCOMMON_PUBLIC_HEADERS
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>$<INSTALL_INTERFACE:include>/gdx/cell.h
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>$<INSTALL_INTERFACE:include>/gdx/line.h
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>$<INSTALL_INTERFACE:include>/gdx/log.h
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>$<INSTALL_INTERFACE:include>/gdx/rastermetadata.h
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>$<INSTALL_INTERFACE:include>/gdx/point.h
)

add_library(gdxcommon INTERFACE)
target_sources(gdxcommon INTERFACE ${GDXCOMMON_PUBLIC_HEADERS})
if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.15)
    set_target_properties(gdxcommon PROPERTIES PUBLIC_HEADER "${GDXCOMMON_PUBLIC_HEADERS}")
endif ()

add_library(geodynamix::gdxcommon ALIAS gdxcommon)

if(MSVC)
    # make sure visual studio creates a project with the sources
    add_custom_target(gdxcommon.headeronly SOURCES ${GDXCOMMON_PUBLIC_HEADERS})
endif()

target_include_directories(gdxcommon
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

if (GDX_INSTALL_DEVELOPMENT_FILES)
    install(TARGETS gdxcommon EXPORT geodynamix-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gdx
    )
endif ()

target_link_libraries(gdxcommon INTERFACE Infra::infra)
