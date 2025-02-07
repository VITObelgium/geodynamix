#include "gdx/test/testbase.h"

#include "gdx/algo/cast.h"
#include "gdx/algo/distribute.h"

#include <numeric>
#include <random>

namespace gdx::test {

TEST_CASE_TEMPLATE("distribute", TypeParam, RasterTypes)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;

    if (!typeSupported<T>()) return;

    RasterMetadata meta(3, 3, 255.0);

    SUBCASE("vector")
    {
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                0.0, 32.0, 32.0,
                                64.0, 255.0, 64.0,
                                96.0, 96.0, 255.0}));

        Raster expectedHalf(meta, convertTo<T>(std::vector<double>{
                                      0.0, 16.0, 16.0,
                                      32, 255.0, 32.0,
                                      48.0, 48.0, 255.0}));

        Raster expectedQuarter(meta, convertTo<T>(std::vector<double>{
                                         0.0, 8.0, 8.0,
                                         16, 255.0, 16.0,
                                         24.0, 24.0, 255.0}));

        std::vector<float> proportions = {0.5, 0.5};
        auto result                    = gdx::distribute<T>(raster, proportions);

        REQUIRE(result.size() == 2);
        CHECK_RASTER_EQ(expectedHalf, result[0]);
        CHECK_RASTER_EQ(expectedHalf, result[1]);

        proportions = {0.5, 0.25, 0.25};
        result      = gdx::distribute<T>(raster, proportions);

        REQUIRE(result.size() == 3);
        CHECK_RASTER_EQ(expectedHalf, result[0]);
        CHECK_RASTER_EQ(expectedQuarter, result[1]);
        CHECK_RASTER_EQ(expectedQuarter, result[2]);
    }

    SUBCASE("empty vector")
    {
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                0.0, 32.0, 32.0,
                                64.0, 255.0, 64.0,
                                96.0, 96.0, 255.0}));

        std::vector<float> proportions;
        CHECK(gdx::distribute<T>(raster, proportions).empty());
    }

    SUBCASE("value distribution")
    {
        using TResult          = double;
        using ResultRasterType = decltype(raster_cast<TResult>(std::declval<Raster>()));

        double value         = 100.0;
        constexpr double sum = 384.0;
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                0.0, 32.0, 32.0,
                                64.0, 255.0, 64.0,
                                96.0, 96.0, 255.0}));

        ResultRasterType expected(meta, convertTo<TResult>(std::vector<double>{
                                            0.0, (32.0 * value / sum), (32.0 * value / sum),
                                            (64.0 * value / sum), 255.0, (64.0 * value / sum),
                                            (96.0 * value / sum), (96.0 * value / sum), 255.0}));

        CHECK_RASTER_EQ(expected, gdx::value_distribution<TResult>(value, raster));
    }

    SUBCASE("raster distribution")
    {
        using TFrac               = float;
        using TResult             = double;
        using ResultRasterType    = decltype(raster_cast<TResult>(std::declval<Raster>()));
        using FractionsRasterType = decltype(raster_cast<TFrac>(std::declval<Raster>()));

        Raster raster(meta, convertTo<T>(std::vector<double>{
                                0.0, 32.0, 32.0,
                                64.0, 255.0, 64.0,
                                96.0, 96.0, 255.0}));

        FractionsRasterType fractions(meta, convertTo<TFrac>(std::vector<double>{
                                                0.0, 0.5, 0.75,
                                                255.0, 255.0, 1.0,
                                                0.25, 0.0, 0.5}));

        ResultRasterType expectedFractions(meta, convertTo<TResult>(std::vector<double>{
                                                     0.0, 16.0, 24.0,
                                                     0.0, 255.0, 64.0,
                                                     24.0, 0.0, 255.0}));

        ResultRasterType expectedRemainders(meta, convertTo<TResult>(std::vector<double>{
                                                      0.0, 16.0, 8.0,
                                                      64.0, 255.0, 0.0,
                                                      72.0, 96.0, 255.0}));

        auto result = gdx::raster_distribution<TResult>(raster, fractions);
        CHECK_RASTER_EQ(expectedFractions, result.fraction);
        CHECK_RASTER_EQ(expectedRemainders, result.remainder);
    }

    SUBCASE("spatially")
    {
        using TMask          = int32_t;
        using MaskRasterType = decltype(raster_cast<TMask>(std::declval<Raster>()));

        Raster raster(meta, convertTo<T>(std::vector<double>{
                                0.0, 1.0, 1.0,
                                12.0, 255.0, 22.0,
                                5.0, 6.0, 255.0}));

        MaskRasterType mask(meta, convertTo<TMask>(std::vector<double>{
                                      1.0, 1.0, 1.0,
                                      1.0, 255.0, 2.0,
                                      2.0, 3.0, 255.0}));

        Raster expected(meta, convertTo<T>(std::vector<double>{
                                  14.0, 255.0, 255.0,
                                  255.0, 255.0, 6.0,
                                  27.0, 255.0, 255.0}));

        std::unordered_map<TMask, Cell> mapping = {
            {1, Cell(0, 0)},
            {2, Cell(2, 0)},
            {3, Cell(1, 2)},
        };

        CHECK_RASTER_EQ(expected, gdx::distribute(raster, mask, mapping));
    }
}
}