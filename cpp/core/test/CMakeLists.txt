add_executable(gdxcoretest
    eigeniterationtest.cpp
    operatorstest.cpp
    rasterareatest.cpp
    rastertest.cpp
    rasteriotest.cpp
    rasterspantest.cpp
    rastercomparetest.cpp
    rasteriteratortest.cpp
    sparserastertest.cpp
    testconfig.h.in
)

if (GDX_ENABLE_SIMD)
    target_sources(gdxcoretest PRIVATE denserastertest.cpp)
endif()

target_include_directories(gdxcoretest
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_definitions(gdxcoretest PRIVATE GDX_DOCTEST)
target_link_libraries(gdxcoretest gdxcore gdxtestutil infratestutilmain)
add_test(NAME gdxcoretest COMMAND gdxcoretest)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/testconfig.h)
