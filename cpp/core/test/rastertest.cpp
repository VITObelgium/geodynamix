#include "gdx/test/testbase.h"

#include "gdx/algo/cast.h"
#include "gdx/raster.h"
#include "infra/cast.h"

#include <type_traits>

namespace gdx::test {

TEST_CASE_TEMPLATE("raster", TypeParam, RasterTypes)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    SUBCASE("sum operator")
    {
        RasterType r1, r2, expected;
        r1.resize_and_fill(3, 3, static_cast<T>(6.0));
        r2.resize_and_fill(3, 3, static_cast<T>(3.0));
        expected.resize_and_fill(3, 3, static_cast<T>(9.0));

        auto res = r1 + r2;

        CHECK(expected == res);
    }

    SUBCASE("subtract operator")
    {
        RasterType r1, r2, expected;
        r1.resize_and_fill(s_rows, s_cols, static_cast<T>(4.f));
        r2.resize_and_fill(s_rows, s_cols, static_cast<T>(5.f));
        expected.resize_and_fill(s_rows, s_cols, static_cast<T>(-1.0));

        auto res = r1 - r2;

        CHECK(r1.size() == res.size());
        CHECK(r1.rows() == res.rows());
        CHECK(r1.cols() == res.cols());

        CHECK(expected == res);
    }

    SUBCASE("multiply operator")
    {
        RasterType r1, r2, expected;
        r1.resize_and_fill(s_rows, s_cols, static_cast<T>(4.0));
        r2.resize_and_fill(s_rows, s_cols, static_cast<T>(5.0));
        expected.resize_and_fill(s_rows, s_cols, static_cast<T>(20.0));

        auto res = r1 * r2;

        CHECK(r1.size() == res.size());
        CHECK(r1.rows() == res.rows());
        CHECK(r1.cols() == res.cols());

        CHECK(expected == res);
    }

    SUBCASE("divide operator")
    {
        RasterType r1, r2;
        r1.resize_and_fill(s_rows, s_cols, static_cast<T>(16.0));
        r2.resize_and_fill(s_rows, s_cols, static_cast<T>(8.0));
        decltype(r1 / r2) expected(s_rows, s_cols, 2.f);

        auto res = r1 / r2;

        CHECK(r1.size() == res.size());
        CHECK(r1.rows() == res.rows());
        CHECK(r1.cols() == res.cols());

        CHECK_RASTER_EQ(expected, res);
    }

    SUBCASE("equal to")
    {
        RasterType r1, r2, expected;
        r1.resize_and_fill(s_rows, s_cols, static_cast<T>(4.0));
        r2.resize_and_fill(s_rows, s_cols, static_cast<T>(2.0));

        CHECK_RASTER_NE(r1, r2);

        r2.fill(static_cast<T>(4.0));

        CHECK_RASTER_EQ(r1, r2);
    }

    SUBCASE("subtract scalar operator")
    {
        RasterType raster(5, 4, convertTo<T>(std::vector<double>{0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0}));
        RasterType expected(5, 4, convertTo<T>(std::vector<double>{-1.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0, -1.0}));

        auto result = raster - static_cast<T>(1.0);
        CHECK_RASTER_EQ(expected, result);
    }

    SUBCASE("add scalar operator")
    {
        RasterType raster(5, 4, convertTo<T>(std::vector<double>{0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0}));
        RasterType expected(5, 4, convertTo<T>(std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0}));

        auto result = raster + static_cast<T>(1.0);
        CHECK_RASTER_EQ(expected, result);
    }

    SUBCASE("multiply scalar operator")
    {
        RasterType raster(5, 4, convertTo<T>(std::vector<double>{0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0}));
        RasterType expected(5, 4, convertTo<T>(std::vector<double>{0.0, 2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0, 18.0, 16.0, 14.0, 12.0, 10.0, 8.0, 6.0, 4.0, 2.0, 0.0}));

        auto result = raster * static_cast<T>(2.0);
        CHECK_RASTER_EQ(expected, result);
    }

    SUBCASE("divide scalar operator")
    {
        RasterType raster(5, 4, convertTo<T>(std::vector<double>{0.0, 2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0, 18.0, 16.0, 14.0, 12.0, 10.0, 8.0, 6.0, 4.0, 2.0, 0.0}));
        RasterType expected(5, 4, convertTo<T>(std::vector<double>{0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0}));

        auto result = raster / static_cast<T>(2.0);
        CHECK_RASTER_EQ(expected, result);
    }
}

