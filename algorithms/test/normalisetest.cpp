#include "gdx/test/testbase.h"

#include "gdx/algo/cast.h"
#include "gdx/algo/normalise.h"

#include <numeric>
#include <random>

namespace gdx::test {

TEST_CASE_TEMPLATE("Normalise", TypeParam, RasterTypes)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

    if (!typeSupported<T>()) return;

    RasterMetadata meta(3, 3);
    meta.nodata = 32.0;

    SUBCASE("normalise")
    {
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                0.0, 32.0, 32.0,
                                64.0, 64.0, 64.0,
                                96.0, 96.0, 128.0}));

        ResultRasterType expected(meta, std::vector<uint8_t>{
                                            0, 32, 32,
                                            127, 127, 127,
                                            191, 191, 254});

        ResultRasterType result(meta);
        normalise(raster, result, T(0), T(128), uint8_t(0), uint8_t(254));
        CHECK_RASTER_EQ(expected, result);
    }

    SUBCASE("normalise min max")
    {
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                0.0, 32.0, 32.0,
                                64.0, 64.0, 64.0,
                                96.0, 96.0, 128.0}));

        ResultRasterType expected(meta, std::vector<uint8_t>{
                                            0, 32, 32,
                                            127, 127, 127,
                                            191, 191, 254});

        ResultRasterType result = normalise_min_max<uint8_t>(raster, uint8_t(0), uint8_t(254));
        CHECK_RASTER_EQ(expected, result);
    }
}

TEST_CASE_TEMPLATE("Normalise float", TypeParam, RasterFloatTypes)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;

    if (!typeSupported<T>()) return;

    RasterMetadata meta(3, 3);
    meta.nodata = 4.0;

    SUBCASE("normalise max")
    {
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                0.0, 1.0, 2.0,
                                3.0, 4.0, 5.0,
                                6.0, 7.0, 10.0}));

        Raster expected(meta, convertTo<T>(std::vector<double>{
                                  0.0, 10.0, 20.0,
                                  30.0, 4.0, 50.0,
                                  60.0, 70.0, 100.0}));

        SUBCASE("normalise max")
        {
            Raster result = normalise_max(raster, T(0.0), T(100.0));
            CHECK_RASTER_EQ(expected, result);
        }

        SUBCASE("in place")
        {
            normalise_max_in_place(raster, T(0.0), T(100.0));
            CHECK_RASTER_EQ(expected, raster);
        }
    }
}
}
