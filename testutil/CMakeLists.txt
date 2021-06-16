set (GDX_TESTUTIL_PUBLIC_HEADERS
    include/gdx/test/comparisons.h
    include/gdx/test/platformsupport.h
    include/gdx/test/printsupport.h
    include/gdx/test/rasterasserts.h
    include/gdx/test/testbase.h
)

add_library(gdxtestutil
    ${GDX_TESTUTIL_PUBLIC_HEADERS}
    testbase.cpp
)

add_library(Gdx::gdxtestutil ALIAS gdxtestutil)

set_target_properties(gdxtestutil PROPERTIES
    PUBLIC_HEADER "${GDX_TESTUTIL_PUBLIC_HEADERS}"
)

target_include_directories(gdxtestutil PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(gdxtestutil PUBLIC Gdx::gdxcore Infra::infratestutil doctest::doctest)

if (GDX_INSTALL_DEVELOPMENT_FILES)
    install(TARGETS gdxtestutil EXPORT gdx-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gdx/test
    )
endif ()
