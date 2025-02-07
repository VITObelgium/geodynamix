#include "gdx/algo/aggregatemultiresolution.h"
#include "gdx/test/testbase.h"

namespace gdx::test {

TEST_CASE("AggregateMultiResolution.aggregateMultiResolution")
{
    RasterMetadata meta(1, 5, 0.0, 0.0, 100.0, -9999.0);
    RasterMetadata meta10(2, 10, 0.0, 0.0, 50.0, -9999);
    const DenseRaster<int> ancillary(meta10, std::vector<int>{
                                                 1, 0, 1, 2, 1, 0, 1, 0, 0, 2,
                                                 0, 0, 1, 0, 0, 2, 1, 1, 0, 1});
    const std::unordered_map<int, double> weights = {{0, 1.0}, {1, 0.0}, {2, 100.0}};
    auto actual                                   = gdx::aggregate_multi_resolution<float>(ancillary, weights, meta);

    DenseRaster<float> expected(meta, std::vector<float>{
                                          3.0f, 101.0f, 102.0f, 1.0f, 102.0f});
    auto nan = std::numeric_limits<float>::quiet_NaN();
    expected.set_nodata(nan);
    CHECK(actual.metadata() == expected.metadata());
    CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5);
}

TEST_CASE("AggregateMultiResolution.aggregateMultiResolutionWeiss")
{
    RasterMetadata meta(5, 5, 0.0, 0.0, 100.0, -9999.0);
    RasterMetadata meta10(10, 10, 0.0, 0.0, 50.0, -9999);
    const DenseRaster<int> ancillary(meta10, std::vector<int>{
                                                 0, 1, 2, 3, 1, -9999, 1, 2, -9999, -9999,
                                                 0, 1, 2, 3, 1, -9999, 1, 2, -9999, -9999,
                                                 0, 1, 2, 3, 1, -9999, 1, 2, -9999, -9999,
                                                 0, 1, 2, 3, 1, -9999, 1, 2, -9999, -9999,
                                                 0, 1, 2, 3, 1, -9999, 1, 2, -9999, -9999,
                                                 0, 1, 2, 3, 1, -9999, 1, 2, -9999, -9999,
                                                 0, 1, 2, 3, 1, -9999, 1, 2, -9999, -9999,
                                                 0, 1, 2, 3, 1, -9999, 1, 2, -9999, -9999,
                                                 0, 1, 2, 3, 1, -9999, 1, 2, -9999, -9999,
                                                 0, 1, 2, 3, 1, -9999, 1, 2, -9999, -9999});
    const std::unordered_map<int, double> weights = {{0, 1}, {1, 0}, {2, 1.2}, {3, 1.3}};
    auto actual                                   = gdx::aggregate_multi_resolution<float>(ancillary, weights, meta);

    auto nan = std::numeric_limits<float>::quiet_NaN();
    DenseRaster<float> expected(meta, std::vector<float>{
                                          2.0f, 5.0f, 0.0f, 2.4f, nan,
                                          2.0f, 5.0f, 0.0f, 2.4f, nan,
                                          2.0f, 5.0f, 0.0f, 2.4f, nan,
                                          2.0f, 5.0f, 0.0f, 2.4f, nan,
                                          2.0f, 5.0f, 0.0f, 2.4f, nan});
    expected.set_nodata(nan);
    CHECK(actual.metadata() == expected.metadata());
    CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5);
}

}
