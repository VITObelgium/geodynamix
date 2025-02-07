#include "gdx/algo/aggregateandspreadmultiresolution.h"
#include "gdx/test/testbase.h"

#include <map>

namespace gdx::test {

TEST_CASE("AggegateAndSpreadMultiResolution.aggegateAndSpreadMultiResolution")
{
    RasterMetadata meta10(10, 10, 0.0, 0.0, 50.0, -9999.0);
    RasterMetadata meta5(5, 5, 0.0, 0.0, 100.0, -9999.0);
    const DenseRaster<int> landuse_map(meta10, std::vector<int>{
                                                   0, 1, 2, 3, 4, 0, 1, 2, 3, 4,
                                                   0, 1, 2, 3, 4, 0, 1, 2, 3, 4,
                                                   0, 1, 2, 3, 4, 0, 1, 2, 3, 4,
                                                   0, 1, 2, 3, 4, 0, 1, 2, 3, 4,
                                                   0, 1, 2, 3, 4, 0, 1, 2, 3, 4,
                                                   0, 1, 2, 3, 4, 0, 1, 2, 3, 4,
                                                   0, 1, 2, 3, 4, 0, 1, 2, 3, 4,
                                                   0, 1, 2, 3, 4, 0, 1, 2, 3, 4,
                                                   0, 1, 2, 3, 4, 0, 1, 2, 3, 4,
                                                   0, 1, 2, 3, 4, 0, 1, 2, 3, 4});
    const std::unordered_map<int, double> weight_per_landuse = {{0, 1.0}, {1, 1.1}, {2, 1.2}, {3, 1.3}, {4, 1.4}};
    const DenseRaster<int> zones(meta10, std::vector<int>{
                                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                             1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                             1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                             2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                             2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                             3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                             3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                             4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
                                             4, 4, 4, 4, 4, 4, 4, 4, 4, 4});
    const std::unordered_map<int, double> amount_per_zone = {{0, 0.0}, {1, 10.0}, {2, 20.0}, {3, 30.0}, {4, 40.0}};
    DenseRaster<float> actual                             = gdx::aggregate_and_spread_multi_resolution<float>(landuse_map, weight_per_landuse, zones, amount_per_zone, meta5);
    float x[5];
    x[0]      = static_cast<float>(2 * weight_per_landuse.at(0) + 2 * weight_per_landuse.at(1));
    x[1]      = static_cast<float>(2 * weight_per_landuse.at(2) + 2 * weight_per_landuse.at(3));
    x[2]      = static_cast<float>(2 * weight_per_landuse.at(4) + 2 * weight_per_landuse.at(0));
    x[3]      = static_cast<float>(2 * weight_per_landuse.at(1) + 2 * weight_per_landuse.at(2));
    x[4]      = static_cast<float>(2 * weight_per_landuse.at(3) + 2 * weight_per_landuse.at(4));
    float sum = 0;
    for (int i = 0; i < 5; ++i) {
        sum += x[i];
    }
    for (int i = 0; i < 5; ++i) {
        x[i] /= sum;
    }
    DenseRaster<float> expected(meta5, std::vector<float>{
                                           0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                           10.0f * x[0], 10.0f * x[1], 10.0f * x[2], 10.0f * x[3], 10.0f * x[4],
                                           20.0f * x[0], 20.0f * x[1], 20.0f * x[2], 20.0f * x[3], 20.0f * x[4],
                                           30.0f * x[0], 30.0f * x[1], 30.0f * x[2], 30.0f * x[3], 30.0f * x[4],
                                           40.0f * x[0], 40.0f * x[1], 40.0f * x[2], 40.0f * x[3], 40.0f * x[4]});
    expected.set_nodata(expected.NaN);
    CHECK(actual.metadata() == expected.metadata());
    CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5);
}

