#include "pythonutils.h"

#include "gdx/config.h"
#include "gdx/exception.h"
#include "infra/log.h"

namespace gdx {

namespace py = pybind11;
using namespace py::literals;

void set_gdal_path()
{
#if defined(GDX_DYNAMIC_BUILD) && defined(_WIN32)
    /*
    if 'USE_PATH_FOR_GDAL_PYTHON' in os.environ and 'PATH' in os.environ:
      for p in os.environ['PATH'].split(';'):
        if p:
          try:
            os.add_dll_directory(p)
          except (FileNotFoundError, OSError):
            continue
    */

    auto os = py::module::import("os");
    if (os.attr("environ")["USE_PATH_FOR_GDAL_PYTHON"] && os.attr("environ")["PATH"]) {
        auto paths = os.attr("environ")["PATH"].attr("split")(";");
        for (auto path : paths) {
            if (path) {
                try {
                    inf::Log::warn("Add to path: '{}'", std::string(py::str(path)));
                    os.attr("add_dll_directory")(path);
                } catch (const std::exception& e) {
                    inf::Log::error("Add to path error: '{}'", e.what());
                }
            }
        }
    }
#else
    inf::Log::warn("No gdal path modification");
#endif
}

const std::type_info& dtypeToRasterType(py::dtype type)
{
    auto typeStr = static_cast<std::string>(py::str(type));

    if (typeStr == "bool") return typeid(uint8_t);
    if (typeStr == "uint8") return typeid(uint8_t);
    if (typeStr == "int16") return typeid(int16_t);
    if (typeStr == "uint16") return typeid(uint16_t);
    if (typeStr == "int32") return typeid(int32_t);
    if (typeStr == "uint32") return typeid(uint32_t);
    // when saying int as datatype it is translated to int64 on 64bit linux
    // assume the client simply wants a 32bit integer
    if (typeStr == "int64") return typeid(int32_t);
    if (typeStr == "uint64") return typeid(uint32_t);
    if (typeStr == "float32") return typeid(float);
    if (typeStr == "float64") return typeid(double);

    throw InvalidArgument("Unsupported numpy data type {}", typeStr);
}

py::dtype rasterTypeToDtype(const std::type_info& type)
{
    if (type == typeid(uint8_t)) return py::dtype::of<uint8_t>();
    if (type == typeid(int16_t)) return py::dtype::of<int16_t>();
    if (type == typeid(uint16_t)) return py::dtype::of<uint16_t>();
    if (type == typeid(int32_t)) return py::dtype::of<int32_t>();
    if (type == typeid(uint32_t)) return py::dtype::of<uint32_t>();
    if (type == typeid(float)) return py::dtype::of<float>();
    if (type == typeid(double)) return py::dtype::of<double>();

    throw InvalidArgument("Invalid raster data type");
}

fs::path handle_path(py::object arg)
{
    if (py::isinstance<py::str>(arg)) {
        return fs::u8path(std::string(py::str(arg)));
    } else {
        return fs::u8path(std::string(py::str(arg.attr("__str__")())));
    }
}
}
