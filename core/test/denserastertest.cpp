#include "gdx/denseraster.h"
#include "gdx/test/testbase.h"
#include "infra/cast.h"

#include <set>

namespace gdx::test {

TEST_CASE_TEMPLATE("dense raster", T, uint8_t, int16_t, int32_t, int64_t, float, double)
{
    using RasterType = DenseRaster<T>;

    double nod = 99.0;

    SUBCASE("init nodata")
    {
        RasterMetadata meta(3, 3, nod);
        RasterType raster(meta, convertTo<T>(std::vector<double>{
                                    99.0, 2.0, 3.0,
                                    99.0, 99.0, 99.0,
                                    5.0, 6.0, 99.0}));

        CHECK(raster.is_nodata(0));
        CHECK_FALSE(raster.is_nodata(1));
        CHECK_FALSE(raster.is_nodata(2));
        CHECK(raster.is_nodata(3));
        CHECK(raster.is_nodata(4));
        CHECK(raster.is_nodata(5));
        CHECK_FALSE(raster.is_nodata(6));
        CHECK_FALSE(raster.is_nodata(7));
        CHECK(raster.is_nodata(8));

        if constexpr (RasterType::raster_type_has_nan) {
            CHECK(std::isnan(raster[0]));
            CHECK_FALSE(std::isnan(raster[1]));
            CHECK_FALSE(std::isnan(raster[2]));
            CHECK(std::isnan(raster[3]));
            CHECK(std::isnan(raster[4]));
            CHECK(std::isnan(raster[5]));
            CHECK_FALSE(std::isnan(raster[6]));
            CHECK_FALSE(std::isnan(raster[7]));
            CHECK(std::isnan(raster[8]));
        }
    }

    SUBCASE("collapse data")
    {
        RasterMetadata meta(5, 5, nod);
        std::vector<double> data = {
            99.0, 2.0, 3.0, 4.0, 5.0,
            99.0, 99.0, 99.0, 99.0, 99.0,
            6.0, 7.0, 8.0, 9.0, 10.0,
            11.0, 12.0, 13.0, 14.0, 15.0,
            99.0, 99.0, 5.0, 6.0, 99.0};

        RasterType raster(meta, convertTo<T>(data));

        const std::set<size_t> nodataIndexes = {0, 5, 6, 7, 8, 9, 20, 21, 24};

        for (size_t i = 0; i < raster.size(); ++i) {
            if (nodataIndexes.count(i) > 0) {
                CHECK(raster.is_nodata(i));
                if constexpr (RasterType::raster_type_has_nan) {
                    CHECK(std::isnan(raster[i]));
                }
            } else {
                CHECK_FALSE(raster.is_nodata(i));
                if constexpr (RasterType::raster_type_has_nan) {
                    CHECK_FALSE(std::isnan(raster[i]));
                }
            }
        }

        raster.collapse_data();

        for (size_t i = 0; i < raster.size(); ++i) {
            if (nodataIndexes.count(i) > 0) {
                CHECK(raster[i] == inf::truncate<T>(*meta.nodata));
            } else {
                CHECK_FALSE(raster.is_nodata(i));
            }

            if constexpr (RasterType::raster_type_has_nan) {
                CHECK_FALSE(std::isnan(raster[i]));
            }
        }
    }

    SUBCASE("add or assign")
    {
        RasterMetadata meta(5, 5, nod);
        RasterType raster(meta, convertTo<T>(std::vector<double>{
                                    nod, 2.0, 3.0, 4.0, 5.0,
                                    nod, nod, nod, nod, nod,
                                    6.0, 7.0, 8.0, 9.0, 10.0,
                                    11.0, 12.0, 13.0, 14.0, 15.0,
                                    nod, nod, 5.0, 6.0, nod}));

        RasterType rasterToAdd(meta, convertTo<T>(std::vector<double>{
                                         1.0, 2.0, 3.0, nod, nod,
                                         3.0, nod, 3.0, nod, 3.0,
                                         1.0, 1.0, 1.0, 0.0, 0.0,
                                         0.0, 0.0, 0.0, 0.0, 0.0,
                                         nod, nod, 1.0, 1.0, nod}));

        RasterType expected(meta, convertTo<T>(std::vector<double>{
                                      1.0, 4.0, 6.0, 4.0, 5.0,
                                      3.0, nod, 3.0, nod, 3.0,
                                      7.0, 8.0, 9.0, 9.0, 10.0,
                                      11.0, 12.0, 13.0, 14.0, 15.0,
                                      nod, nod, 6.0, 7.0, nod}));

        CHECK_RASTER_EQ(expected, raster.add_or_assign(rasterToAdd));
    }

    SUBCASE("sub raster, fully contained")
    {
        RasterMetadata meta(5, 5, 10, 10, 5, nod);
        RasterType raster(meta, convertTo<T>(std::vector<double>{
                                    nod, 2.0, 3.0, 4.0, 5.0,
                                    nod, nod, nod, nod, nod,
                                    6.0, 7.0, 8.0, 9.0, 10.0,
                                    11.0, 12.0, 13.0, 14.0, 15.0,
                                    nod, nod, 5.0, 6.0, nod}));

        RasterType expected(RasterMetadata(3, 3, 15, 15, 5, nod), convertTo<T>(std::vector<double>{
                                                                      nod, nod, nod,
                                                                      7.0, 8.0, 9.0,
                                                                      12.0, 13.0, 14.0}));

        CHECK_RASTER_EQ(expected, sub_raster(raster, expected.metadata()));
    }

    SUBCASE("sub raster, over the edge")
    {
        RasterMetadata meta(5, 5, 10, 10, 5, nod);
        RasterType raster(meta, convertTo<T>(std::vector<double>{
                                    nod, 2.0, 3.0, 4.0, 5.0,
                                    nod, nod, nod, nod, nod,
                                    6.0, 7.0, 8.0, 9.0, 10.0,
                                    11.0, 12.0, 13.0, 14.0, 15.0,
                                    nod, nod, 5.0, 6.0, nod}));

        RasterType expected(RasterMetadata(3, 3, 10, 10, 5, nod), convertTo<T>(std::vector<double>{
                                                                      6.0, 7.0, 8.0,
                                                                      11.0, 12.0, 13.0,
                                                                      nod, nod, 5.0}));

        CHECK_RASTER_EQ(expected, sub_raster(raster, RasterMetadata(5, 5, 0, 0, 5, nod)));
    }
}
}
