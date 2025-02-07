set(PYTHON_TESTS
    test/testgdx.py
)

# make sure the pybind11 detected python version matches
set(PYBIND11_PYTHON_VERSION ${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR})
set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

add_library(gdxpy MODULE
    gdxpy.cpp
    pythonadapters.h
    pythonadapters.cpp
    rasterargument.h
    rasterargument.cpp
    pythonutils.h
    pythonutils.cpp
    pythonioadapters.h
    pythonioadapters.cpp
    pythonalgoadapters.h
    pythonalgoadapters.cpp
    pythonlogsink.h
    pythonlogsink.cpp
    ${PYTHON_TESTS}
)

if(WIN32)
    target_sources(gdxpy PRIVATE ${CMAKE_BINARY_DIR}/version.rc)
endif()

target_include_directories(gdxpy
    PRIVATE ${PYTHON_INCLUDE_DIRS}
)

source_group("Python files" FILES ${PYTHON_TESTS})

message(STATUS "Using python from ${PYTHON_INCLUDE_DIRS}")
message(STATUS "Using python library ${PYTHON_LIBRARIES}")

target_link_libraries(gdxpy PRIVATE
    gdxcore
    gdxalgo
    pybind11::module
)

if(TARGET OpenMP::OpenMP_CXX)
    target_link_libraries(gdxpy PRIVATE OpenMP::OpenMP_CXX)
endif()

if(MINGW AND NOT PYTHON_MODULE_EXTENSION)
    # detection of module extension does not seem to work on MINGW
    set(PYTHON_MODULE_EXTENSION -cpython-${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}m.dll)
endif()

message(STATUS "Python extension: ${PYTHON_MODULE_EXTENSION}")

set_target_properties(gdxpy PROPERTIES
    DEBUG_POSTFIX ""
    OUTPUT_NAME geodynamix
    PREFIX "${PYTHON_MODULE_PREFIX}"
    SUFFIX "${PYTHON_MODULE_EXTENSION}"
)

if(UNIX)
    set(INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR})
else()
    set(INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
endif()

install(TARGETS gdxpy
    LIBRARY DESTINATION ${INSTALL_DIR}
    RUNTIME DESTINATION ${INSTALL_DIR}
)

add_subdirectory(test)
