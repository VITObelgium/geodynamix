#include "gdx/algo/polygoncoverage.h"
#include "gdx/test/testbase.h"
#include "infra/crs.h"
#include "infra/gdalalgo.h"
#include "infra/gdalio.h"

#include "testconfig.h"

namespace gdx::test {

using namespace inf;
namespace gdal = inf::gdal;

TEST_CASE("PolygonCoverage")
{
    auto boundaries = file::u8path(TEST_DATA_DIR) / "boundaries.gpkg";

    auto ds = gdal::VectorDataSet::open(boundaries);
    GeoMetadata outputExtent(120, 260, 11000.0, 140000.0, {1000.0, -1000.0}, std::numeric_limits<double>::quiet_NaN(), gdal::SpatialReference(crs::epsg::BelgianLambert72).export_to_wkt());

    auto warpedMeta      = gdal::warp_metadata(outputExtent, crs::epsg::WGS84);
    const auto coverages = create_polygon_coverages(warpedMeta, ds, gdx::BorderHandling::None, 1.0, {}, {}, "Code3", nullptr);

    CHECK(coverages.size() == 3);

    for (auto& coverage : coverages) {
        auto totalCoverage = std::accumulate(coverages[0].cells.begin(), coverages[0].cells.end(), 0.0, [](double sum, const PolygonCellCoverage::CellInfo& cellInfo) {
            return sum + cellInfo.coverage;
        });

        if (coverage.name == "BEB") {
            CHECK(coverage.cells.size() == 145);

            auto cellIter = std::find_if(coverage.cells.begin(), coverage.cells.end(), [](auto& c) { return c.computeGridCell == Cell(55, 147); });
            CHECK(cellIter->cellCoverage == Approx(0.6037847694229548));

        } else if (coverage.name == "BEF") {
            CHECK(coverage.cells.size() == 10053);

            auto cellIter = std::find_if(coverage.cells.begin(), coverage.cells.end(), [](auto& c) { return c.computeGridCell == Cell(55, 147); });
            CHECK(cellIter->cellCoverage == Approx(0.3962152305751532));
        } else if (coverage.name == "NL") {
            CHECK(coverage.cells.size() == 28072);
            // This cell does not overlap NL
            CHECK(std::find_if(coverage.cells.begin(), coverage.cells.end(), [](auto& c) { return c.computeGridCell == Cell(55, 147); }) == coverage.cells.end());
        } else {
            CHECK_FALSE_MESSAGE("Unexpected polygon name", coverage.name);
        }

        CHECK_MESSAGE(totalCoverage == Approx(1.0), "Coverages for ", coverage.name, " do not add up to 1");
    }
}
}
