#include "gdx/algo/arealweighteddistribution.h"
#include "gdx/test/testbase.h"

namespace gdx::test {

TEST_CASE("ArealWeightedDistribution.arealWeightedDistribution")
{
    RasterMetadata meta(1, 5, 0.0, 0.0, 100.0, -9999.0);
    const DenseRaster<int> zones(meta, std::vector<int>{1, 1, 1, 2, 2});
    const std::unordered_map<int, double> amounts = {{1, 50}, {2, 90}};
    auto actual                                   = gdx::areal_weighted_distribution<float>(zones, amounts);

    meta.nodata = 0.0;
    DenseRaster<float> expected(meta, std::vector<float>{50.0f / 3.0f, 50.0f / 3.0f, 50.0f / 3.0f, 90.0f / 2.0f, 90.0f / 2.0f});
    CHECK(actual.metadata() == expected.metadata());
    CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5);
}

TEST_CASE("ArealWeightedDistribution.arealWeightedDistributionWeiss")
{
    RasterMetadata meta(5, 5, 0.0, 0.0, 100.0, -9999);
    const DenseRaster<int> zones(meta, std::vector<int>{
                                           0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1,
                                           2, 2, 2, 2, 2,
                                           3, 3, 3, 3, 3,
                                           -9999, -9999, -9999, -9999, -9999});
    const std::unordered_map<int, double> amounts = {{0, 1}, {1, 0}, {2, 100.0}, {3, 1000.0}};
    auto actual                                   = gdx::areal_weighted_distribution<float>(zones, amounts);

    meta.nodata = 0.0;
    DenseRaster<float> expected(meta, std::vector<float>{
                                          0.20f, 0.20f, 0.20f, 0.20f, 0.20f,
                                          0, 0, 0, 0, 0,
                                          20.0f, 20.0f, 20.0f, 20.0f, 20.0f,
                                          200.0f, 200.0f, 200.0f, 200.0f, 200.0f,
                                          0, 0, 0, 0, 0});
    CHECK(actual.metadata() == expected.metadata());
    CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5);
}
}
