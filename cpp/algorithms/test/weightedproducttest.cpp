#include "gdx/algo/weightedproduct.h"
#include "gdx/test/testbase.h"

namespace gdx::test {

TEST_CASE_TEMPLATE("Weighted product", TypeParam, UnspecializedRasterTypes)
{
    using FloatRaster = typename TypeParam::template type<float>;
    auto nan          = std::numeric_limits<double>::quiet_NaN();

    SUBCASE("weightedProduct")
    {
        RasterMetadata meta(1, 5, 0.0, 0.0, 100.0, -9999.0);
        const FloatRaster raster1(meta, std::vector<float>{0.1f, 1.1f, 2.1f, 3.1f, 4.1f});
        const FloatRaster raster2(meta, std::vector<float>{1.0f, 4.0f, 9.0f, 16.0f, 25.0f});
        const std::vector<std::pair<const FloatRaster*, double>> rasters = {{&raster1, 1.0}, {&raster2, 0.5}};
        auto actual                                                      = gdx::weighted_product<float>(rasters);

        meta.nodata = nan;
        FloatRaster expected(meta, std::vector<float>{0.1f * 1.0f, 1.1f * 2.0f, 2.1f * 3.0f, 3.1f * 4.0f, 4.1f * 5.0f});
        expected /= 4.1f * 5.0f;
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
    }

    SUBCASE("rasterWithNan")
    {
        RasterMetadata meta(1, 5, 0.0, 0.0, 100.0, -9999.0);
        auto nan = std::numeric_limits<float>::quiet_NaN();
        const FloatRaster raster1(meta, std::vector<float>{0.1f, 1.1f, 2.1f, nan, 4.1f});
        const FloatRaster raster2(meta, std::vector<float>{1.0f, 4.0f, 9.0f, 16.0f, 25.0f});
        const std::vector<std::pair<const FloatRaster*, double>> rasters = {{&raster1, 1.0}, {&raster2, 0.5}};
        auto actual                                                      = gdx::weighted_product<float>(rasters);

        meta.nodata = nan;
        FloatRaster expected(meta, std::vector<float>{0.1f * 1.0f, 1.1f * 2.0f, 2.1f * 3.0f, nan, 4.1f * 5.0f});
        expected /= 4.1f * 5.0f;
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
    }

    SUBCASE("errorIncompatibleWithRasters")
    {
        const FloatRaster raster1(1, 5, std::vector<float>{0.1f, 1.1f, 2.1f, 3.1f, 4.1f});
        const FloatRaster raster2(1, 4, std::vector<float>{1.0f, 4.0f, 9.0f, 16.0f});
        const std::vector<std::pair<const FloatRaster*, double>> rasters = {{&raster1, 1.0}, {&raster2, 0.5}};
        CHECK_THROWS_AS(gdx::weighted_product<float>(rasters), InvalidArgument);
    }

    SUBCASE("errorSqrtOfNegativeNumber")
    {
        RasterMetadata meta(1, 5, 0.0, 0.0, 100.0, -9999.0);
        auto nan = std::numeric_limits<float>::quiet_NaN();
        FloatRaster raster1(meta, std::vector<float>{0.1f, 1.1f, 2.1f, 3.1f, 4.1f});
        raster1.set_nodata(nan);
        FloatRaster raster2(meta, std::vector<float>{1.0f, -4.0f, 9.0f, 16.0f, 25.0f});
        raster2.set_nodata(nan);
        const std::vector<std::pair<const FloatRaster*, double>> rasters = {{&raster1, 1.0}, {&raster2, 0.5}};
        auto actual                                                      = gdx::weighted_product<float>(rasters);

        meta.nodata = nan;
        FloatRaster expected(meta, std::vector<float>{0.1f * 1.0f, nan, 2.1f * 3.0f, 3.1f * 4.0f, 4.1f * 5.0f});
        expected.set_nodata(nan);
        expected /= 4.1f * 5.0f;
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
    }

    SUBCASE("weightedProductWeiss")
    {
        RasterMetadata meta(5, 5, 0.0, 0.0, 100.0, -9999.0);
        auto nan = std::numeric_limits<float>::quiet_NaN();
        FloatRaster raster1(meta, std::vector<float>{
                                      0, 0, 0, 0, 0,
                                      1, 1, 1, 1, 1,
                                      2, 2, 2, 2, 2,
                                      3, 3, 3, 3, 3,
                                      nan, nan, nan, nan, nan});
        raster1.set_nodata(nan);
        FloatRaster raster2(meta, std::vector<float>{
                                      0, 1, 2, 3, nan,
                                      0, 1, 2, 3, nan,
                                      0, 1, 2, 3, nan,
                                      0, 1, 2, 3, nan,
                                      0, 1, 2, 3, nan});
        raster2.set_nodata(nan);
        const std::vector<std::pair<const FloatRaster*, double>> rasters = {{&raster1, 1.1}, {&raster2, 1.0}};
        auto actual                                                      = gdx::weighted_product<float>(rasters);

        meta.nodata = nan;
        FloatRaster expected(meta, std::vector<float>{
                                       0.0f, 0.0f, 0.0f, 0.0f, nan,
                                       0.0f, 0.09955094009637833f, 0.19910188019275665f, 0.2986528277397156f, nan,
                                       0.0f, 0.21339212357997894f, 0.4267842471599579f, 0.6401763558387756f, nan,
                                       0.0f, 0.3333333432674408f, 0.6666666865348816f, 1.0f, nan,
                                       nan, nan, nan, nan, nan});
        expected.set_nodata(nan);
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
    }
}
}