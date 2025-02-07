#include "gdx/algo/nodata.h"
#include "gdx/algo/logical.h"
#include "gdx/test/testbase.h"

#include <numeric>
#include <random>

namespace gdx::test {

TEST_CASE_TEMPLATE("Nodata", TypeParam, RasterTypes)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    SUBCASE("is_nodata")
    {
        const std::vector<double> v = {
            -999.0, -999.0, 4.0, 4.0,
            4.0, 8.0, 4.0, 9.0,
            2.0, 4.0, -999.0, 7.0,
            4.0, 4.0, -5.0, 8.0,
            3.0, -999.0, 4.0, -999.0};

        Raster raster1(RasterMetadata(5, 4), convertTo<T>(v));
        Raster raster2(RasterMetadata(5, 4, -999), convertTo<T>(v));

        MaskedRaster<uint8_t> expected1(RasterMetadata(5, 4), std::vector<uint8_t>{
                                                                  0, 0, 0, 0,
                                                                  0, 0, 0, 0,
                                                                  0, 0, 0, 0,
                                                                  0, 0, 0, 0,
                                                                  0, 0, 0, 0});

        MaskedRaster<uint8_t> expected2(RasterMetadata(5, 4), std::vector<uint8_t>{
                                                                  1, 1, 0, 0,
                                                                  0, 0, 0, 0,
                                                                  0, 0, 1, 0,
                                                                  0, 0, 0, 0,
                                                                  0, 1, 0, 1});

        MaskedRaster<uint8_t> output(RasterMetadata(5, 4));
        is_nodata(raster1, output);
        CHECK_RASTER_EQ(expected1, output);

        is_nodata(raster2, output);
        CHECK_RASTER_EQ(expected2, output);

        is_data(raster1, output);
        CHECK_RASTER_EQ(expected1, !output);

        is_data(raster2, output);
        CHECK_RASTER_EQ(expected2, !output);
    }

    SUBCASE("is_nodataAllis_nodata")
    {
        RasterMetadata meta(5, 4);
        meta.nodata = -999;

        Raster raster(meta, std::vector<T>(meta.rows * meta.cols, static_cast<T>(*meta.nodata)));
        MaskedRaster<uint8_t> output(5, 4);

        is_nodata(raster, output);
        CHECK(all_of(output));
    }

    SUBCASE("replaceNodata")
    {
        const std::vector<double> v = {
            -999.0, -999.0, 4.0, 4.0,
            4.0, 8.0, 4.0, 9.0,
            2.0, 4.0, -999.0, 7.0,
            4.0, 4.0, -5.0, 8.0,
            3.0, -999.0, 4.0, -999.0};

        Raster raster(RasterMetadata(5, 4, -999), convertTo<T>(v));

        Raster expected(RasterMetadata(5, 4), convertTo<T>(std::vector<double>{
                                                  44.0, 44.0, 4.0, 4.0,
                                                  4.0, 8.0, 4.0, 9.0,
                                                  2.0, 4.0, 44.0, 7.0,
                                                  4.0, 4.0, -5.0, 8.0,
                                                  3.0, 44.0, 4.0, 44.0}));

        replace_nodata_in_place(raster, T(44));
        CHECK_RASTER_EQ(expected, raster);

        MaskedRaster<uint8_t> isDataOutput(5, 4);
        is_data(raster, isDataOutput);
        CHECK(all_of(isDataOutput));
    }

    SUBCASE("turnValueIntoNodata")
    {
        const std::vector<double> v = {
            -999.0, -999.0, 4.0, 4.0,
            4.0, 8.0, 4.0, 9.0,
            2.0, 4.0, -999.0, 7.0,
            4.0, 4.0, -5.0, 8.0,
            3.0, -999.0, 4.0, -999.0};

        Raster raster(RasterMetadata(5, 4, -999), convertTo<T>(v));

        Raster expected(RasterMetadata(5, 4, -999), convertTo<T>(std::vector<double>{
                                                        -999.0, -999.0, -999.0, -999.0,
                                                        -999.0, 8.0, -999.0, 9.0,
                                                        2.0, -999.0, -999.0, 7.0,
                                                        -999.0, -999.0, -5.0, 8.0,
                                                        3.0, -999.0, -999.0, -999.0}));

        turn_value_into_nodata(raster, T(4.0));
        CHECK_RASTER_EQ(expected, raster);
    }
}
}