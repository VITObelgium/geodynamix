#include "gdx/test/testbase.h"

#include "gdx/algo/cast.h"

#include <numeric>
#include <random>

namespace gdx::test {

TEST_CASE_TEMPLATE("[algo] cast", TypeParam, UnspecializedRasterTypes)
{
    RasterMetadata meta(3, 3, -1.0);

    SUBCASE("float to integer")
    {
        using FloatRaster = typename TypeParam::template type<float>;
        using IntRaster   = typename TypeParam::template type<int32_t>;

        FloatRaster raster(meta, std::vector<float>{
                                     -1.f, 1.0f, 1.5f,
                                     -2.f, 2.f, 2.5f,
                                     -300.f, 300.f, -1.f});

        IntRaster expected(meta, std::vector<int32_t>{
                                     -1, 1, 1,
                                     -2, 2, 2,
                                     -300, 300, -1});

        CHECK_RASTER_EQ(expected, raster_cast<int32_t>(raster));
    }

    SUBCASE("signed to unsigned")
    {
        using SignedRaster   = typename TypeParam::template type<int32_t>;
        using UnsignedRaster = typename TypeParam::template type<uint8_t>;

        SignedRaster raster(meta, std::vector<int32_t>{
                                      -1, 1, 1,
                                      -2, 2, 2,
                                      -300, 300, -1});

        auto expectedMeta   = meta;
        expectedMeta.nodata = 255;
        UnsignedRaster expected(expectedMeta, std::vector<uint8_t>{
                                                  255, 1, 1,
                                                  254, 2, 2,
                                                  212, 44, 255});

        CHECK_RASTER_EQ(expected, raster_cast<uint8_t>(raster));
    }
}
}
