#include "gdx/test/testbase.h"

#include "gdx/algo/rasterizelineantialiased.h"
#include "infra/crs.h"

namespace gdx::test {

using namespace inf;

TEST_CASE_TEMPLATE("RasterizeLineAntiAliased", TypeParam, UnspecializedRasterTypes)
{
    using DoubleRaster = typename TypeParam::template type<double>;

    SUBCASE("rasterizeSegmentAntiAliasedTest")
    {
        RasterMetadata meta(5, 5, 0.0, 0.0, 100.0, -9999.0);
        DoubleRaster actual(meta, 0);
        double xStart = 50.0, yStart = 50.0, xEnd = 350.0, yEnd = 150.0;
        std::vector<std::pair<Cell, float /*brightness*/>> locs;
        details::rasterize_segment_anti_aliased(xStart, yStart, xEnd, yEnd, meta, locs);
        for (auto& loc : locs) {
            int ry = loc.first.r, cx = loc.first.c;
            actual(ry, cx) += loc.second;
        }

        DoubleRaster expected(meta, std::vector<double>{
                                        0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0,
                                        0, 1.0 / 3.0, 2.0 / 3.0, 0.5, 0,
                                        0.5, 2.0 / 3.0, 1.0 / 3.0, 0, 0});
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
    }

    SUBCASE("rasterizeSegmentsAntiAliasedTest")
    {
        RasterMetadata meta(5, 5, 0.0, 0.0, 100.0, -9999.0);
        std::vector<std::vector<float>> raster;
        raster.resize(meta.rows);
        for (int ry = 0; ry < meta.rows; ++ry) {
            raster[ry].assign(meta.cols, 0.0f);
        }
        std::vector<std::vector<Point<float>>> endPoints = {{{50.0f, 50.0f}, {350.0f, 150.0f}, {350.0f, 350.0f}}};
        float value                                      = 5.0f;
        details::rasterize_segments_anti_aliased(endPoints, value, false, meta, raster);
        endPoints = {{{50.0f, 350.0f}, {200.0f, 350.0f}}};
        value     = 7.0f;
        details::rasterize_segments_anti_aliased(endPoints, value, false, meta, raster);
        DoubleRaster actual(meta);
        for (int ry = 0; ry < meta.rows; ++ry) {
            for (int cx = 0; cx < meta.cols; ++cx) {
                actual(ry, cx) = raster[ry][cx];
            }
        }

        DoubleRaster expected(meta, std::vector<double>{
                                        0, 0, 0, 0, 0,
                                        7 * 0.5, 7.0 * 1.0, 0, 5.0 * 0.5, 0,
                                        0, 0, 0, 5.0 * 1.0, 0,
                                        0, 5.0 * 1.0 / 3.0, 5.0 * 2.0 / 3.0, 5.0 * 1.0, 0,
                                        5 * 0.5, 5.0 * 2.0 / 3.0, 5.0 * 1.0 / 3.0, 0, 0});
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
    }
}

TEST_CASE("RasterizeLineAntiAliased.reprojectsLines")
{
    RasterMetadata meta(5, 5, 150000.0, 150000.0, 100.0, 0.0);
    meta.set_projection_from_epsg(crs::epsg::BelgianLambert72);

    auto driver = gdal::VectorDriver::create(gdal::VectorType::Memory);
    auto source = driver.create_dataset("source");
    auto target = driver.create_dataset("target");
    gdal::SpatialReference sourceProjection(crs::epsg::WGS84);
    gdal::SpatialReference targetProjection(crs::epsg::BelgianLambert72);
    auto sourceLayer = source.create_layer("lines", sourceProjection, gdal::Geometry::Type::Unknown);
    auto targetLayer = target.create_layer("lines", targetProjection, gdal::Geometry::Type::Unknown);
    auto field = gdal::FieldDefinition::create<double>("value");
    sourceLayer.create_field(field);
    targetLayer.create_field(field);

    OGRLineString sourceLine1, targetLine1, sourceLine2, targetLine2;
    auto addPoint = [&](OGRLineString& sourceLine, OGRLineString& targetLine, double x, double y) {
        targetLine.addPoint(x, y);
        auto wgs84Point = gdal::convert_point_projected(crs::epsg::BelgianLambert72, crs::epsg::WGS84, Point<double>(x, y));
        sourceLine.addPoint(wgs84Point.x, wgs84Point.y);
    };
    addPoint(sourceLine1, targetLine1, 150120.0, 150130.0);
    addPoint(sourceLine1, targetLine1, 150240.0, 150280.0);
    addPoint(sourceLine1, targetLine1, 150360.0, 150340.0);
    addPoint(sourceLine2, targetLine2, 150160.0, 150360.0);
    addPoint(sourceLine2, targetLine2, 150290.0, 150260.0);

    gdal::Feature sourceFeature(sourceLayer.layer_definition());
    gdal::Feature targetFeature(targetLayer.layer_definition());
    SUBCASE("Line")
    {
        sourceFeature.set_geometry(gdal::LineCRef(&sourceLine1));
        targetFeature.set_geometry(gdal::LineCRef(&targetLine1));
    }
    SUBCASE("MultiLine")
    {
        OGRMultiLineString sourceMultiLine, targetMultiLine;
        sourceMultiLine.addGeometry(&sourceLine1);
        sourceMultiLine.addGeometry(&sourceLine2);
        targetMultiLine.addGeometry(&targetLine1);
        targetMultiLine.addGeometry(&targetLine2);
        sourceFeature.set_geometry(gdal::MultiLineCRef(&sourceMultiLine));
        targetFeature.set_geometry(gdal::MultiLineCRef(&targetMultiLine));
    }
    sourceFeature.set_field("value", 10.0);
    targetFeature.set_field("value", 10.0);
    sourceLayer.create_feature(sourceFeature);
    targetLayer.create_feature(targetFeature);

    auto actual   = gdx::rasterize_lines_anti_aliased<DenseRaster<float>>(sourceLayer, meta, "value", true, true);
    auto expected = gdx::rasterize_lines_anti_aliased<DenseRaster<float>>(targetLayer, meta, "value", true, true);
    CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-3f);
}
}