TEST_CASE_TEMPLATE("raster int", TypeParam, RasterIntTypes)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    SUBCASE("add rasters nodata")
    {
        RasterMetadata meta(2, 3, -1.0);

        RasterType raster1(meta, convertTo<T>(std::vector<double>{
                                     -1.0, -1.0, 2.0,
                                     4.0, -1.0, 6.0}));

        auto metaDifferentNodata   = meta;
        metaDifferentNodata.nodata = -2.0;

        RasterType raster2(metaDifferentNodata, convertTo<T>(std::vector<double>{
                                                    1.0, -2.0, 3.0,
                                                    1.0, -2.0, -2.0}));

        RasterType expected(meta, convertTo<T>(std::vector<double>{
                                      -1.0, -1.0, 5.0,
                                      5.0, -1.0, -1.0}));

        CHECK_RASTER_EQ(expected, raster1 + raster2);
    }

    SUBCASE("subtract rasters nodata")
    {
        RasterMetadata meta(2, 3);
        meta.nodata = -1.0;

        RasterType raster1(meta, convertTo<T>(std::vector<double>{
                                     -1.0, -1.0, 8.0,
                                     4.0, -1.0, 6.0}));

        RasterType raster2(meta, convertTo<T>(std::vector<double>{
                                     1.0, -1.0, 3.0,
                                     1.0, -1.0, -1.0}));

        RasterType expected(meta, convertTo<T>(std::vector<double>{
                                      -1.0, -1.0, 5.0,
                                      3.0, -1.0, -1.0}));

        CHECK(expected == raster1 - raster2);
    }

    SUBCASE("multiply rasters nodata")
    {
        RasterMetadata meta(2, 3);
        meta.nodata = -1.0;

        RasterType raster1(meta, convertTo<T>(std::vector<double>{
                                     -1.0, -1.0, 8.0,
                                     4.0, -1.0, 6.0}));

        RasterType raster2(meta, convertTo<T>(std::vector<double>{
                                     1.0, -1.0, 3.0,
                                     1.0, -1.0, -1.0}));

        RasterType expected(meta, convertTo<T>(std::vector<double>{
                                      -1.0, -1.0, 24.0,
                                      4.0, -1.0, -1.0}));

        CHECK(expected == raster1 * raster2);
    }

    SUBCASE("divide rasters nodata")
    {
        using ResultRasterType = decltype(raster_cast<float>(std::declval<RasterType>()));

        RasterMetadata meta(2, 3);
        meta.nodata = -1.0;

        RasterType raster1(meta, convertTo<T>(std::vector<double>{
                                     -1.0, -1.0, 8.0,
                                     4.0, -1.0, 6.0}));

        RasterType raster2(meta, convertTo<T>(std::vector<double>{
                                     1.0, -1.0, 4.0,
                                     1.0, -1.0, -1.0}));

        ResultRasterType expected(meta, convertTo<float>(std::vector<double>{
                                            -1.0, -1.0, 2.0,
                                            4.0, -1.0, -1.0}));

        auto res = raster1 / raster2;
        static_assert(std::is_same_v<ResultRasterType, decltype(res)>, "Types should match");
        CHECK_RASTER_EQ(expected, res);
    }

    SUBCASE("subtract scalar operator")
    {
        RasterMetadata meta(5, 4);
        meta.nodata = -1.0;

        RasterType raster(meta, convertTo<T>(std::vector<double>{
                                    -1.0, -1.0, 2.0, 3.0,
                                    4.0, 5.0, 6.0, 7.0,
                                    8.0, -1.0, -1.0, 8.0,
                                    7.0, 6.0, 5.0, 4.0,
                                    3.0, 2.0, -1.0, -1.0}));

        RasterType expected(meta, convertTo<T>(std::vector<double>{
                                      -1.0, -1.0, 1.0, 2.0,
                                      3.0, 4.0, 5.0, 6.0,
                                      7.0, -1.0, -1.0, 7.0,
                                      6.0, 5.0, 4.0, 3.0,
                                      2.0, 1.0, -1.0, -1.0}));

        auto result = raster - static_cast<T>(1.0);
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("divide by zero")
    {
        using ResultType = decltype(raster_cast<float>(std::declval<RasterType>()));

        // input rasters do not have no data, nan is used as nodata using RasterType = typename TypeParam::raster;
        RasterType raster1(RasterMetadata(5, 4), 0);
        RasterType raster2(RasterMetadata(5, 4), 0);
        ResultType expected(RasterMetadata(5, 4, 255), 255);
        auto result = raster1 / raster2;
        CHECK_RASTER_EQ(expected, result);
        CHECK(std::isnan(result.nodata().value()));

        // input rasters do have no data, for division still nan used as the result is always floating point
        raster1.set_nodata(99);
        raster2.set_nodata(99);
        expected.set_nodata(99);
        result = raster1 / raster2;
        CHECK_RASTER_EQ(expected, result);
        CHECK(std::isnan(result.nodata().value()));
    }

    SUBCASE("divide int raster becomes float")
    {
        using ResultType = decltype(raster_cast<float>(std::declval<RasterType>()));

        RasterMetadata meta(5, 4);
        RasterType raster1(meta, 0);
        RasterType raster2(meta, 1);

        static_assert(std::is_same_v<decltype(raster1 / raster2), ResultType>, "Integer division should become float");

        meta.nodata = -1.0;
    }

    SUBCASE("add scalar operator")
    {
        RasterMetadata meta(5, 4);
        meta.nodata = -1.0;

        RasterType raster(meta, convertTo<T>(std::vector<double>{
                                    -1.0, -1.0, 2.0, 3.0,
                                    4.0, 5.0, 6.0, 7.0,
                                    8.0, -1.0, -1.0, 8.0,
                                    7.0, 6.0, 5.0, 4.0,
                                    3.0, 2.0, -1.0, -1.0}));

        RasterType expected(meta, convertTo<T>(std::vector<double>{
                                      -1.0, -1.0, 3.0, 4.0,
                                      5.0, 6.0, 7.0, 8.0,
                                      9.0, -1.0, -1.0, 9.0,
                                      8.0, 7.0, 6.0, 5.0,
                                      4.0, 3.0, -1.0, -1.0}));

        auto result = raster + static_cast<T>(1.0);

        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("multiply scalar operator")
    {
        constexpr auto nod = -1.0;

        RasterMetadata meta(5, 4, nod);
        using RasterType = typename TypeParam::raster;

        RasterType raster(meta, convertTo<T>(std::vector<double>{
                                    nod, nod, 2.0, 3.0,
                                    4.0, 5.0, 6.0, 7.0,
                                    8.0, nod, nod, 8.0,
                                    7.0, 6.0, 5.0, 4.0,
                                    3.0, 2.0, nod, nod}));

        RasterType expected(meta, convertTo<T>(std::vector<double>{
                                      nod, nod, 4.0, 6.0,
                                      8.0, 10.0, 12.0, 14.0,
                                      16.0, nod, nod, 16.0,
                                      14.0, 12.0, 10.0, 8.0,
                                      6.0, 4.0, nod, nod}));

        auto result = raster * static_cast<T>(2.0);
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("divide scalar operator")
    {
        constexpr auto nod = -1.0;
        RasterMetadata meta(5, 4, nod);

        RasterType raster(meta, convertTo<T>(std::vector<double>{
                                    nod, nod, 4.0, 6.0,
                                    8.0, 10.0, 12.0, 14.0,
                                    16.0, nod, nod, 16.0,
                                    14.0, 12.0, 10.0, 8.0,
                                    6.0, 4.0, nod, nod}));

        RasterType expected(meta, convertTo<T>(std::vector<double>{
                                      nod, nod, 2.0, 3.0,
                                      4.0, 5.0, 6.0, 7.0,
                                      8.0, nod, nod, 8.0,
                                      7.0, 6.0, 5.0, 4.0,
                                      3.0, 2.0, nod, nod}));

        auto result = raster / static_cast<T>(2.0);
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("fill values")
    {
        constexpr auto nod = -1.0;

        RasterMetadata meta(5, 4, nod);
        using RasterType = typename TypeParam::raster;

        RasterType raster(meta, convertTo<T>(std::vector<double>{
                                    nod, nod, 2.0, 3.0,
                                    4.0, 5.0, 6.0, 7.0,
                                    8.0, nod, nod, 8.0,
                                    7.0, 6.0, 5.0, 4.0,
                                    3.0, 2.0, nod, nod}));

        RasterType expected(meta, convertTo<T>(std::vector<double>{
                                      nod, nod, 1.0, 1.0,
                                      1.0, 1.0, 1.0, 1.0,
                                      1.0, nod, nod, 1.0,
                                      1.0, 1.0, 1.0, 1.0,
                                      1.0, 1.0, nod, nod}));

        raster.fill_values(inf::truncate<T>(1.0));
        CHECK_RASTER_NEAR(expected, raster);
    }
}

TEST_CASE_TEMPLATE("raster float", TypeParam, RasterFloatTypes)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    SUBCASE("subtract scalar operator")
    {
        constexpr auto nan = -1.0;

        RasterMetadata meta(5, 4);
        meta.nodata = -1.0;

        RasterType raster(meta, convertTo<T>(std::vector<double>{
                                    nan, nan, 2.0, 3.0,
                                    4.0, 5.0, 6.0, 7.0,
                                    8.0, nan, nan, 8.0,
                                    7.0, 6.0, 5.0, 4.0,
                                    3.0, 2.0, nan, nan}));

        RasterType expected(meta, convertTo<T>(std::vector<double>{
                                      nan, nan, 1.0, 2.0,
                                      3.0, 4.0, 5.0, 6.0,
                                      7.0, nan, nan, 7.0,
                                      6.0, 5.0, 4.0, 3.0,
                                      2.0, 1.0, nan, nan}));

        auto result = raster - static_cast<T>(1.0);
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("subtract scalar operator scalar first")
    {
        constexpr auto nod = -1.0;

        RasterMetadata meta(5, 4);
        meta.nodata = -1.0;

        RasterType raster(meta, convertTo<T>(std::vector<double>{
                                    nod, nod, 2.0, 3.0,
                                    4.0, 5.0, 6.0, 7.0,
                                    8.0, nod, nod, 8.0,
                                    7.0, 6.0, 5.0, 4.0,
                                    3.0, 2.0, nod, nod}));

        RasterType expected(meta, convertTo<T>(std::vector<double>{
                                      nod, nod, 8.0, 7.0,
                                      6.0, 5.0, 4.0, 3.0,
                                      2.0, nod, nod, 2.0,
                                      3.0, 4.0, 5.0, 6.0,
                                      7.0, 8.0, nod, nod}));

        auto result = static_cast<T>(10.0) - raster;
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("divide scalar operator scalar first")
    {
        constexpr auto nod = -1.0;

        RasterMetadata meta(5, 4);
        meta.nodata = -1.0;

        RasterType raster(meta, convertTo<T>(std::vector<double>{
                                    nod, nod, 2.0, 3.0,
                                    4.0, 4.0, 6.0, 24.0,
                                    8.0, nod, nod, 8.0,
                                    48.0, 6.0, 12.0, 4.0,
                                    3.0, 8.0, nod, nod}));

        auto result = static_cast<T>(24) / raster;

        decltype(result) expected(meta, convertTo<typename decltype(result)::value_type>(std::vector<double>{
                                            nod, nod, 12.0, 8.0,
                                            6.0, 6.0, 4.0, 1.0,
                                            3.0, nod, nod, 3.0,
                                            0.5, 4.0, 2.0, 6.0,
                                            8.0, 3.0, nod, nod}));

        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("add scalar operator")
    {
        constexpr auto nod = -1.0;

        RasterType raster(RasterMetadata(5, 4, nod), convertTo<T>(std::vector<double>{
                                                         nod, nod, 2.0, 3.0,
                                                         4.0, 5.0, 6.0, 7.0,
                                                         8.0, nod, nod, 8.0,
                                                         7.0, 6.0, 5.0, 4.0,
                                                         3.0, 2.0, nod, nod}));

        RasterType expected(RasterMetadata(5, 4, nod), convertTo<T>(std::vector<double>{
                                                           nod, nod, 3.0, 4.0,
                                                           5.0, 6.0, 7.0, 8.0,
                                                           9.0, nod, nod, 9.0,
                                                           8.0, 7.0, 6.0, 5.0,
                                                           4.0, 3.0, nod, nod}));

        auto result = raster + static_cast<T>(1.0);
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("multiply scalar operator")
    {
        constexpr auto nod = -1.0;
        RasterMetadata meta(5, 4, nod);

        RasterType raster(meta, convertTo<T>(std::vector<double>{
                                    nod, nod, 2.0, 3.0,
                                    4.0, 5.0, 6.0, 7.0,
                                    8.0, nod, nod, 8.0,
                                    7.0, 6.0, 5.0, 4.0,
                                    3.0, 2.0, nod, nod}));

        RasterType expected(meta, convertTo<T>(std::vector<double>{
                                      nod, nod, 4.0, 6.0,
                                      8.0, 10.0, 12.0, 14.0,
                                      16.0, nod, nod, 16.0,
                                      14.0, 12.0, 10.0, 8.0,
                                      6.0, 4.0, nod, nod}));

        auto result = raster * static_cast<T>(2.0);
        CHECK_RASTER_NEAR(expected, result);
    }

    SUBCASE("divide scalar operator")
    {
        constexpr auto nod = -1.0;
        RasterMetadata meta(5, 4, nod);

        RasterType raster(meta, convertTo<T>(std::vector<double>{
                                    nod, nod, 4.0, 6.0,
                                    8.0, 10.0, 12.0, 14.0,
                                    16.0, nod, nod, 16.0,
                                    14.0, 12.0, 10.0, 8.0,
                                    6.0, 4.0, nod, nod}));

        RasterType expected(meta, convertTo<T>(std::vector<double>{
                                      nod, nod, 2.0, 3.0,
                                      4.0, 5.0, 6.0, 7.0,
                                      8.0, nod, nod, 8.0,
                                      7.0, 6.0, 5.0, 4.0,
                                      3.0, 2.0, nod, nod}));

        auto result = raster / static_cast<T>(2.0);
        CHECK_RASTER_NEAR(expected, result);
    }
}

TEST_CASE("masked raster")
{
    SUBCASE("multiply operator different types")
    {
        MaskedRaster<float> floatRas(s_rows, s_cols, 2.f);
        MaskedRaster<uint8_t> intRas(s_rows, s_cols, 3);
        MaskedRaster<float> expected(s_rows, s_cols, 6.f);

        auto res = floatRas * intRas;

        CHECK(expected.tolerant_equal_to(res));
    }

    SUBCASE("multiply different types")
    {
        MaskedRaster<float> r1, expected;
        MaskedRaster<int32_t> r2;

        r1.resize_and_fill(3, 3, 4.f);
        r2.resize_and_fill(3, 3, 5);
        expected.resize_and_fill(3, 3, 20.f);

        CHECK_RASTER_EQ(expected, r1 * r2);
    }
}

}
