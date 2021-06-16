#include "gdx/raster.h"

#include "infra/colormap.h"
#include "infra/exception.h"
#include "infra/log.h"
#include "infra/gdal.h"
#include "infra/gdallog.h"
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <lyra/lyra.hpp>

#include <regex>

using Log     = inf::Log;
namespace cli = lyra;
using namespace std::string_literals;

int main(int argc, char* argv[])
{
    try {
        Log::add_console_sink(Log::Colored::On);
        inf::LogRegistration logReg("gdxconvert");
#ifdef NDEBUG
        Log::set_level(Log::Level::Info);
#else
        Log::set_level(Log::Level::Debug);
#endif

        struct Cli
        {
            std::string inputRaster, outputRaster;
            bool showHelp = false;
            std::optional<int> epsg;
            std::string type;
            std::string colorMap;
        } options;

        auto cli = cli::help(options.showHelp) |
                   cli::opt([&](int tol) { options.epsg = tol; }, "number")["-e"]["--epsg"]("Modify epsg") |
                   cli::opt([&](std::string s) {
                       if (std::regex_match(s, std::regex("^(byte|int|float|double)$"))) {
                           options.type = s;
                           return cli::parser_result::ok(cli::parser_result_type::matched);
                       } else {
                           return cli::parser_result::runtimeError(cli::parser_result_type::no_match, "Type must match : byte|int|float|double");
                       }
                   },
                       "type")["-t"]["--type"]("Change type (byte, int, float, double)") |
                   cli::opt(options.colorMap, "value")["-c"]["--color-map"]("Color map of the image output") | cli::arg(options.inputRaster, "input")("input raster") | cli::arg(options.outputRaster, "output")("output raster");

        auto result = cli.parse(cli::args(argc, argv));
        if (!result) {
            Log::error("Error in command line: {}", result.errorMessage());
            return EXIT_FAILURE;
        }

        if (options.showHelp || argc == 1) {
            fmt::print("{}", cli);
            return EXIT_SUCCESS;
        }

        inf::gdal::Registration reg;
        inf::gdal::set_log_handler();
        auto raster = gdx::Raster::read(options.inputRaster);
        if (options.epsg.has_value()) {
            raster.set_projection(options.epsg.value());
        }

        auto outputPath = fs::u8path(options.outputRaster);
        if (!options.colorMap.empty()) {
            auto outputFormat = inf::gdal::guess_rastertype_from_filename(outputPath);
            switch (outputFormat) {
            case inf::gdal::RasterType::Gif:
            case inf::gdal::RasterType::Png:
                break;
            default:
                throw inf::RuntimeError("Color maps are only supported for image outputs");
            }

            if (options.type != "byte" && !options.type.empty()) {
                Log::warn("Color mapped image outputs always use byte output");
            }

            raster.writeColorMapped(outputPath, inf::ColorMap::create(options.colorMap));
        } else {
            if (!options.type.empty()) {
                if (options.type == "float") {
                    raster.write(outputPath, typeid(float));
                } else if (options.type == "double") {
                    raster.write(outputPath, typeid(double));
                } else if (options.type == "int") {
                    raster.write(outputPath, typeid(int32_t));
                } else if (options.type == "byte") {
                    raster.write(outputPath, typeid(uint8_t));
                } else {
                    throw inf::RuntimeError("Invalid raster type: {}", options.type);
                }
            } else {
                raster.write(outputPath);
            }
        }

        return EXIT_SUCCESS;
    } catch (const std::bad_alloc&) {
        Log::critical("{}: Out of memory", argv[0]);
    } catch (const std::exception& e) {
        Log::error("{}: {}", argv[0], e.what());
    }

    return EXIT_FAILURE;
}
