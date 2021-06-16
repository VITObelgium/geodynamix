find_package(PythonInterp REQUIRED)

add_test(NAME pygdxtest
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/testgdx.py
)

if (MSVC)
    set_tests_properties(pygdxtest PROPERTIES ENVIRONMENT PYTHONPATH=${LIBRARY_OUTPUT_PATH}/$<CONFIG>)
else ()
    set_tests_properties(pygdxtest PROPERTIES ENVIRONMENT PYTHONPATH=${LIBRARY_OUTPUT_PATH})
endif ()
