#include "gdx/algo/distance.h"
#include "gdx/test/testbase.h"

namespace gdx::test {

TEST_CASE_TEMPLATE("distance", TypeParam, UnspecializedRasterTypes)
{
    using FloatRaster = typename TypeParam::template type<float>;
    using ByteRaster  = typename TypeParam::template type<uint8_t>;

    constexpr auto nan = std::numeric_limits<float>::quiet_NaN();
    RasterMetadata meta(5, 10, nan);
    meta.set_cell_size(100.0);

    SUBCASE("distance")
    {
        auto targetsMeta   = meta;
        targetsMeta.nodata = 255;
        ByteRaster targets(targetsMeta, std::vector<uint8_t>{
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            1, 2, 0, 0, 0, 0, 0, 0, 0, 0,
                                            3, 0, 0, 1, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0});

        FloatRaster expected(meta, std::vector<float>{
                                       200.0f, 200.0f, 241.421f, 282.843f, 341.421f, 382.843f, 424.264f, 524.264f, 624.264f, 724.264f,
                                       100.f, 100.f, 141.421f, 200.f, 241.421f, 282.843f, 382.843f, 482.843f, 582.843f, 682.843f,
                                       0.f, 0.f, 100.f, 100.f, 141.421f, 241.421f, 341.421f, 441.421f, 541.421f, 641.421f,
                                       0.f, 100.f, 100.f, 0.f, 100.f, 200.f, 300.f, 400.f, 500.f, 600.f,
                                       100.f, 141.421f, 141.421f, 100.f, 141.421f, 241.421f, 341.421f, 441.421f, 541.421f, 641.421f});

        auto actual = distance(targets);

        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-4);
    }

    SUBCASE("distance all ones")
    {
        auto targetsMeta   = meta;
        targetsMeta.nodata = 255;
        ByteRaster targets(targetsMeta, std::vector<uint8_t>{
                                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1});

        FloatRaster expected(meta, std::vector<float>{
                                       0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
                                       0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
                                       0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
                                       0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
                                       0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f});

        auto actual = distance(targets);

        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-4);
    }

    SUBCASE("with obstacles")
    {
        auto targetsMeta   = meta;
        targetsMeta.nodata = 255;
        ByteRaster targets(targetsMeta, std::vector<uint8_t>{
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            1, 2, 0, 0, 0, 0, 0, 0, 0, 0,
                                            3, 0, 0, 1, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0});

        ByteRaster barrier(targetsMeta, std::vector<uint8_t>{
                                            0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
                                            1, 1, 1, 0, 0, 0, 1, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
                                            0, 0, 0, 1, 0, 0, 1, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0});

        const float inf = std::numeric_limits<float>::infinity();

        FloatRaster expected(meta, std::vector<float>{
                                       541.421f, 441.421f, 341.421f, 300.f, 341.421f, 382.843f, inf, 782.843f, 824.264f, 865.685f,
                                       inf, inf, inf, 200.f, 241.421f, 282.843f, inf, 682.843f, 724.264f, 765.685f,
                                       0.f, 0.f, 100.f, 100.f, 141.421f, 241.421f, inf, 582.843f, 624.264f, 724.264f,
                                       0.f, 100.f, 100.f, 0.f, 100.f, 200.f, inf, 482.843f, 582.843f, 682.843f,
                                       100.f, 141.421f, 141.421f, 100.f, 141.421f, 241.421f, 341.421f, 441.421f, 541.421f, 641.421f});

        auto actual = distance(targets, barrier);

        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-4);
    }
}
}
