#include "gdx/algo/weighteddistribution.h"
#include "gdx/rastermetadata.h"
#include "gdx/test/testbase.h"

namespace gdx::test {

TEST_CASE_TEMPLATE("Weighted distribution", TypeParam, UnspecializedRasterTypes)
{
    using FloatRaster = typename TypeParam::template type<float>;
    using IntRaster   = typename TypeParam::template type<int32_t>;

    SUBCASE("nonUniformWeight")
    {
        RasterMetadata meta(1, 5, 0.0, 0.0, 100.0, -9999);
        const IntRaster zones(meta, std::vector<int>{1, 1, 1, 2, 2});
        const FloatRaster weights(meta, std::vector<float>{1, 2, 3, 4, 5});
        const std::unordered_map<int, double> amounts = {{1, 60}, {2, 90}};
        auto actual                                   = gdx::weighted_distribution<float>(zones, weights, amounts, true);

        meta.nodata = 0.0;
        FloatRaster expected(meta, std::vector<float>{10, 20, 30, 40, 50});
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
    }

    SUBCASE("uniformWeight")
    {
        RasterMetadata meta(4, 5, 0.0, 0.0, 100.0, -9999);
        const IntRaster zones(meta, std::vector<int>{1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4});
        const FloatRaster weights(meta, std::vector<float>{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1});
        const std::unordered_map<int, double> amounts = {{0, 0}, {1, 1}, {2, 1}, {3, 1}, {4, 1}};
        FloatRaster actual                            = gdx::weighted_distribution<float>(zones, weights, amounts, true);

        meta.nodata = 0.0;
        FloatRaster expected(meta, std::vector<float>{0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f});
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
    }

    SUBCASE("nanWeight")
    {
        RasterMetadata meta(1, 5, 0.0, 0.0, 100.0, -9999);
        const IntRaster zones(meta, std::vector<int>{1, 1, 1, 2, 2});
        const FloatRaster weights(meta, std::vector<float>{1, 2, -9999, -9999, -9999});
        const std::unordered_map<int, double> amounts = {{1, 60}, {2, 90}};
        auto actual                                   = gdx::weighted_distribution<float>(zones, weights, amounts, true);

        meta.nodata = 0.0;
        FloatRaster expected(meta, std::vector<float>{20, 40, 0, 45, 45});
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
    }

    SUBCASE("weightedDistributionWeiss")
    {
        RasterMetadata meta(5, 5, 0.0, 0.0, 100.0, -9999.0);
        const IntRaster zones(meta, std::vector<int>{
                                        0, 0, 0, 0, 0,
                                        1, 1, 1, 1, 1,
                                        2, 2, 2, 2, 2,
                                        3, 3, 3, 3, 3,
                                        -9999, -9999, -9999, -9999, -9999});
        const FloatRaster weights(meta, std::vector<float>{
                                            1, 1, 1, 1, 1,
                                            0, 0, 0, 0, 0,
                                            2, 0, 1.1f, 0.9f, -9999,
                                            0, 0, 0, 0, -9999,
                                            1, 1, 1, 1, -9999});
        CHECK(weights.is_nodata(2, 4));
        const std::unordered_map<int, double> amounts = {{0, 1}, {1, 0}, {2, 100}, {3, 1000}};
        FloatRaster actual                            = gdx::weighted_distribution<float> //, DenseRaster, int, float, double>
            (zones, weights, amounts, true);

        meta.nodata = 0.0;
        FloatRaster expected(meta, std::vector<float>{
                                       0.20f, 0.20f, 0.20f, 0.20f, 0.20f,
                                       0, 0, 0, 0, 0,
                                       50, 0, 27.5f, 22.5f, 0,
                                       250, 250, 250, 250, 0,
                                       0, 0, 0, 0, 0});
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
    }

    SUBCASE("weightedDistributionWeiss2")
    {
        RasterMetadata meta(5, 5, 0.0, 0.0, 100.0, -9999.0);
        const IntRaster zones(meta, std::vector<int>{
                                        0, 0, 0, 0, 0,
                                        1, 1, 1, 1, 1,
                                        2, 2, 2, 2, 2,
                                        3, 3, 3, 3, 3,
                                        -9999, -9999, -9999, -9999, -9999});
        const FloatRaster weights(meta, std::vector<float>{
                                            -9999, -9999, -9999, -9999, -9999,
                                            0, 0, 0, 0, 0,
                                            2, 0, 1.1f, 0.9f, -9999,
                                            0, 0, 0, 0, -9999,
                                            1, 1, 1, 1, -9999});
        CHECK(weights.is_nodata(2, 4));
        const std::unordered_map<int, double> amounts = {{0, 1}, {1, 0}, {2, 100}, {3, 1000}};
        FloatRaster actual                            = gdx::weighted_distribution<float> //, DenseRaster, int, float, double>
            (zones, weights, amounts, true);

        meta.nodata = 0.0;
        FloatRaster expected(meta, std::vector<float>{
                                       0.20f, 0.20f, 0.20f, 0.20f, 0.20f,
                                       0, 0, 0, 0, 0,
                                       50, 0, 27.5f, 22.5f, 0,
                                       250, 250, 250, 250, 0,
                                       0, 0, 0, 0, 0});
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
    }

    SUBCASE("invalidAmountsBadZone")
    {
        const IntRaster zones(1, 5, std::vector<int>{1, 1, 1, 2, 2});
        const FloatRaster weights(1, 5, std::vector<float>{1, 2, 3, 4, 5});
        const std::unordered_map<int, double> amounts = {{1, 60}, {2, 60}, {3, 90}};
        CHECK_THROWS_AS(gdx::weighted_distribution<float>(zones, weights, amounts, true), InvalidArgument);
    }

    SUBCASE("invalidWeightsNegative")
    {
        const IntRaster zones(1, 5, std::vector<int>{1, 1, 1, 2, 2});
        const FloatRaster weights(1, 5, std::vector<float>{1, 2, 3, -4, -5});
        const std::unordered_map<int, double> amounts = {{1, 60}, {2, 90}};
        CHECK_THROWS_AS(gdx::weighted_distribution<float>(zones, weights, amounts, true), InvalidArgument);
    }

    SUBCASE("invalidZonesNegative")
    {
        const IntRaster zones(1, 5, std::vector<int>{1, 1, 1, -2, -2});
        const FloatRaster weights(1, 5, std::vector<float>{1, 2, 3, 4, 5});
        const std::unordered_map<int, double> amounts = {{1, 60}, {2, 90}};
        CHECK_THROWS_AS(gdx::weighted_distribution<float>(zones, weights, amounts, true), InvalidArgument);
    }

    SUBCASE("incompatibleRasterSize")
    {
        const IntRaster zones(1, 4, std::vector<int>{1, 1, 1, 2});
        const FloatRaster weights(1, 5, std::vector<float>{1, 2, 3, 4, 5});
        const std::unordered_map<int, double> amounts = {{1, 60}, {2, 90}};
        CHECK_THROWS_AS(gdx::weighted_distribution<float>(zones, weights, amounts, true), InvalidArgument);
    }
}
}