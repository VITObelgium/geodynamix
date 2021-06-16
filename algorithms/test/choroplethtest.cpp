#include "gdx/test/testbase.h"

#include "gdx/algo/choropleth.h"

#include <numeric>
#include <random>

namespace gdx::test {

TEST_CASE_TEMPLATE("choropleth", TypeParam, RasterTypes)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;

    if (!typeSupported<T>()) return;

    RasterMetadata meta(3, 3);

    SUBCASE("sum")
    {
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                0.0, 1.0, 2.0,
                                3.0, 4.0, 5.0,
                                6.0, 7.0, 8.0}));

        Raster areas(meta, convertTo<T>(std::vector<double>{
                               1.0, 1.0, 2.0,
                               1.0, 1.0, 2.0,
                               3.0, 3.0, 3.0}));

        Raster expected(meta, convertTo<T>(std::vector<double>{
                                  8.0, 8.0, 7.0,
                                  8.0, 8.0, 7.0,
                                  21.0, 21.0, 21.0}));

        Raster result(meta);
        choropleth_sum(raster, areas, result);
        CHECK_RASTER_EQ(expected, result);
    }

    SUBCASE("avg")
    {
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                0.0, 1.0, 2.0,
                                3.0, 4.0, 5.0,
                                6.0, 7.0, 8.0}));

        Raster areas(meta, convertTo<T>(std::vector<double>{
                               1.0, 1.0, 2.0,
                               1.0, 1.0, 2.0,
                               3.0, 3.0, 3.0}));

        Raster expected(meta, convertTo<T>(std::vector<double>{
                                  2.0, 2.0, 3.5,
                                  2.0, 2.0, 3.5,
                                  7.0, 7.0, 7.0}));

        Raster result(meta);
        choropleth_average(raster, areas, result);
        CHECK_RASTER_EQ(expected, result);
    }
}
}