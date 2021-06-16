#include "gdx/algo/propdist.h"
#include "gdx/test/testbase.h"
#include "infra/gdalio.h"

#include "testconfig.h"

namespace gdx::test {

using namespace inf;

TEST_CASE_TEMPLATE("Proportional distribution", TypeParam, UnspecializedRasterTypes)
{
    using FloatRaster = typename TypeParam::template type<float>;

    SUBCASE("propDistWeiss")
    {
        RasterMetadata meta(5, 5, 0.0, 0.0, 100.0, -9999.0);
        gdal::VectorDataSet lines = gdal::VectorDataSet::open(TEST_DATA_DIR "/propdist.shp", gdal::VectorType::ShapeFile);
        auto actual               = gdx::proportional_distribution_aa<FloatRaster>(lines.layer(0), meta, "value");

        meta.nodata = 0.0;
        FloatRaster expected(meta, std::vector<float>{
                                       2.00976300239563f, 2.0147697925567627f, 0, 0, 0,
                                       3.167771816253662f, 1.2201496362686157f, 0, 0, 0,
                                       8.1311616897583f, 2.710212469100952f, 0.1630522906780243f, 0, 0,
                                       7.029526233673096f, 6.341089248657227f, 8.64463996887207f, 6.66719388961792f, 4.070267677307129f,
                                       0.5446348190307617f, 0, 0, 2.140498638153076f, 4.145268440246582f});
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
    }

    SUBCASE("propDistWeiss2")
    {
        RasterMetadata meta(5, 5, 0.0, 0.0, 100.0, -9999.0);
        gdal::VectorDataSet lines = gdal::VectorDataSet::open(TEST_DATA_DIR "/propdist2.shp", gdal::VectorType::ShapeFile);
        auto actual               = gdx::proportional_distribution_aa<FloatRaster>(lines.layer(0), meta, "value");

        meta.nodata = 0.0;
        FloatRaster expected(meta, std::vector<float>{
                                       5.607907295227051f, 2.170300006866455f, 0, 0, 0,
                                       6.775643825531006f, 8.047548294067383f, 0, 0, 0,
                                       2.391568660736084f, 12.431623458862305f, 0, 0, 0,
                                       3.410076856613159f, 6.010531425476074f, 5.402583599090576f, 0, 0,
                                       1.9907413721084595f, 0, 4.638828754425049f, 0.122648686170578f, 0});
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
    }

    SUBCASE("propDistWeiss3")
    {
        auto meta                 = gdal::io::read_metadata(TEST_DATA_DIR "/propdist3_meta.tif");
        gdal::VectorDataSet lines = gdal::VectorDataSet::open(TEST_DATA_DIR "/propdist3.shp", gdal::VectorType::ShapeFile);
        auto actual               = gdx::proportional_distribution_aa<FloatRaster>(lines.layer(0), meta, "Nombre_km");

        FloatRaster expected;
        gdx::read_raster(TEST_DATA_DIR "/reference/propdist3.tif", expected);
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
    }
}
}
