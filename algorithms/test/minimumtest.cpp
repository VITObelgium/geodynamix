#include "gdx/test/testbase.h"

#include "gdx/algo/minimum.h"

#include <numeric>
#include <random>

namespace gdx::test {

TEST_CASE_TEMPLATE("Minimum", TypeParam, RasterTypes)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    SUBCASE("minimumValue")
    {
        auto nodata = static_cast<double>(std::numeric_limits<T>::lowest());

        std::vector<T> v(s_rows * s_cols);
        std::iota(v.begin() + 1, v.end(), static_cast<T>(5.0));
        std::shuffle(v.begin() + 1, v.end(), std::mt19937(std::random_device()()));

        v[0] = T(nodata);

        Raster raster(RasterMetadata(s_rows, s_cols, nodata), v);

        CHECK(minimum(raster) == static_cast<T>(5.0));
        CHECK(minimum(raster * 2) == static_cast<T>(10.0));
    }

    SUBCASE("minimumValueIntegerNoData")
    {
        const std::vector<double> v = {
            -999.0, -999.0, 4.0, 4.0,
            4.0, 8.0, 4.0, 9.0,
            2.0, 4.0, -999.0, 7.0,
            4.0, 4.0, -5.0, 8.0,
            3.0, -999.0, 4.0, -999.0};

        RasterMetadata meta(5, 4);

        Raster raster1(meta, convertTo<T>(v));
        CHECK(minimum(raster1) == static_cast<T>(-999.0));

        meta.nodata = -999.0;
        Raster raster2(meta, convertTo<T>(v));
        CHECK(minimum(raster2) == static_cast<T>(-5.0));
    }
}

TEST_CASE_TEMPLATE("Minimum", TypeParam, RasterFloatTypes)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    SUBCASE("minimumValueHasNoData")
    {
        auto nan = std::numeric_limits<double>::quiet_NaN();

        const std::vector<double> v = {
            nan, nan, 4.0, 4.0,
            4.0, 8.0, 4.0, 9.0,
            2.0, 4.0, nan, 7.0,
            4.0, 4.0, 4.0, 8.0,
            3.0, nan, 4.0, -5.0};

        Raster raster(RasterMetadata(5, 4, nan), convertTo<T>(v));
        CHECK(minimum(raster) == static_cast<T>(-5.f));
    }

    SUBCASE("minimumValueNoDataAroundMin")
    {
        auto nan = std::numeric_limits<double>::quiet_NaN();

        const std::vector<double> v = {
            nan, nan, 4.0, 4.0,
            4.0, 8.0, 4.0, 9.0,
            nan, -5.0, nan, 7.0,
            4.0, 4.0, 4.0, 8.0,
            3.0, nan, 4.0, 5.0};

        Raster raster(RasterMetadata(5, 4, nan), convertTo<T>(v));
        CHECK(minimum(raster) == static_cast<T>(-5.f));
    }

    SUBCASE("minimumValueOnlyNodata")
    {
        Raster raster(RasterMetadata(5, 4, 9999), 9999);
        CHECK_THROWS_AS_MESSAGE(minimum(raster), InvalidArgument, fmt::format("Actual result: {}", minimum(raster)));
    }

    SUBCASE("minimumValueOnlyNaNNodata")
    {
        Raster raster(RasterMetadata(5, 4, Raster::NaN), Raster::NaN);
        CHECK_THROWS_AS_MESSAGE(minimum(raster), InvalidArgument, fmt::format("Actual result: {}", minimum(raster)));
    }

    SUBCASE("minimumMultipleRasters")
    {
        double nod = -999;

        Raster r1(RasterMetadata(3, 3, nod), convertTo<T>(std::vector<double>({nod, nod, 4.0,
                                                 4.0, 8.0, 4.0,
                                                 nod, -5.0, 1.0})));

        Raster r2(RasterMetadata(3, 3, nod), convertTo<T>(std::vector<double>{nod, 1.0, 3.0,
                                                 5.0, 8.0, 1.0,
                                                 nod, -5.0, nod}));

        Raster expected(RasterMetadata(3, 3, nod), convertTo<T>(std::vector<double>{nod, nod, 3.0,
                                                       4.0, 8.0, 1.0,
                                                       nod, -5.0, nod}));

        CHECK_RASTER_EQ(expected, gdx::minimum<Raster>({&r1, &r2}));
    }

    SUBCASE("minmaxHasNoData")
    {
        auto nan = std::numeric_limits<double>::quiet_NaN();

        const std::vector<double> v = {
            nan, nan, 4.0, 4.0,
            4.0, 8.0, 4.0, 9.0,
            2.0, 4.0, nan, 7.0,
            4.0, 4.0, 4.0, 8.0,
            3.0, nan, 4.0, -5.0};

        Raster raster(RasterMetadata(5, 4, nan), convertTo<T>(v));
        std::pair<T, T> expected(T(-5), T(9));
        CHECK(minmax(raster) == expected);
    }

    SUBCASE("minmaxCellHasNoData")
    {
        auto nan = std::numeric_limits<double>::quiet_NaN();

        const std::vector<double> v = {
            nan, nan, 4.0, 4.0,
            4.0, 8.0, 4.0, 9.0,
            2.0, 4.0, nan, 7.0,
            4.0, 4.0, 4.0, 8.0,
            3.0, nan, 4.0, -5.0};

        Raster raster(RasterMetadata(5, 4, nan), convertTo<T>(v));
        auto [minCell, maxCell] = minmax_cell(raster);
        CHECK(raster[minCell] == T(-5));
        CHECK(raster[maxCell] == T(9));

        CHECK(minCell == Cell(4, 3));
        CHECK(maxCell == Cell(1, 3));
    }
}
}