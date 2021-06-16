#include "gdx/algo/suminbuffer.h"
#include "gdx/test/testbase.h"

namespace gdx::test {

TEST_CASE_TEMPLATE("Sum in buffer", TypeParam, RasterTypes)
{
    constexpr auto nan = std::numeric_limits<float>::quiet_NaN();

    RasterMetadata meta(5, 4, nan);
    meta.cellSize = 5;

    MaskedRaster<float> raster(meta, std::vector<float>{
                                         2.f, nan, 4.f, 4.f,
                                         4.f, 8.f, 4.f, 9.f,
                                         2.f, 4.f, nan, 7.f,
                                         4.f, 4.f, 4.f, 8.f,
                                         3.f, nan, 4.f, -5.f});

    SUBCASE("sumInBuffer")
    {
        MaskedRaster<float> expected(meta, std::vector<float>{
                                               6.f, 14.f, 12.f, 17.f,
                                               16.f, 20.f, 25.f, 24.f,
                                               14.f, 18.f, 19.f, 24.f,
                                               13.f, 16.f, 20.f, 14.f,
                                               7.f, 11.f, 3.f, 7.f});

        auto actual = sum_in_buffer(raster, 5.f, BufferStyle::Circular);

        CHECK_RASTER_EQ(expected, actual);
    }

    SUBCASE("sumInBufferSquare")
    {
        MaskedRaster<float> expected(meta, std::vector<float>{
                                               14.f, 22.f, 29.f, 21.f,
                                               20.f, 28.f, 40.f, 28.f,
                                               26.f, 34.f, 48.f, 32.f,
                                               17.f, 25.f, 26.f, 18.f,
                                               11.f, 19.f, 15.f, 11.f});

        auto actual = sum_in_buffer(raster, 10.f, BufferStyle::Square);

        CHECK_RASTER_EQ(expected, actual);
    }

    SUBCASE("maxInBuffer")
    {
        MaskedRaster<float> expected(meta, std::vector<float>{
                                               4.f, 8.f, 4.f, 9.f,
                                               8.f, 8.f, 9.f, 9.f,
                                               4.f, 8.f, 7.f, 9.f,
                                               4.f, 4.f, 8.f, 8.f,
                                               4.f, 4.f, 4.f, 8.f});

        auto actual = max_in_buffer(raster, 5.f);

        CHECK_RASTER_EQ(expected, actual);
    }
}
}
