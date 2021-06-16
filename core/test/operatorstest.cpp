#include "gdx/algo/cast.h"
#include "gdx/test/testbase.h"

#include <numeric>
#include <random>

namespace gdx::test {

TEST_CASE_TEMPLATE("raster operators", TypeParam, RasterTypes)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;

    double nod = 100.0;
    RasterMetadata meta(3, 3, nod);

    SUBCASE("less equal scalar")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

        meta.nodata.reset();
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                1.0, 2.0, 3.0,
                                4.0, 4.0, 4.0,
                                5.0, 6.0, 7.0}));

        auto resultMeta = meta;
        ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                                  1, 1, 1,
                                                  1, 1, 1,
                                                  0, 0, 0});

        auto result = raster <= static_cast<T>(4.0);
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("less equal scalar has nodata")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

        Raster raster(meta, convertTo<T>(std::vector<double>{
                                nod, 1.0, 2.0,
                                4.0, nod, 4.0,
                                5.0, 6.0, nod}));

        auto resultMeta   = meta;
        resultMeta.nodata = 255;
        ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                                  255, 1, 1,
                                                  1, 255, 1,
                                                  0, 0, 255});

        auto result = raster <= static_cast<T>(4.0);
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("less equal raster")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

        Raster raster1(3, 3, convertTo<T>(std::vector<double>{2.0, 8.0, 4.0, 4.0, 8.0, 4.0, 2.0, 4.0, -1.0}));
        Raster raster2(3, 3, convertTo<T>(std::vector<double>{1.0, 7.0, 3.0, 4.0, 8.0, 4.0, 3.0, 5.0, 0.0}));

        ResultRasterType expected(3, 3, std::vector<uint8_t>{0, 0, 0, 1, 1, 1, 1, 1, 1});

        auto result = raster1 <= raster2;
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("less equal raster has nodata")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

        Raster raster1(meta, convertTo<T>(std::vector<double>{
                                 nod, 8.0, 4.0,
                                 4.0, nod, 4.0,
                                 2.0, 4.0, -1.0}));

        Raster raster2(meta, convertTo<T>(std::vector<double>{
                                 1.0, 7.0, 3.0,
                                 4.0, nod, 4.0,
                                 3.0, 5.0, nod}));

        auto resultMeta   = meta;
        resultMeta.nodata = 255.0;
        ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                                  255, 0, 0,
                                                  1, 255, 1,
                                                  1, 1, 255});

        auto result = raster1 <= raster2;
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("less scalar")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

        meta.nodata.reset();
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                1.0, 2.0, 3.0,
                                4.0, 4.0, 4.0,
                                5.0, 6.0, 7.0}));

        ResultRasterType expected(meta, std::vector<uint8_t>{
                                            1, 1, 1,
                                            0, 0, 0,
                                            0, 0, 0});

        auto result = raster < static_cast<T>(4.0);
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("less scalar has nodata")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

        Raster raster(meta, convertTo<T>(std::vector<double>{
                                nod, 1.0, 2.0,
                                4.0, nod, 4.0,
                                5.0, 6.0, nod}));

        auto resultMeta   = meta;
        resultMeta.nodata = 255;
        ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                                  255, 1, 1,
                                                  0, 255, 0,
                                                  0, 0, 255});

        auto result = raster < static_cast<T>(4.0);
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("less raster")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

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
                                            0, 0, 0,
                                            1, 1, 1});

        auto result = raster1 < raster2;
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("less raster has nodata")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

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
                                                  0, 255, 0,
                                                  1, 1, 255});

        auto result = raster1 < raster2;
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("greater scalar")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

        meta.nodata.reset();
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                1.0, 2.0, 3.0,
                                4.0, 4.0, 4.0,
                                5.0, 6.0, 7.0}));

        ResultRasterType expected(meta, std::vector<uint8_t>{
                                            0, 0, 0,
                                            0, 0, 0,
                                            1, 1, 1});

        auto result = raster > static_cast<T>(4.0);
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("greater scalar has nodata")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

        Raster raster(meta, convertTo<T>(std::vector<double>{
                                nod, 1.0, 2.0,
                                4.0, nod, 4.0,
                                5.0, 6.0, nod}));

        auto resultMeta   = meta;
        resultMeta.nodata = 255;
        ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                                  255, 0, 0,
                                                  0, 255, 0,
                                                  1, 1, 255});

        auto result = raster > static_cast<T>(4.0);
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("greater raster")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

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
                                            1, 1, 1,
                                            0, 0, 0,
                                            0, 0, 0});

        auto result = raster1 > raster2;
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("greater raster has nodata")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

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
                                                  255, 1, 1,
                                                  0, 255, 0,
                                                  0, 0, 255});

        auto result = raster1 > raster2;
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("greater equal scalar")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

        meta.nodata.reset();
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                1.0, 2.0, 3.0,
                                4.0, 4.0, 4.0,
                                5.0, 6.0, 7.0}));

        ResultRasterType expected(meta, std::vector<uint8_t>{
                                            0, 0, 0,
                                            1, 1, 1,
                                            1, 1, 1});

        auto result = raster >= static_cast<T>(4.0);
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("greater equal scalar has nodata")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

        Raster raster(meta, convertTo<T>(std::vector<double>{
                                nod, 1.0, 2.0,
                                4.0, nod, 4.0,
                                5.0, 6.0, nod}));

        auto resultMeta   = meta;
        resultMeta.nodata = 255;
        ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                                  255, 0, 0,
                                                  1, 255, 1,
                                                  1, 1, 255});

        auto result = raster >= static_cast<T>(4.0);
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("greater equal raster")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

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
                                            1, 1, 1,
                                            1, 1, 1,
                                            0, 0, 0});

        auto result = raster1 >= raster2;
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("greater equal raster has nodata")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

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
                                                  255, 1, 1,
                                                  1, 255, 1,
                                                  0, 0, 255});

        auto result = raster1 >= raster2;
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("not equal scalar")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

        meta.nodata.reset();
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                1.0, 2.0, 3.0,
                                4.0, 4.0, 4.0,
                                5.0, 6.0, 7.0}));

        ResultRasterType expected(meta, std::vector<uint8_t>{
                                            1, 1, 1,
                                            0, 0, 0,
                                            1, 1, 1});

        auto result = raster.not_equals(static_cast<T>(4.0));
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("not equal sscalar matches nodata")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

        Raster raster(meta, convertTo<T>(std::vector<double>{
                                1.0, 2.0, nod,
                                4.0, nod, 4.0,
                                nod, 6.0, 7.0}));

        auto resultMeta   = meta;
        resultMeta.nodata = 255;
        ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                                  1, 1, 255,
                                                  1, 255, 1,
                                                  255, 1, 1});

        auto result = raster.not_equals(static_cast<T>(0.0));
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("not equal scalar has nodata")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

        Raster raster(meta, convertTo<T>(std::vector<double>{
                                nod, 1.0, 2.0,
                                4.0, nod, 4.0,
                                5.0, 6.0, nod}));

        auto resultMeta   = meta;
        resultMeta.nodata = 255;
        ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                                  255, 1, 1,
                                                  0, 255, 0,
                                                  1, 1, 255});

        auto result = raster.not_equals(static_cast<T>(4.0));
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("not equal raster")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

        auto resultMeta = meta;
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
                                            1, 1, 1,
                                            0, 0, 0,
                                            1, 1, 1});

        auto result = raster1.not_equals(raster2);
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("not equal raster has nodata")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

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
                                                  255, 1, 1,
                                                  0, 255, 0,
                                                  1, 1, 255});

        auto result = raster1.not_equals(raster2);
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("logical and")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

        meta.nodata.reset();
        Raster raster1(meta, convertTo<T>(std::vector<double>{
                                 0.0, 0.0, 0.0,
                                 4.0, 8.0, 0.0,
                                 2.0, 4.0, -1.0}));

        Raster raster2(meta, convertTo<T>(std::vector<double>{
                                 1.0, 7.0, 3.0,
                                 4.0, 8.0, 0.0,
                                 0.0, 0.0, 0.0}));

        ResultRasterType expected(meta, std::vector<uint8_t>{
                                            0, 0, 0,
                                            1, 1, 0,
                                            0, 0, 0});

        auto result = raster1 && raster2;
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("logical and has nodata")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

        Raster raster1(meta, convertTo<T>(std::vector<double>{
                                 nod, 0.0, 0.0,
                                 4.0, nod, 0.0,
                                 2.0, 4.0, -1.0}));

        Raster raster2(meta, convertTo<T>(std::vector<double>{
                                 1.0, 7.0, 3.0,
                                 4.0, nod, 0.0,
                                 0.0, 0.0, nod}));

        auto resultMeta   = meta;
        resultMeta.nodata = 255;
        ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                                  255, 0, 0,
                                                  1, 255, 0,
                                                  0, 0, 255});

        auto result = raster1 && raster2;
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("logical or")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

        meta.nodata.reset();

        Raster raster1(meta, convertTo<T>(std::vector<double>{
                                 0.0, 0.0, 0.0,
                                 4.0, 8.0, 0.0,
                                 2.0, 4.0, -1.0}));

        Raster raster2(meta, convertTo<T>(std::vector<double>{
                                 1.0, 7.0, 3.0,
                                 4.0, 8.0, 0.0,
                                 0.0, 0.0, 0.0}));

        ResultRasterType expected(meta, std::vector<uint8_t>{
                                            1, 1, 1,
                                            1, 1, 0,
                                            1, 1, 1});

        auto result = raster1 || raster2;
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("logical or has nodata")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

        Raster raster1(meta, convertTo<T>(std::vector<double>{
                                 nod, 0.0, 0.0,
                                 4.0, nod, 0.0,
                                 2.0, 4.0, -1.0}));

        Raster raster2(meta, convertTo<T>(std::vector<double>{
                                 1.0, 7.0, 3.0,
                                 4.0, nod, 0.0,
                                 0.0, 0.0, nod}));

        auto resultMeta   = meta;
        resultMeta.nodata = 255;
        ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                                  255, 1, 1,
                                                  1, 255, 0,
                                                  1, 1, 255});

        auto result = raster1 || raster2;
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("logical not")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

        meta.nodata.reset();
        Raster raster1(meta, convertTo<T>(std::vector<double>{
                                 0.0, 0.0, 0.0,
                                 4.0, 8.0, 0.0,
                                 2.0, 4.0, -1.0}));

        ResultRasterType expected(meta, std::vector<uint8_t>{
                                            1, 1, 1,
                                            0, 0, 1,
                                            0, 0, 0});

        auto result = !raster1;
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("logical not has nodata")
    {
        using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

        Raster raster1(meta, convertTo<T>(std::vector<double>{
                                 nod, 0.0, 0.0,
                                 4.0, nod, 0.0,
                                 2.0, 4.0, -1.0}));

        auto resultMeta   = meta;
        resultMeta.nodata = 255;
        ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                                  255, 1, 1,
                                                  0, 255, 1,
                                                  0, 0, 0});

        auto result = !raster1;
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("minus")
    {
        Raster raster1(3, 3, convertTo<T>(std::vector<double>{0.0, 0.0, 0.0, 4.0, 8.0, 1.0, -2.0, -4.0, -1.0}));
        Raster expected(3, 3, convertTo<T>(std::vector<double>{0.0, 0.0, 0.0, -4.0, -8.0, -1.0, 2.0, 4.0, 1.0}));

        auto result = -raster1;
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("minus has nodata")
    {
        Raster raster1(meta, convertTo<T>(std::vector<double>{
                                 nod, 0.0, 0.0,
                                 4.0, nod, 1.0,
                                 -2.0, -4.0, -1.0}));

        Raster expected(meta, convertTo<T>(std::vector<double>{
                                  nod, 0.0, 0.0,
                                  -4.0, nod, -1.0,
                                  2.0, 4.0, 1.0}));

        auto result = -raster1;
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("add operator nodata")
    {
        Raster raster1(meta, convertTo<T>(std::vector<double>{
                                 nod, 2.0, 2.0,
                                 3.0, nod, 3.0,
                                 1.0, 1.0, 0.0}));

        Raster raster2(meta, convertTo<T>(std::vector<double>{
                                 1.0, 3.0, 3.0,
                                 3.0, nod, 3.0,
                                 3.0, 3.0, nod}));

        Raster expected(meta, convertTo<T>(std::vector<double>{
                                  nod, 5.0, 5.0,
                                  6.0, nod, 6.0,
                                  4.0, 4.0, nod}));

        Raster result;
        SUBCASE("inplace")
        {
            result = raster1.copy();
            result += raster2;
        }

        SUBCASE("not inplace")
        {
            result = raster1 + raster2;
        }

        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("add operator nodata rhs")
    {
        auto meta1 = meta;
        meta1.nodata.reset();
        Raster raster1(meta1, convertTo<T>(std::vector<double>{
                                  1.0, 2.0, 2.0,
                                  3.0, 4.0, 5.0,
                                  6.0, 7.0, 8.0}));

        Raster raster2(meta, convertTo<T>(std::vector<double>{
                                 1.0, 3.0, 3.0,
                                 3.0, nod, 3.0,
                                 3.0, 3.0, nod}));

        Raster expected(meta, convertTo<T>(std::vector<double>{
                                  2.0, 5.0, 5.0,
                                  6.0, nod, 8.0,
                                  9.0, 10.0, nod}));

        REQUIRE_FALSE(raster1.nodata().has_value());
        REQUIRE(raster2.nodata().has_value());

        SUBCASE("inplace")
        {
            CHECK_RASTER_NEAR(expected, raster1 += raster2);
        }

        SUBCASE("not inplace")
        {
            CHECK_RASTER_NEAR(expected, raster1 + raster2);
        }
    }

    SUBCASE("add operator scalar nodata")
    {
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                nod, 2.0, 2.0,
                                3.0, nod, 3.0,
                                1.0, 1.0, 0.0}));

        Raster expected(meta, convertTo<T>(std::vector<double>{
                                  nod, 4.0, 4.0,
                                  5.0, nod, 5.0,
                                  3.0, 3.0, 2.0}));

        SUBCASE("inplace")
        {
            raster += T(2);
        }

        SUBCASE("not inplace ")
        {
            raster = raster + T(2);
        }

        CHECK_RASTER_NEAR(expected, raster);
    }

    SUBCASE("multiply operator nodata")
    {
        Raster raster1(meta, convertTo<T>(std::vector<double>{
                                 nod, 2.0, 2.0,
                                 3.0, nod, 3.0,
                                 1.0, 1.0, 0.0}));

        Raster raster2(meta, convertTo<T>(std::vector<double>{
                                 1.0, 3.0, 3.0,
                                 3.0, nod, 3.0,
                                 3.0, 3.0, nod}));

        Raster expected(meta, convertTo<T>(std::vector<double>{
                                  nod, 6.0, 6.0,
                                  9.0, nod, 9.0,
                                  3.0, 3.0, nod}));

        auto result = raster1 * raster2;
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("multiply operator scalar nodata")
    {
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                nod, 2.0, 2.0,
                                3.0, nod, 3.0,
                                1.0, 1.0, 0.0}));

        Raster expected(meta, convertTo<T>(std::vector<double>{
                                  nod, 4.0, 4.0,
                                  6.0, nod, 6.0,
                                  2.0, 2.0, 0.0}));

        SUBCASE("inplace")
        {
            raster *= T(2);
        }

        SUBCASE("not inplace")
        {
            raster = raster * T(2);
        }

        CHECK_RASTER_NEAR(expected, raster);
    }

    SUBCASE("multiply operator nodata rhs")
    {
        auto leftMeta = meta;
        leftMeta.nodata.reset();
        Raster raster1(leftMeta, convertTo<T>(std::vector<double>{
                                     1.0, 2.0, 2.0,
                                     3.0, 3.0, 3.0,
                                     1.0, 1.0, 0.0}));

        Raster raster2(meta, convertTo<T>(std::vector<double>{
                                 1.0, 3.0, 3.0,
                                 3.0, nod, 3.0,
                                 3.0, 3.0, nod}));

        Raster expected(meta, convertTo<T>(std::vector<double>{
                                  1.0, 6.0, 6.0,
                                  9.0, nod, 9.0,
                                  3.0, 3.0, nod}));

        SUBCASE("inplace")
        {
            CHECK_RASTER_NEAR(expected, raster1 *= raster2);
        }

        SUBCASE("not inplace")
        {
            CHECK_RASTER_NEAR(expected, raster1 * raster2);
        }
    }

    SUBCASE("multiply operator all data")
    {
        meta.nodata.reset();
        Raster raster1(meta, convertTo<T>(std::vector<double>{
                                 1.0, 2.0, 2.0,
                                 3.0, 3.0, 3.0,
                                 1.0, 1.0, 2.0}));

        Raster raster2(meta, convertTo<T>(std::vector<double>{
                                 1.0, 3.0, 3.0,
                                 3.0, 3.0, 3.0,
                                 3.0, 3.0, 5.0}));

        Raster expected(meta, convertTo<T>(std::vector<double>{
                                  1.0, 6.0, 6.0,
                                  9.0, 9.0, 9.0,
                                  3.0, 3.0, 10.0}));

        SUBCASE("inplace")
        {
            CHECK_RASTER_NEAR(expected, raster1 *= raster2);
        }

        SUBCASE("not inplace")
        {
            CHECK_RASTER_NEAR(expected, raster1 * raster2);
        }
    }

    SUBCASE("divide operator scalar nodata")
    {
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                nod, 6.0, 4.0,
                                8.0, nod, 2.0,
                                10.0, 0.0, 2.0}));

        Raster expected(meta, convertTo<T>(std::vector<double>{
                                  nod, 3.0, 2.0,
                                  4.0, nod, 1.0,
                                  5.0, 0.0, 1.0}));

        SUBCASE("inplace")
        {
            raster /= T(2);
        }

        SUBCASE("not inplace")
        {
            raster = raster / T(2);
        }

        CHECK_RASTER_NEAR(expected, raster);
    }

    SUBCASE("divide operator nodata rhs")
    {
        auto leftMeta = meta;
        leftMeta.nodata.reset();
        Raster raster1(leftMeta, convertTo<T>(std::vector<double>{
                                     8.0, 10.0, 12.0,
                                     30.0, 40.0, 50.0,
                                     1.0, 1.0, 0.0}));

        Raster raster2(meta, convertTo<T>(std::vector<double>{
                                 0.0, 2.0, 3.0,
                                 3.0, nod, 0.0,
                                 5.0, 2.0, nod}));

        SUBCASE("inplace")
        {
            const std::vector<double> expectedData = {
                nod, 5.0, 4.0,
                10.0, nod, nod,
                double(inf::truncate<T>(0.2)), double(inf::truncate<T>(0.5)), nod}; // truncate to drop value after digit in case of int raster

            Raster expected(meta, convertTo<T>(expectedData));
            CHECK_RASTER_NEAR(expected, raster1 /= raster2);
        }

        SUBCASE("not inplace")
        {
            using TResult      = decltype(0.f / T()); // use float or double as result type
            using ResultRaster = decltype(raster_cast<TResult>(std::declval<Raster>()));

            nod                                    = ResultRaster::NaN;
            const std::vector<double> expectedData = {
                nod, 5.0, 4.0,
                10.0, nod, nod,
                0.2, 0.5, nod};
            ResultRaster expected(inf::copy_metadata_replace_nodata(meta, ResultRaster::NaN), convertTo<TResult>(expectedData));
            CHECK_RASTER_NEAR(expected, raster1 / raster2);
        }
    }

    SUBCASE("divide operator all data")
    {
        meta.nodata.reset();
        Raster raster1(meta, convertTo<T>(std::vector<double>{
                                 1.0, 4.0, 4.0,
                                 3.0, 3.0, 3.0,
                                 1.0, 5.0, 2.0}));

        Raster raster2(meta, convertTo<T>(std::vector<double>{
                                 0.0, 2.0, 8.0,
                                 3.0, 0.0, 3.0,
                                 4.0, 2.0, 0.0}));

        if constexpr (Raster::has_nan()) {
            meta.nodata = Raster::NaN;
        } else {
            meta.nodata = double(std::numeric_limits<T>::max());
        }

        SUBCASE("inplace")
        {
            auto nod = meta.nodata.value();

            std::vector<double> expectedData = {
                nod, 2.0, double(inf::truncate<T>(0.5)),
                1.0, nod, 1.0,
                double(inf::truncate<T>(0.25)), double(inf::truncate<T>(2.5)), nod};

            Raster expected(meta, convertTo<T>(expectedData));
            CHECK_RASTER_NEAR(expected, raster1 /= raster2);
        }

        SUBCASE("not inplace")
        {
            using TResult      = decltype(0.f / T()); // use float or double as result type
            using ResultRaster = decltype(raster_cast<TResult>(std::declval<Raster>()));
            meta.nodata        = ResultRaster::NaN;
            auto nod           = meta.nodata.value();

            std::vector<double> expectedData = {
                nod, 2.0, 0.5,
                1.0, nod, 1.0,
                0.25, 2.5, nod};

            ResultRaster expected(inf::copy_metadata_replace_nodata(meta, ResultRaster::NaN), convertTo<TResult>(expectedData));
            CHECK_RASTER_NEAR(expected, raster1 / raster2);
        }
    }

    SUBCASE("multiply operator nodata different types uint8_t float")
    {
        using UintRaster  = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
        using FloatRaster = decltype(raster_cast<float>(std::declval<Raster>()));

        uint8_t nod = 255;
        float fnod  = 255.f;

        RasterMetadata intMeta(3, 3, nod);
        RasterMetadata floatMeta(3, 3, nod);

        UintRaster raster1(intMeta, std::vector<uint8_t>{
                                        nod, 2, 2,
                                        3, nod, 3,
                                        1, 1, 0});

        FloatRaster raster2(floatMeta, std::vector<float>{
                                           1.0, 3.0, 3.0,
                                           3.0, fnod, 3.0,
                                           3.0, 3.0, fnod});

        FloatRaster expected(floatMeta, std::vector<float>{
                                            fnod, 6.0, 6.0,
                                            9.0, fnod, 9.0,
                                            3.0, 3.0, fnod});

        auto result1 = raster1 * raster2;
        CHECK_RASTER_NEAR(expected, result1);

        auto result2 = raster2 * raster1;
        CHECK_RASTER_NEAR(expected, result2);
    }

    SUBCASE("multiply operator nodata different types uint16_t float")
    {
        using UintRaster  = decltype(raster_cast<uint16_t>(std::declval<Raster>()));
        using FloatRaster = decltype(raster_cast<float>(std::declval<Raster>()));

        uint8_t nod = 255;
        float fnod  = 255.f;

        RasterMetadata intMeta(5, 5, nod);
        RasterMetadata floatMeta(5, 5, fnod);

        UintRaster raster1(intMeta, std::vector<uint16_t>{
                                        nod, 2, 2, 2, 2,
                                        3, nod, 3, 3, 3,
                                        4, 4, nod, 4, 4,
                                        5, 5, 5, nod, 5,
                                        1, 1, 1, 1, 0});

        FloatRaster raster2(floatMeta, std::vector<float>{
                                           1.0, 3.0, 3.0, 3.0, 3.0,
                                           3.0, fnod, 3.0, 3.0, 3.0,
                                           4.0, 4.0, fnod, 4.0, 4.0,
                                           5.0, 5.0, fnod, 5.0, 5.0,
                                           3.0, 3.0, 3.0, 3.0, fnod});

        FloatRaster expected(floatMeta, std::vector<float>{
                                            fnod, 6.0, 6.0, 6.0, 6.0,
                                            9.0, fnod, 9.0, 9.0, 9.0,
                                            16.0, 16.0, fnod, 16.0, 16.0,
                                            25.0, 25.0, fnod, fnod, 25.0,
                                            3.0, 3.0, 3.0, 3.0, fnod});

        auto result1 = raster1 * raster2;
        CHECK_RASTER_NEAR(expected, result1);

        auto result2 = raster2 * raster1;
        CHECK_RASTER_NEAR(expected, result2);
    }
}
}
