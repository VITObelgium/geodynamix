#include "gdx/algo/distance.h"
#include "gdx/test/testbase.h"

namespace gdx::test {

TEST_CASE_TEMPLATE("SumWithinTravelDistance", TypeParam, UnspecializedRasterTypes)
{
    using FloatRaster = typename TypeParam::template type<float>;
    using ByteRaster  = typename TypeParam::template type<uint8_t>;

    constexpr auto nan = std::numeric_limits<float>::quiet_NaN();
    RasterMetadata meta(5, 4, nan);
    meta.cellSize = 100;

    SUBCASE("sumWithinTravelDistance")
    {
        ByteRaster mask(meta, std::vector<uint8_t>{
                                  1, 1, 1, 1,
                                  1, 1, 1, 1,
                                  1, 1, 1, 1,
                                  1, 1, 1, 1,
                                  1, 1, 1, 1});

        FloatRaster resistance(meta, std::vector<float>{
                                         1.f, 1.f, 1.f, 1.f,
                                         1.f, 1.f, 9.f, 1.f,
                                         0.5f, 0.5f, 0.5f, 0.5f,
                                         1.f, 1.f, 1.f, 1.f,
                                         1.f, 1.f, 1.f, 1.f});

        FloatRaster value(meta, std::vector<float>{
                                    1.f, 10.f, 1.f, 1.f,
                                    1.f, 10.f, 1.f, 1.f,
                                    1.f, 10.f, 1.f, 1.f,
                                    1.f, 10.f, 0.f, 1.f,
                                    1.f, 10.f, 1.f, 1.f});

        FloatRaster expected(meta, std::vector<float>{
                                       12.f, 22.f, 12.f, 3.f,
                                       13.f, 31.f, 1.f, 3.f,
                                       14.f, 33.f, 13.f, 14.f,
                                       13.f, 31.f, 13.f, 3.f,
                                       12.f, 22.f, 12.f, 3.f});

        auto actual = sum_within_travel_distance<float>(mask, resistance, value, 1.01f, false);

        CHECK_RASTER_EQ(expected, actual);
    }

    SUBCASE("sumWithinTravelDistanceNan")
    {
        ByteRaster mask(meta, std::vector<uint8_t>{
                                  1, 1, 1, 1,
                                  1, 1, 1, 1,
                                  1, 1, 1, 1,
                                  1, 1, 1, 1,
                                  1, 1, 1, 1});

        FloatRaster resistance(meta, std::vector<float>{
                                         1.f, 1.f, 1.f, 1.f,
                                         1.f, 1.f, nan, 1.f,
                                         0.5f, 0.5f, 0.5f, 0.5f,
                                         1.f, 1.f, 1.f, 1.f,
                                         1.f, 1.f, 1.f, 1.f});

        FloatRaster value(meta, std::vector<float>{
                                    1.f, 10.f, 1.f, 1.f,
                                    1.f, 10.f, 1.f, 1.f,
                                    1.f, 10.f, 1.f, 1.f,
                                    1.f, 10.f, nan, 1.f,
                                    1.f, 10.f, 1.f, 1.f});

        FloatRaster expected(meta, std::vector<float>{
                                       12.f, 22.f, 12.f, 3.f,
                                       13.f, 31.f, 1.f, 3.f,
                                       14.f, 33.f, 13.f, 14.f,
                                       13.f, 31.f, 13.f, 3.f,
                                       12.f, 22.f, 12.f, 3.f});

        auto actual = sum_within_travel_distance<float>(mask, resistance, value, 1.01f, false);

        CHECK_RASTER_EQ(expected, actual);
    }

    SUBCASE("sumWithinTravelDistanceWithAdjacent")
    {
        ByteRaster mask(meta, std::vector<uint8_t>{
                                  1, 1, 1, 1,
                                  1, 1, 1, 1,
                                  1, 1, 1, 1,
                                  1, 1, 1, 1,
                                  1, 1, 1, 1});

        FloatRaster resistance(meta, std::vector<float>{
                                         1.f, 1.f, 1.f, 1.f,
                                         1.f, 1.f, 9.f, 1.f,
                                         0.5f, 0.5f, 0.5f, 0.5f,
                                         1.f, 1.f, 1.f, 1.f,
                                         1.f, 1.f, 1.f, 1.f});

        FloatRaster value(meta, std::vector<float>{
                                    1.f, 10.f, 1.f, 1.f,
                                    1.f, 10.f, 1.f, 1.f,
                                    1.f, 10.f, 1.f, 1.f,
                                    1.f, 10.f, 0.f, 1.f,
                                    1.f, 10.f, 1.f, 1.f});

        FloatRaster expected(meta, std::vector<float>{
                                       24.f, 35.f, 25.f, 15.f,
                                       35.f, 46.f, 14.f, 7.f,
                                       38.f, 58.f, 39.f, 38.f,
                                       34.f, 46.f, 37.f, 16.f,
                                       24.f, 34.f, 25.f, 14.f});

        auto actual = sum_within_travel_distance<float>(mask, resistance, value, 1.01f, true);

        CHECK_RASTER_EQ(expected, actual);
    }

    SUBCASE("sumWithinTravelDistanceWithAdjacentNan")
    {
        ByteRaster mask(meta, std::vector<uint8_t>{
                                  1, 1, 1, 1,
                                  1, 1, 1, 1,
                                  1, 1, 1, 1,
                                  1, 1, 1, 1,
                                  1, 1, 1, 1});

        FloatRaster resistance(meta, std::vector<float>{
                                         1.f, 1.f, 1.f, 1.f,
                                         1.f, 1.f, nan, 1.f,
                                         0.5f, 0.5f, 0.5f, 0.5f,
                                         1.f, 1.f, 1.f, 1.f,
                                         1.f, 1.f, 1.f, 1.f});

        FloatRaster value(meta, std::vector<float>{
                                    1.f, 10.f, 1.f, 1.f,
                                    1.f, 10.f, 1.f, 1.f,
                                    1.f, 10.f, 1.f, 1.f,
                                    1.f, 10.f, nan, 1.f,
                                    1.f, 10.f, 1.f, 1.f});

        FloatRaster expected(meta, std::vector<float>{
                                       24.f, 35.f, 25.f, 15.f,
                                       35.f, 46.f, 14.f, 7.f,
                                       38.f, 58.f, 39.f, 38.f,
                                       34.f, 46.f, 37.f, 16.f,
                                       24.f, 34.f, 25.f, 14.f});

        auto actual = sum_within_travel_distance<float>(mask, resistance, value, 1.01f, true);

        CHECK_RASTER_EQ(expected, actual);
    }

    SUBCASE("sumWithinTravelDistancePytest0")
    {
        meta          = RasterMetadata(5, 10, nan);
        meta.cellSize = 100;

        ByteRaster mask(meta, std::vector<uint8_t>{
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
                                  1, 0, 0, 1, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0});

        FloatRaster resistance(meta, std::vector<float>{
                                         9, 9, 9, 9, 9, 9, 9, 9, 1, 1,
                                         9, 9, 9, 9, 9, 9, 9, 1, 9, 1,
                                         1, 1, 1, 1, 1, 1, 1, 9, 9, 1,
                                         9, 9, 9, 9, 9, 9, 9, 9, 9, 1,
                                         9, 9, 9, 9, 9, 9, 9, 9, 9, 1});

        FloatRaster value(meta, std::vector<float>{
                                    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                                    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
                                    30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
                                    40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
                                    50, 51, 52, 53, 54, 55, 56, 57, 58, 59});

        FloatRaster expected(meta, std::vector<float>{
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                       685, 823, 0, 0, 0, 0, 0, 0, 0, 0,
                                       346, 0, 0, 463, 0, 0, 0, 0, 0, 0,
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0});

        auto actual = sum_within_travel_distance<float>(mask, resistance, value, 10.0001f, false);

        CHECK_RASTER_EQ(expected, actual);
    }
}
}
