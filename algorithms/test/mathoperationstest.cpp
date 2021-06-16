#include "gdx/test/testbase.h"

#include "gdx/algo/mathoperations.h"

#include <numeric>
#include <random>

namespace gdx::test {

TEST_CASE_TEMPLATE("Math operations", TypeParam, RasterTypes)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    auto nan = std::numeric_limits<double>::quiet_NaN();

    SUBCASE("abs")
    {
        const std::vector<double> v = {
            nan, 1.0, -1.0,
            -4.0, nan, -2.0,
            0.0, 2.0, nan};

        const std::vector<double> exp = {
            nan, 1.0, 1.0,
            4.0, nan, 2.0,
            0.0, 2.0, nan};

        Raster raster(RasterMetadata(3, 3, nan), convertTo<T>(v));
        Raster expected(RasterMetadata(3, 3, nan), convertTo<T>(exp));
        CHECK_RASTER_EQ(expected, gdx::abs(raster));
    }

    SUBCASE("clip")
    {
        const std::vector<double> input = {
            nan, 0.0, -1.0,
            1.0, nan, 2.0,
            3.0, 4.0, nan};

        const std::vector<double> expectedData = {
            nan, 1.0, 1.0,
            1.0, nan, 2.0,
            3.0, 3.0, nan};

        Raster raster(RasterMetadata(3, 3, nan), convertTo<T>(input));
        Raster expected(RasterMetadata(3, 3, nan), convertTo<T>(expectedData));
        CHECK_RASTER_EQ(expected, gdx::clip(raster, T(1), T(3)));
    }
}
}
