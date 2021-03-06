#include "gdx/test/testbase.h"

#include "gdx/algo/majorityfilter.h"

#include <numeric>
#include <random>

namespace gdx::test {

TEST_CASE_TEMPLATE("Majority filter", TypeParam, RasterTypes)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    RasterMetadata meta(6, 6, -9999);
    meta.set_cell_size(100.0);

    SUBCASE("MajorityFilter")
    {
        const std::vector<double> inputData = {
            1, 1, 0, 2, 2, -3,
            1, 1, 5, 2, 2, -3,
            4, 4, 5, 3, 3, -3,
            4, 4, 5, 3, 3, -9999,
            0, 0, 6, 6, -9999, -9999,
            0, 0, 6, -9999, -9999, -9999};

        const std::vector<double> expectedData = {
            1, 1, 1, 2, 2, -3,
            1, 1, 5, 2, 2, -3,
            4, 4, 5, 3, 3, -3,
            4, 4, 5, 3, 3, 3,
            0, 0, 6, 6, 3, -3,
            0, 0, 6, 6, 6, -9999};

        Raster raster(meta, convertTo<T>(inputData));
        Raster expected(meta, convertTo<T>(expectedData));

        auto result = majority_filter(raster, 200.0);
        CHECK_RASTER_EQ(expected, result);
    }

    SUBCASE("MajorityFilterOnlyNodata")
    {
        Raster raster(meta, -9999);
        Raster expected(meta, -9999);

        auto result = majority_filter(raster, 200.0);
        CHECK_RASTER_EQ(expected, result);
    }
}

TEST_CASE("Majority filter float")
{
    const std::vector<float> inputData = {
        4.1f, 4.1f, 4, 4, -3, -3,
        4.1f, 4.1f, 7, 7, 7, 3,
        5, 5, 7, 7, 6, 7,
        5, 5, 5, 5, 5, 6,
        0, 0, 5, 5, -9999, -9999,
        0, 0, 0, 2, -9999, -9999};

    const std::vector<float> expectedData = {
        4.1f, 4.1f, 4.f, 7.f, -3.f, -3.f,
        4.1f, 4.1f, 7.f, 7.f, 7.f, -3.f,
        5.f, 5.f, 7.f, 7.f, 7.f, 7.f,
        5.f, 5.f, 5.f, 5.f, 5.f, 6.f,
        0.f, 0.f, 5.f, 5.f, 5.f, 5.f,
        0.f, 0.f, 0.f, 5.f, 2.f, -9999.f};

    RasterMetadata meta(6, 6, -9999);
    meta.set_cell_size(100.0);

    MaskedRaster<float> raster(meta, inputData);
    MaskedRaster<float> expected(meta, expectedData);

    auto result = majority_filter(raster, 142.0);
    CHECK_RASTER_EQ(expected, result);
}
}
