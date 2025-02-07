#include "gdx/algo/weightedsum.h"
#include "gdx/test/testbase.h"

namespace gdx::test {

TEST_CASE_TEMPLATE("Weighted Sum", TypeParam, UnspecializedRasterTypes)
{
    using FloatRaster = typename TypeParam::template type<float>;

    SUBCASE("weightedSum")
    {
        RasterMetadata meta(1, 5, 0.0, 0.0, 100.0, -9999.0);
        FloatRaster raster1(meta, std::vector<float>{0.1f, 1.1f, 2.1f, 3.1f, 4.1f});
        FloatRaster raster2(meta, std::vector<float>{1.0f, 4.0f, 9.0f, 16.0f, 25.0f});
        const std::vector<std::pair<const FloatRaster*, double>> rasters = {{&raster1, 1.0}, {&raster2, 0.5}};
        auto actual                                                      = gdx::weighted_sum<float>(rasters);

        meta.nodata = std::numeric_limits<double>::quiet_NaN();
        FloatRaster expected(meta, std::vector<float>{(0.1f + 1.0f * 0.5f), (1.1f + 4.0f * 0.5f), (2.1f + 9.0f * 0.5f), (3.1f + 16.0f * 0.5f), (4.1f + 25.0f * 0.5f)});
        expected /= (1.0f + 0.5f);
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
    }

    SUBCASE("rasterWithNan")
    {
        RasterMetadata meta(1, 5, 0.0, 0.0, 100.0, -9999.0);
        auto nan = std::numeric_limits<float>::quiet_NaN();
        FloatRaster raster1(meta, std::vector<float>{0.1f, 1.1f, 2.1f, nan, 4.1f});
        FloatRaster raster2(meta, std::vector<float>{1.0f, 4.0f, 9.0f, 16.0f, 25.0f});
        const std::vector<std::pair<const FloatRaster*, double>> rasters = {{&raster1, 1.0}, {&raster2, 0.5}};
        auto actual                                                      = gdx::weighted_sum<float>(rasters);

        meta.nodata = std::numeric_limits<double>::quiet_NaN();
        FloatRaster expected(meta, std::vector<float>{
                                       (0.1f + 1.0f * 0.5f),
                                       (1.1f + 4.0f * 0.5f),
                                       (2.1f + 9.0f * 0.5f),
                                       (16.0f * 0.5f),
                                       (4.1f + 25.0f * 0.5f)});
        expected /= (1.0f + 0.5f);
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
    }

    SUBCASE("errorIncompatibleWithRasters")
    {
        const FloatRaster raster1(1, 5, std::vector<float>{0.1f, 1.1f, 2.1f, 3.1f, 4.1f});
        const FloatRaster raster2(1, 4, std::vector<float>{1.0f, 4.0f, 9.0f, 16.0f});
        const std::vector<std::pair<const FloatRaster*, double>> rasters = {{&raster1, 1.0}, {&raster2, 0.5}};
        CHECK_THROWS_AS(gdx::weighted_sum<float>(rasters), InvalidArgument);
    }

    SUBCASE("weightedSumWeiss")
    {
        RasterMetadata meta(5, 5, 0.0, 0.0, 100.0, -9999.0);
        FloatRaster raster1(meta, std::vector<float>{
                                      0, 0, 0, 0, 0,
                                      1, 1, 1, 1, 1,
                                      2, 2, 2, 2, 2,
                                      3, 3, 3, 3, 3,
                                      -9999, -9999, -9999, -9999, -9999});
        FloatRaster raster2(meta, std::vector<float>{
                                      0, 1, 2, 3, -9999,
                                      0, 1, 2, 3, -9999,
                                      0, 1, 2, 3, -9999,
                                      0, 1, 2, 3, -9999,
                                      0, 1, 2, 3, -9999});
        const std::vector<std::pair<const FloatRaster*, double>> rasters = {{&raster1, 1.2}, {&raster2, 1.3}};
        auto actual                                                      = gdx::weighted_sum<float>(rasters);

        meta.nodata = std::numeric_limits<double>::quiet_NaN();
        auto nan    = std::numeric_limits<float>::quiet_NaN();
        FloatRaster expected(meta, std::vector<float>{
                                       nan, 0.52f, 1.04f, 1.56f, nan,
                                       0.48f, 1.0f, 1.52f, 2.04f, 0.48f,
                                       0.96f, 1.48f, 2.0f, 2.52f, 0.96f,
                                       1.44f, 1.96f, 2.48f, 3.0f, 1.44f,
                                       nan, 0.52f, 1.04f, 1.56f, nan});
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
    }
}
}