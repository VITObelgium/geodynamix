#include "gdx/test/testbase.h"

#include "gdx/algo/conditionals.h"

#include <numeric>
#include <random>

namespace gdx::test {

TEST_CASE_TEMPLATE("conditionals", TypeParam, RasterTypes)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    SUBCASE("if then else")
    {
        static_assert(std::is_same_v<MaskedRaster<uint8_t>,
                          decltype(if_then_else(std::declval<MaskedRaster<int32_t>>(), std::declval<MaskedRaster<uint8_t>>(), std::declval<MaskedRaster<uint8_t>>()))>,
            "Unexpected result type");

        static_assert(std::is_same_v<MaskedRaster<uint32_t>,
                          decltype(if_then_else(std::declval<MaskedRaster<int32_t>>(), std::declval<MaskedRaster<uint8_t>>(), std::declval<MaskedRaster<uint32_t>>()))>,
            "Unexpected result type");

        const std::vector<double> ifRasterData = {
            1, 0, 1,
            0, 1, 0,
            1, 0, 1};

        const std::vector<double> thenRasterData = {
            2, 0, 2,
            0, 2, 0,
            2, 0, 2};

        const std::vector<double> elseRasterData = {
            0, 2, 0,
            2, 0, 2,
            0, 2, 0};

        const std::vector<double> expectedData = {
            2, 2, 2,
            2, 2, 2,
            2, 2, 2};

        RasterMetadata meta(3, 3);

        Raster ifRaster(meta, convertTo<T>(ifRasterData));
        Raster thenRaster(meta, convertTo<T>(thenRasterData));
        Raster elseRaster(meta, convertTo<T>(elseRasterData));
        Raster expectedRaster(meta, convertTo<T>(expectedData));

        auto result = if_then_else(ifRaster, thenRaster, elseRaster);

        CHECK_RASTER_EQ(expectedRaster, result);
    }

    SUBCASE("if then else nodata")
    {
        const std::vector<double> ifRasterData = {
            -999, 0, 1,
            0, -999, 0,
            1, 0, -999};

        const std::vector<double> thenRasterData = {
            2, 0, 2,
            0, 2, 0,
            2, 0, 2};

        const std::vector<double> elseRasterData = {
            0, 2, 0,
            2, 0, 2,
            0, 2, 0};

        double nodata = -999;
        if (std::numeric_limits<T>::has_quiet_NaN) {
            nodata = std::numeric_limits<double>::quiet_NaN();
        }

        const std::vector<double> expectedData = {
            nodata, 2, 2,
            2, nodata, 2,
            2, 2, nodata};

        RasterMetadata meta(3, 3);
        meta.nodata = -999;

        Raster ifRaster(meta, convertTo<T>(ifRasterData));
        Raster thenRaster(meta, convertTo<T>(thenRasterData));
        Raster elseRaster(meta, convertTo<T>(elseRasterData));
        Raster expectedRaster(meta, convertTo<T>(expectedData));

        auto result = if_then_else(ifRaster, thenRaster, elseRaster);

        CHECK_RASTER_NEAR(expectedRaster, result);
    }
}
}