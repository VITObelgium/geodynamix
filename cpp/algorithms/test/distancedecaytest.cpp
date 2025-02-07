#include "gdx/algo/distancedecay.h"
#include "gdx/algo/distance.h"
#include "gdx/test/testbase.h"

namespace gdx::test {

TEST_CASE_TEMPLATE("distancedecay", TypeParam, UnspecializedRasterTypes)
{
    using FloatRaster = typename TypeParam::template type<float>;
    using ByteRaster  = typename TypeParam::template type<uint8_t>;

    constexpr auto nan = std::numeric_limits<float>::quiet_NaN();
    RasterMetadata meta(5, 10, nan);
    meta.set_cell_size(100.0);

    auto targetsMeta   = meta;
    targetsMeta.nodata = 255;
    ByteRaster targets(targetsMeta, std::vector<uint8_t>{
                                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                        1, 2, 0, 0, 0, 0, 0, 0, 0, 0,
                                        3, 0, 0, 1, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0});

    FloatRaster travelTimes(targetsMeta, std::vector<float>{
                                             200.f, 141.421f, 100.f, 100.f, 141.421f, 241.421f, 341.421f, 400.f, 441.421f, 482.843f,
                                             100.f, 100.f, 0.f, 0.f, 100.f, 200.f, 300.f, 300.f, 341.421f, 382.843f,
                                             0.f, 100.f, 100.f, 100.f, 141.421f, 241.421f, 200.f, 200.f, 241.421f, 282.843f,
                                             100.f, 141.421f, 100.f, 0.f, 100.f, 141.421f, 100.f, 100.f, 141.421f, 241.421f,
                                             200.f, 241.421f, 141.421f, 100.f, 141.421f, 100.f, 0.f, 0.f, 100.f, 200.f});

    SUBCASE("node value distance decay dist 200")
    {
        FloatRaster expected(meta, std::vector<float>{
                                       0.f, 3.62842e-37f, 1.26756e-17f, 1.26756e-17f, 3.62842e-37f, 0.f, 0.f, 0.f, 0.f, 0.f,
                                       1.90134e-17f, 1.26756e-17f, 1.99101f, 1.99101f, 1.26756e-17f, 0.f, 0.f, 0.f, 0.f, 0.f,
                                       2.98651f, 1.99101f, 1.26756e-17f, 1.26756e-17f, 3.62842e-37f, 0.f, 0.f, 0.f, 0.f, 0.f,
                                       2.98651f, 1.5275e-25f, 6.33781e-18f, 0.995504f, 6.33781e-18f, 0.f, 0.f, 0.f, 0.f, 0.f,
                                       5.44263e-37f, 0.f, 1.81421e-37f, 6.33781e-18f, 1.81421e-37f, 0.f, 0.f, 0.f, 0.f, 0.f});

        auto actual = node_value_distance_decay(targets, travelTimes, 200.f, 5.4f, 0.45f);

        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-4);
    }
}
}
