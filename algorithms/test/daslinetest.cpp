#include "gdx/algo/dasline.h"
#include "gdx/test/testbase.h"

#include "testconfig.h"

namespace gdx::test {

TEST_CASE("DasLine.dasLineWeiss")
{
    RasterMetadata meta(5, 5, 0.0, 0.0, 100.0, -9999.0);
    const DenseRaster<int> zones(meta, std::vector<int>{
                                           0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1,
                                           2, 2, 2, 2, 2,
                                           3, 3, 3, 3, 3,
                                           -9999, -9999, -9999, -9999, -9999});
    inf::gdal::VectorDataSet lines                = inf::gdal::VectorDataSet::open(TEST_DATA_DIR "/dasline.shp", gdal::VectorType::ShapeFile);
    const std::unordered_map<int, double> amounts = {{0, 1}, {1, 0}, {2, 100}, {3, 1000}};
    auto actual                                   = gdx::linear_distribution_aa<float>(zones, lines.layer(0), amounts);

    meta.nodata = 0.0;
    DenseRaster<float> expected(meta, std::vector<float>{
                                          0.51621127128601074219f, 0.4837887883186340332f, 0.0f, 0.0f, 0.0f,
                                          0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                          27.846202850341796875f, 72.1537933349609375f, 0.0f, 0.0f, 0.0f,
                                          401.93011474609375f, 342.774993896484375f, 255.2948760986328125f, 0.0f, 0.0f,
                                          0, 0, 0, 0, 0});
    CHECK(actual.metadata() == expected.metadata());
    CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
}

TEST_CASE("DasLine.dasLineWeiss2")
{
    RasterMetadata meta(5, 5, 0.0, 0.0, 100.0, -9999.0);
    const DenseRaster<int> zones(meta, std::vector<int>{
                                           0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1,
                                           2, 2, 2, 2, 2,
                                           3, 3, 3, 3, 3,
                                           -9999, -9999, -9999, -9999, -9999});
    inf::gdal::VectorDataSet lines                = inf::gdal::VectorDataSet::open(TEST_DATA_DIR "/dasline2.shp", gdal::VectorType::ShapeFile);
    const std::unordered_map<int, double> amounts = {{0, 1}, {1, 0}, {2, 100}, {3, 1000}};
    auto actual                                   = gdx::linear_distribution_aa<float>(zones, lines.layer(0), amounts);

    meta.nodata = 0.0;
    DenseRaster<float> expected(meta, std::vector<float>{
                                          0.499377965927124, 0.500622034072876, 0, 0, 0,
                                          0, 0, 0, 0, 0,
                                          79.78083801269531, 19.162033081054688, 1.0571200847625732, 0, 0,
                                          293.98297119140625, 174.0420379638672, 237.26693725585938, 182.99253845214844, 111.71546936035156,
                                          0, 0, 0, 0, 0});
    CHECK(actual.metadata() == expected.metadata());
    CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
}
}