TEST_CASE("AggegateAndSpreadMultiResolution.aggegateAndSpreadMultiResolutionNodata")
{
    RasterMetadata meta10(10, 10, 0.0, 0.0, 50.0, -9999.0);
    RasterMetadata meta5(5, 5, 0.0, 0.0, 100.0, -9999.0);
    const DenseRaster<int> landuse_map(meta10, std::vector<int>{
                                                   0, 1, 2, 3, 4, 0, 1, 2, 3, 4,
                                                   0, 1, 2, 3, 4, 0, 1, 2, 3, 4,
                                                   0, 1, 2, 3, 4, 0, 1, 2, 3, 4,
                                                   0, 1, 2, 3, 4, 0, 1, 2, 3, 4,
                                                   0, 1, 2, 3, 4, 0, 1, 2, 3, 4,
                                                   0, 1, 2, 3, 4, 0, 1, 2, 3, 4,
                                                   0, 1, 2, 3, 4, 0, 1, 2, 3, 4,
                                                   0, 1, 2, 3, 4, 0, 1, 2, 3, 4,
                                                   0, 1, 2, 3, 4, 0, 1, 2, 3, 4,
                                                   0, 1, 2, 3, 4, 0, 1, 2, 3, 4});
    const std::unordered_map<int, double> weight_per_landuse = {{0, 1.0}, {1, 1.1}, {2, 1.2}, {3, 1.3}, {4, 1.4}};
    const DenseRaster<int> zones(meta10, std::vector<int>{
                                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                             1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                             1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                             2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                             2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                             3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                             3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                             4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
                                             4, 4, 4, 4, 4, 4, 4, 4, 4, 4});
    const std::unordered_map<int, double> amount_per_zone = {{0, 0.0}, {1, 10.0}, {2, 20.0}, {3, 30.0}, {4, 40.0}};
    DenseRaster<float> actual                             = gdx::aggregate_and_spread_multi_resolution<float>(landuse_map, weight_per_landuse, zones, amount_per_zone, meta5);
    float x[5];
    x[0]      = static_cast<float>(2 * weight_per_landuse.at(0) + 2 * weight_per_landuse.at(1));
    x[1]      = static_cast<float>(2 * weight_per_landuse.at(2) + 2 * weight_per_landuse.at(3));
    x[2]      = static_cast<float>(2 * weight_per_landuse.at(4) + 2 * weight_per_landuse.at(0));
    x[3]      = static_cast<float>(2 * weight_per_landuse.at(1) + 2 * weight_per_landuse.at(2));
    x[4]      = static_cast<float>(2 * weight_per_landuse.at(3) + 2 * weight_per_landuse.at(4));
    float sum = 0;
    for (int i = 0; i < 5; ++i) {
        sum += x[i];
    }
    for (int i = 0; i < 5; ++i) {
        x[i] /= sum;
    }
    DenseRaster<float> expected(meta5, std::vector<float>{
                                           0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                           10.0f * x[0], 10.0f * x[1], 10.0f * x[2], 10.0f * x[3], 10.0f * x[4],
                                           20.0f * x[0], 20.0f * x[1], 20.0f * x[2], 20.0f * x[3], 20.0f * x[4],
                                           30.0f * x[0], 30.0f * x[1], 30.0f * x[2], 30.0f * x[3], 30.0f * x[4],
                                           40.0f * x[0], 40.0f * x[1], 40.0f * x[2], 40.0f * x[3], 40.0f * x[4]});
    expected.set_nodata(expected.NaN);
    CHECK(actual.metadata() == expected.metadata());
    CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5);
}

TEST_CASE("AggegateAndSpreadMultiResolution.aggegateAndSpreadMultiResolutionWeiss")
{
    RasterMetadata meta10(10, 10, 0.0, 0.0, 50.0, -9999.0);
    RasterMetadata meta5(5, 5, 0.0, 0.0, 100.0, -9999.0);
    const DenseRaster<int> landuse_map(meta10, std::vector<int>{
                                                   0, 1, 2, 3, 4, -9999, 1, 2, -9999, -9999,
                                                   0, 1, 2, 3, 4, -9999, 1, 2, -9999, -9999,
                                                   0, 1, 2, 3, 4, -9999, 1, 2, -9999, -9999,
                                                   0, 1, 2, 3, 4, -9999, 1, 2, -9999, -9999,
                                                   0, 1, 2, 3, 4, -9999, 1, 2, -9999, -9999,
                                                   0, 1, 2, 3, 4, -9999, 1, 2, -9999, -9999,
                                                   0, 1, 2, 3, 4, -9999, 1, 2, -9999, -9999,
                                                   0, 1, 2, 3, 4, -9999, 1, 2, -9999, -9999,
                                                   0, 1, 2, 3, 4, -9999, 1, 2, -9999, -9999,
                                                   0, 1, 2, 3, 4, -9999, 1, 2, -9999, -9999});
    const std::unordered_map<int, double> weight_per_landuse = {{0, 1.0}, {1, 0.0}, {2, 1.2}, {3, 1.3}, {4, 0.0}};
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
    const std::unordered_map<int, double> amount_per_zone = {{0, 1.0}, {1, 0.0}, {2, 100.0}, {3, 1000.0}};
    DenseRaster<float> actual                             = gdx::aggregate_and_spread_multi_resolution<float>(landuse_map, weight_per_landuse, zones, amount_per_zone, meta5);

    auto nan = std::numeric_limits<float>::quiet_NaN();
    DenseRaster<float> expected(meta5, std::vector<float>{
                                           0.21276596188545227f, 0.5319148898124695f, 0.0f, 0.25531914830207825f, nan,
                                           0.0f, 0.0f, 0.0f, 0.0f, nan,
                                           21.276596069335938f, 53.191490173339844f, 0.0f, 25.53191566467285f, nan,
                                           212.76596069335938f, 531.9149169921875f, 0.0f, 255.31915283203125f, nan,
                                           nan, nan, nan, nan, nan});
    expected.set_nodata(expected.NaN);
    CHECK(actual.metadata() == expected.metadata());
    CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5);
}

}
