#include "gdx/algo/dasmaplineairdistribution.h"
#include "gdx/test/testbase.h"

namespace gdx::test {

TEST_CASE_TEMPLATE("DasMapLineairDistribution", TypeParam, RasterTypes)
{
    SUBCASE("dasMapLineairDistribution")
    {
        RasterMetadata meta(5, 5, 0.0, 0.0, 100.0, -9999.0);
        const DenseRaster<int> landuse_map(meta, std::vector<int>{
                                                     0, 1, 2, 3, 4,
                                                     0, 1, 2, 3, 4,
                                                     0, 1, 2, 3, 4,
                                                     0, 1, 2, 3, 4,
                                                     0, 1, 2, 3, 4});
        std::unordered_map<int, double> weight_per_landuse = {{0, 1.0}, {1, 1.1}, {2, 1.2}, {3, 1.3}, {4, 1.4}};
        const DenseRaster<int> zone_map(meta, std::vector<int>{
                                                  0, 0, 0, 0, 0,
                                                  1, 1, 1, 1, 1,
                                                  2, 2, 2, 2, 2,
                                                  3, 3, 3, 3, 3,
                                                  4, 4, 4, 4, 4});
        const std::unordered_map<int, double> amounts = {{0, 0.0}, {1, 10.0}, {2, 20.0}, {3, 30.0}, {4, 40.0}};
        const DenseRaster<float> proxy(meta, std::vector<float>{
                                                 1 / 1.0f, 1 / 1.1f, 1 / 1.2f, 1 / 1.3f, 1 / 1.4f,
                                                 1 / 1.0f, 1 / 1.1f, 1 / 1.2f, 1 / 1.3f, 1 / 1.4f,
                                                 1 / 1.0f, 1 / 1.1f, 1 / 1.2f, 1 / 1.3f, 1 / 1.4f,
                                                 1 / 1.0f, 1 / 1.1f, 1 / 1.2f, 1 / 1.3f, 1 / 1.4f,
                                                 1 / 1.0f, 1 / 1.1f, 1 / 1.2f, 1 / 1.3f, 1 / 1.4f});
        auto actual = gdx::dasMapLineairDistribution<float>(landuse_map, weight_per_landuse, zone_map, amounts, proxy);

        float x[5];
        for (int i = 0; i < 5; ++i) {
            x[i] = 1 / 5.0f;
        }
        meta.nodata = 0.0;
        DenseRaster<float> expected(meta, std::vector<float>{
                                              0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                              10.0f * x[0], 10.0f * x[1], 10.0f * x[2], 10.0f * x[3], 10.0f * x[4],
                                              20.0f * x[0], 20.0f * x[1], 20.0f * x[2], 20.0f * x[3], 20.0f * x[4],
                                              30.0f * x[0], 30.0f * x[1], 30.0f * x[2], 30.0f * x[3], 30.0f * x[4],
                                              40.0f * x[0], 40.0f * x[1], 40.0f * x[2], 40.0f * x[3], 40.0f * x[4]});
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5);
    }

    SUBCASE("dasMapLineairDistributionWeiss")
    {
        RasterMetadata meta(5, 5, 0.0, 0.0, 100.0, -9999.0);
        auto nan = std::numeric_limits<float>::quiet_NaN();
        const DenseRaster<int> landuse_map(meta, std::vector<int>{
                                                     0, 1, 2, 3, -9999,
                                                     0, 1, 2, 3, -9999,
                                                     0, 1, 2, 3, -9999,
                                                     0, 1, 2, 3, -9999,
                                                     0, 1, 2, 3, -9999});
        std::unordered_map<int, double> weight_per_landuse = {{0, 1.0}, {1, 0.0}, {2, 1.2}, {3, 1.3}};
        const DenseRaster<int> zone_map(meta, std::vector<int>{
                                                  0, 0, 0, 0, 0,
                                                  1, 1, 1, 1, 1,
                                                  2, 2, 2, 2, 2,
                                                  3, 3, 3, 3, 3,
                                                  -9999, -9999, -9999, -9999, -9999});
        const std::unordered_map<int, double> amounts = {{0, 1.0}, {1, 0.0}, {2, 100.0}, {3, 1000.0}};
        const DenseRaster<float> proxy(meta, std::vector<float>{
                                                 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                                                 1.0f, 1.1f, 1.0f, 0.0f, 1.0f,
                                                 1.0f, 1.0f, 1.2f, 1.0f, 1.0f,
                                                 1.0f, 0.9f, 1.0f, nan, 1.0f,
                                                 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});
        auto actual = gdx::dasMapLineairDistribution<float>(landuse_map, weight_per_landuse, zone_map, amounts, proxy);

        meta.nodata = 0.0;
        DenseRaster<float> expected(meta, std::vector<float>{
                                              0.2857142984867096f, 0, 0.34285715222358704f, 0.37142854928970337f, 0,
                                              0, 0, 0, 0, 0,
                                              26.73796844482422f, 0, 38.50267791748047f, 34.75935745239258f, 0,
                                              454.5454406738281f, 0, 545.4545288085938f, 0, 0,
                                              0, 0, 0, 0, 0});
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5);
    }
}
}
