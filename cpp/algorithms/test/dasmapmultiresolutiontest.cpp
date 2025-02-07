#include "gdx/algo/dasmapmultiresolution.h"
#include "gdx/test/testbase.h"

namespace gdx::test {

TEST_CASE_TEMPLATE("DasMapMultiResolution", TypeParam, RasterTypes)
{
    SUBCASE("dasymetric_mapping_multiresolution")
    {
        RasterMetadata meta(1, 5, 0.0, 0.0, 100.0, -9999.0);
        const DenseRaster<int> landuse(meta, std::vector<int>{1, 1, 2, 2, 3});
        const std::vector<double> weights = {1.0, 1.0, 1.0, 1.0};
        const DenseRaster<int> zones(meta, std::vector<int>{1, 1, 1, 2, 2});
        const std::vector<double> amounts = {0.0, 1.0, 1.0};
        DenseRaster<float> actual         = gdx::dasymetric_mapping_multiresolution<float, DenseRaster, int, double>(landuse, weights, zones, amounts, meta);

        DenseRaster<float> expected(meta, std::vector<float>{1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f, 2.0f / 3.0f});
        auto nan = std::numeric_limits<float>::quiet_NaN();
        expected.set_nodata(nan);
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5);
    }

    SUBCASE("dasMap1MultiResolution")
    {
        RasterMetadata meta2(2, 10, 0.0, 0.0, 50.0, -9999.0);
        RasterMetadata meta(1, 5, 0.0, 0.0, 100.0, -9999.0);
        const DenseRaster<int> landuse(meta2, std::vector<int>{
                                                  1, 1, 1, 1, 2, 2, 2, 2, 3, 3,
                                                  1, 1, 1, 1, 2, 2, 2, 2, 3, 3});
        const std::vector<double> weights = {1.0, 1.0, 1.0, 1.0};
        const DenseRaster<int> zones(meta2, std::vector<int>{
                                                1, 1, 1, 1, 1, 1, 2, 2, 2, 2,
                                                1, 1, 1, 1, 1, 1, 2, 2, 2, 2});
        const std::vector<double> amounts = {0.0, 1.0, 1.0};
        DenseRaster<float> actual         = gdx::dasymetric_mapping_multiresolution<float, DenseRaster, int, double>(landuse, weights, zones, amounts, meta);

        DenseRaster<float> expected(meta, std::vector<float>{
                                              1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f, 2.0f / 3.0f});
        auto nan = std::numeric_limits<float>::quiet_NaN();
        expected.set_nodata(nan);
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5);
    }

    SUBCASE("dasMap2MultiResolutionWeiss")
    {
        RasterMetadata meta10(10, 10, 0.0, 0.0, 50.0, -9999);
        RasterMetadata meta(5, 5, 0.0, 0.0, 100.0, -9999);
        const DenseRaster<int> landuse(meta10, std::vector<int>{
                                                   0, 1, 2, 3, 3, -9999, 1, 2, -9999, -9999,
                                                   0, 1, 2, 3, 3, -9999, 1, 2, -9999, -9999,
                                                   0, 1, 2, 3, 3, -9999, 1, 2, -9999, -9999,
                                                   0, 1, 2, 3, 3, -9999, 1, 2, -9999, -9999,
                                                   0, 1, 2, 3, 3, -9999, 1, 2, -9999, -9999,
                                                   0, 1, 2, 3, 3, -9999, 1, 2, -9999, -9999,
                                                   0, 1, 2, 3, 3, -9999, 1, 2, -9999, -9999,
                                                   0, 1, 2, 3, 3, -9999, 1, 2, -9999, -9999,
                                                   0, 1, 2, 3, 3, -9999, 1, 2, -9999, -9999,
                                                   0, 1, 2, 3, 3, -9999, 1, 2, -9999, -9999});
        const std::vector<double> weights = {1.0, 0.0, 1.2, 1.3};
        const DenseRaster<int> zones(meta10, std::vector<int>{
                                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                                 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                                 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                                 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                                 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                                 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                                 -9999, -9999, -9999, -9999, -9999, -9999, -9999, -9999, -9999, -9999,
                                                 -9999, -9999, -9999, -9999, -9999, -9999, -9999, -9999, -9999, -9999});
        const std::vector<double> amounts = {1.0, 0.0, 100.0, 1000.0};
        DenseRaster<float> actual         = gdx::dasymetric_mapping_multiresolution<float, DenseRaster, int, double>(landuse, weights, zones, amounts, meta);

        auto nan = std::numeric_limits<float>::quiet_NaN();
        DenseRaster<float> expected(meta, std::vector<float>{
                                              0.2857142984867096, 0.3571428656578064, 0.18571427464485168, 0.17142857611179352, nan,
                                              0.0, 0.0, 0.0, 0.0, nan,
                                              28.571430206298828, 35.71428680419922, 18.571428298950195, 17.142858505249023, nan,
                                              285.71429443359375, 357.1428527832031, 185.7142791748047, 171.42857360839844, nan,
                                              nan, nan, nan, nan, nan});
        expected.set_nodata(nan);
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5);
    }
}
}
