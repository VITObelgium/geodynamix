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

    SUBCASE("abs")
    {
        double nod = -9999.0;

        const std::vector<double> v = {
            nod, 1.0, -1.0,
            -4.0, nod, -2.0,
            0.0, 2.0, nod};

        const std::vector<double> exp = {
            nod, 1.0, 1.0,
            4.0, nod, 2.0,
            0.0, 2.0, nod};

        Raster raster(RasterMetadata(3, 3, nod), convertTo<T>(v));
        Raster expected(RasterMetadata(3, 3, nod), convertTo<T>(exp));
        CHECK_RASTER_EQ(expected, gdx::abs(raster));
    }

    SUBCASE("clip")
    {
        double nod = -9999.0;

        const std::vector<double> input = {
            nod, 0.0, -1.0,
            1.0, nod, 2.0,
            3.0, 4.0, nod};

        const std::vector<double> expectedData = {
            nod, 1.0, 1.0,
            1.0, nod, 2.0,
            3.0, 3.0, nod};

        Raster raster(RasterMetadata(3, 3, nod), convertTo<T>(input));
        Raster expected(RasterMetadata(3, 3, nod), convertTo<T>(expectedData));
        CHECK_RASTER_EQ(expected, gdx::clip(raster, T(1), T(3)));
    }
}
}
