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

TEST_CASE("PolygonCoverage.reprojectsInputPolygons")
{
    GeoMetadata outputExtent(3, 3, 150000.0, 150000.0, 100.0, 0.0);
    outputExtent.set_projection_from_epsg(crs::epsg::BelgianLambert72);

    auto driver = gdal::VectorDriver::create(gdal::VectorType::Memory);
    auto ds     = driver.create_dataset("polygons");
    gdal::SpatialReference sourceProjection(crs::epsg::WGS84);
    auto layer = ds.create_layer("polygons", sourceProjection, gdal::Geometry::Type::Polygon);

    OGRLinearRing ring;
    for (const auto& point : {Point<double>(150120.0, 150120.0), Point<double>(150180.0, 150120.0),
                              Point<double>(150180.0, 150180.0), Point<double>(150120.0, 150180.0),
                              Point<double>(150120.0, 150120.0)}) {
        auto wgs84Point = gdal::convert_point_projected(crs::epsg::BelgianLambert72, crs::epsg::WGS84, point);
        ring.addPoint(wgs84Point.x, wgs84Point.y);
    }
    OGRPolygon polygon;
    polygon.addRing(&ring);
    gdal::Feature feature(layer.layer_definition());
    feature.set_geometry(gdal::PolygonCRef(&polygon));
    layer.create_feature(feature);

    GeoMetadata unrelatedExtent(3, 3, 0.0, 0.0, 1.0, 0.0);
    unrelatedExtent.set_projection_from_epsg(crs::epsg::WGS84);
    CHECK(create_polygon_coverages(unrelatedExtent, ds, BorderHandling::None, 1.0, {}, {}, {}, nullptr).empty());

    const auto coverages = create_polygon_coverages(outputExtent, ds, BorderHandling::None, 1.0, {}, {}, {}, nullptr);
    auto* spatialFilter  = layer.get()->GetSpatialFilter();
    REQUIRE(spatialFilter != nullptr);
    OGREnvelope filterEnvelope, polygonEnvelope;
    spatialFilter->getEnvelope(&filterEnvelope);
    polygon.getEnvelope(&polygonEnvelope);
    CHECK(filterEnvelope.MinX <= polygonEnvelope.MinX);
    CHECK(filterEnvelope.MaxX >= polygonEnvelope.MaxX);
    CHECK(filterEnvelope.MinY <= polygonEnvelope.MinY);
    CHECK(filterEnvelope.MaxY >= polygonEnvelope.MaxY);
    REQUIRE(coverages.size() == 1);
    REQUIRE(coverages.front().cells.size() == 1);
    CHECK(coverages.front().cells.front().computeGridCell == Cell(1, 1));
    CHECK(coverages.front().cells.front().cellCoverage == Approx(0.36).epsilon(0.001));
}
}
