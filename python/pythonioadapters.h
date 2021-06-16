#pragma once

#include <pybind11/pybind11.h>

#if PYBIND11_VERSION_MAJOR == 2 && PYBIND11_VERSION_MINOR >= 6
#define GDX_PYBIND_MODULE module_
#else
#define GDX_PYBIND_MODULE module
#endif

namespace gdx {

void initIoModule(pybind11::GDX_PYBIND_MODULE& ioMod);
}
