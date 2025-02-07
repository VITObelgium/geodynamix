#include "gdx/test/testbase.h"

#include "gdx/algo/cast.h"
#include "gdx/rastercompare.h"

#include <numeric>
#include <random>

namespace gdx::test {

TEST_CASE_TEMPLATE("compare rasters", TypeParam, RasterTypes)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    double nodata = 100.0;
    RasterMetadata meta(3, 3, nodata);

    SUBCASE("equal scalar")
    {
        meta.nodata.reset();
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                1.0, 2.0, 3.0,
                                4.0, 4.0, 4.0,
                                5.0, 6.0, 7.0}));

        ResultRasterType expected(meta, std::vector<uint8_t>{
                                            0, 0, 0,
                                            1, 1, 1,
                                            0, 0, 0});

        auto result = equals(raster, static_cast<T>(4.0));
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("equal scalar has nodata")
    {
        auto nod = nodata;

        Raster raster(meta, convertTo<T>(std::vector<double>{
                                nod, 1.0, 2.0,
                                4.0, nod, 4.0,
                                5.0, 6.0, nod}));

        auto resultMeta   = meta;
        resultMeta.nodata = 255;
        ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                                  255, 0, 0,
                                                  1, 255, 1,
                                                  0, 0, 255});

        auto result = equals(raster, static_cast<T>(4.0));
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("equal raster")
    {
        meta.nodata.reset();
        Raster raster1(meta, convertTo<T>(std::vector<double>{
                                 2.0, 8.0, 4.0,
                                 4.0, 8.0, 4.0,
                                 2.0, 4.0, -1.0}));

        Raster raster2(meta, convertTo<T>(std::vector<double>{
                                 1.0, 7.0, 3.0,
                                 4.0, 8.0, 4.0,
                                 3.0, 5.0, 0.0}));

        ResultRasterType expected(meta, std::vector<uint8_t>{
                                            0, 0, 0,
                                            1, 1, 1,
                                            0, 0, 0});

        auto result = equals(raster1, raster2);
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("equal raster has nodata")
    {
        auto nod = nodata;

        Raster raster1(meta, convertTo<T>(std::vector<double>{
                                 nod, 8.0, 4.0,
                                 4.0, nod, 4.0,
                                 2.0, 4.0, -1.0}));

        Raster raster2(meta, convertTo<T>(std::vector<double>{
                                 1.0, 7.0, 3.0,
                                 4.0, nod, 4.0,
                                 3.0, 5.0, nod}));

        auto resultMeta   = meta;
        resultMeta.nodata = 255;
        ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                                  255, 0, 0,
                                                  1, 255, 1,
                                                  0, 0, 255});

        auto result = equals(raster1, raster2);
        CHECK_RASTER_NEAR(expected, result);
    }
}
}
