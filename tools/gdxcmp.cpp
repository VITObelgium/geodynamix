#include "gdx/raster.h"
#include "gdx/rasterdiff.h"
#include "gdx/rasteriterator.h"

#include <algorithm>
#include <cmath>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <iostream>
#include <limits>
#include <lyra/lyra.hpp>
#include <optional>

namespace cli = lyra;
using namespace std::string_literals;

bool compareRasters(const gdx::Raster& raster1, const gdx::Raster& raster2, float tolerance, bool ignoreMetadata, bool verbose)
{
    if (!ignoreMetadata) {
        if (raster1.metadata() != raster2.metadata()) {
            fmt::print(fg(fmt::color::red), "Metadata mismatch:\n{}\n{}\n", raster1.metadata().to_string(), raster2.metadata().to_string());
            return false;
        }
    }

    return std::visit([tolerance, verbose](auto&& r1, auto&& r2) {
        auto diff = gdx::diff_rasters(r1, r2, tolerance);

        if (diff.differentCells() == 0) {
            fmt::print(fg(fmt::color::green), "Rasters are equal!\n");
            return true;
        }

        fmt::print(fg(fmt::color::green), "# matches:\t\t{}\n", diff.equal);

        if (diff.dataDifference) {
            fmt::print(fg(fmt::color::red), "# mismatches:\t{}\n", diff.dataDifference);
        }

        if (diff.zeroToNonZero) {
            fmt::print(fg(fmt::color::red), "# zero -> non zero:\t{}\n", diff.zeroToNonZero);
        }

        if (diff.nonZeroToZero) {
            fmt::print(fg(fmt::color::red), "# non zero -> zero:\t{}\n", diff.nonZeroToZero);
        }

        if (diff.zeroToNodata) {
            fmt::print(fg(fmt::color::yellow), "# zero -> nodata:\t{}\n", diff.zeroToNodata);
        }

        if (diff.nonZeroToNodata) {
            fmt::print(fg(fmt::color::yellow), "# non zero -> nodata:\t{}\n", diff.nonZeroToNodata);
        }

        if (diff.nodataToZero) {
            fmt::print(fg(fmt::color::yellow), "# nodata -> zero:\t{}\n", diff.nodataToZero);
        }

        if (diff.nodataToNonZero) {
            fmt::print(fg(fmt::color::yellow), "# nodata -> non zero:\t{}\n", diff.nodataToNonZero);
        }

        if (verbose) {
            using WidestType = decltype(*r1.data() * *r2.data());
            auto pred        = gdx::cpu::float_equal_to<WidestType>(static_cast<WidestType>(tolerance));
            for (int r = 0; r < r1.rows(); r++) {
                for (int c = 0; c < r1.cols(); c++) {
                    if (!r1.is_nodata(r, c) && !r2.is_nodata(r, c) && !pred(static_cast<WidestType>(r1(r, c)), static_cast<WidestType>(r2(r, c)))) {
                        fmt::print(fg(fmt::color::yellow), "value difference at cell ({},{}): {} != {}\n", r, c, r1(r, c), r2(r, c));
                    }
                }
            }
        }

        return false;
    },
                      raster1.get(), raster2.get());
}

int main(int argc, char* argv[])
{
    try {
        struct Options
        {
            bool checkMeta = false;
            bool showHelp  = false;
            bool verbose   = false;
            std::optional<float> tolerance;
            std::string expectedRaster, actualRaster;
        } options;

        auto cli = cli::help(options.showHelp) |
                   cli::opt(options.checkMeta)["-m"]["--check-meta"]("Check for metadata differences") |
                   cli::opt(options.verbose)["-v"]["--verbose"]("Verbose output") |
                   cli::opt([&](float tol) { options.tolerance = tol; }, "number")["-f"]["--floating-point-tolerance"]("Use floating point comparison with given tolerance") |
                   cli::arg(options.expectedRaster, "expected")("Reference raster") |
                   cli::arg(options.actualRaster, "actual")("Actual raster");
        ;

        auto result = cli.parse(cli::args(argc, argv));
        if (!result) {
            fmt::print(fg(fmt::color::red), "Error in command line: {}", result.message());
            return EXIT_FAILURE;
        }

        if (options.showHelp || argc == 1) {
            fmt::print("{}", fmt::streamed(cli));
            return EXIT_SUCCESS;
        }

        inf::gdal::Registration reg;
        auto raster1 = gdx::Raster::read(options.expectedRaster);
        auto raster2 = gdx::Raster::read(options.actualRaster);

        if (!compareRasters(raster1, raster2, options.tolerance.value_or(0.f), !options.checkMeta, options.verbose)) {
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    } catch (const std::bad_alloc&) {
        fmt::print(fg(fmt::color::red), "{}: Out of memory\n", argv[0]);
    } catch (const std::exception& e) {
        fmt::print(fg(fmt::color::red), "{}: {}\n", argv[0], e.what());
    }

    return EXIT_FAILURE;
}
