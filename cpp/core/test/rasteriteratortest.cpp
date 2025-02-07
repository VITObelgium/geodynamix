#include "gdx/rasteriterator.h"
#include "gdx/rasterspan.h"
#include "gdx/sparseraster.h"
#include "gdx/test/testbase.h"
#include "infra/log.h"

#include <doctest/doctest.h>
#include <vector>

namespace gdx::test {

using namespace doctest;
using namespace std::string_literals;

#ifdef GDX_ENABLE_SIMD
#define IteratorRasterTypes RasterValuePair<MaskedRaster, int32_t>, \
                            RasterValuePair<DenseRaster, int32_t>
#else
#define IteratorRasterTypes RasterValuePair<MaskedRaster, int32_t>
#endif

TEST_CASE_TEMPLATE("raster iterator", RasterType, RasterTypes)
{
    using T      = typename RasterType::value_type;
    using Raster = typename RasterType::raster;

    SUBCASE("skip nodata in raster")
    {
        Raster ras(RasterMetadata(3, 3, 0), std::vector<T>{{0, 1, 2, 5, 0, 7, 4, 3, 0}});

        std::vector<T> expectedData = {1, 2, 5, 7, 4, 3};

        std::vector<T> result;
        std::copy(value_begin(ras), value_end(ras), std::back_inserter(result));
        CHECK_CONTAINER_EQ(expectedData, result);
    }

    SUBCASE("skip nodata in const raster")
    {
        const Raster ras(RasterMetadata(3, 3, 0), std::vector<T>{{0, 1, 0, 0, 0, 7, 4, 3, 0}});

        std::vector<T> expectedData = {1, 7, 4, 3};

        std::vector<T> result;
        std::copy(value_begin(ras), value_end(ras), std::back_inserter(result));
        CHECK_CONTAINER_EQ(expectedData, result);
    }

    SUBCASE("skip nodata in const raster constant begin and end")
    {
        const Raster ras(RasterMetadata(3, 3, 0), std::vector<T>{{0, 1, 0, 0, 0, 7, 4, 3, 0}});

        std::vector<T> expectedData = {1, 7, 4, 3};

        std::vector<T> result;
        std::copy(value_cbegin(ras), value_cend(ras), std::back_inserter(result));
        CHECK_CONTAINER_EQ(expectedData, result);
    }

    SUBCASE("skip NaN in nodata raster")
    {
        auto nan = std::numeric_limits<float>::quiet_NaN();

        MaskedRaster<float> ras(RasterMetadata(3, 3, nan), std::vector<float>{
                                                               nan, 0.f, 2.f,
                                                               5.f, nan, 7.f,
                                                               4.f, 9.f, nan});

        std::vector<float> expectedData = {0.f, 2.f, 5.f, 7.f, 4.f, 9.f};

        std::vector<float> result;
        std::copy(value_begin(ras), value_end(ras), std::back_inserter(result));
        CHECK_CONTAINER_EQ(expectedData, result);
    }

    SUBCASE("skip nodata with only nodata present")
    {
        Raster ras(RasterMetadata(3, 3, 1), std::vector<T>{
                                                1, 1, 1,
                                                1, 1, 1,
                                                1, 1, 1});

        std::vector<T> result;
        std::copy(value_begin(ras), value_end(ras), std::back_inserter(result));
        CHECK(result.empty());
    }

    SUBCASE("skip nodata with only NaN nodata present")
    {
        auto nan = std::numeric_limits<float>::quiet_NaN();
        MaskedRaster<float> ras(RasterMetadata(3, 3, 0), std::vector<float>{
                                                             nan, nan, nan,
                                                             nan, nan, nan,
                                                             nan, nan, nan});

        std::vector<float> result;
        std::copy(value_begin(ras), value_end(ras), std::back_inserter(result));
        CHECK(result.empty());
    }

    SUBCASE("skip nodata empty input")
    {
        std::vector<float> data;
        MaskedRaster<float> ras;

        std::vector<float> result;
        std::copy(value_begin(ras), value_end(ras), std::back_inserter(result));
        CHECK(result.empty());
    }

    SUBCASE("skip nodata empty input with nodata value")
    {
        std::vector<float> data;
        MaskedRaster<float> ras;

        std::vector<float> result;
        std::copy(value_begin(ras), value_end(ras), std::back_inserter(result));
        CHECK(result.empty());
    }

    SUBCASE("assign values")
    {
        Raster ras(RasterMetadata(3, 3, 0), std::vector<T>{{0, 1, 2, 5, 0, 7, 4, 3, 0}});
        Raster expected(RasterMetadata(3, 3, 0), std::vector<T>{{0, 9, 9, 9, 0, 9, 9, 9, 0}});

        std::for_each(value_begin(ras), value_end(ras), [](auto& value) {
            value = T(9);
        });

        CHECK_RASTER_EQ(expected, ras);
    }

    SUBCASE("cell info")
    {
        Raster ras(RasterMetadata(3, 3, 0), std::vector<T>{{0, 1, 2,
                                                5, 0, 7,
                                                4, 3, 0}});
        std::vector<Cell> expected{{{0, 1}, {0, 2},
            {1, 0}, {1, 2},
            {2, 0}, {2, 1}}};

        std::vector<Cell> result;
        std::for_each(optional_value_begin(ras), optional_value_end(ras), [&result](auto& value) {
            if (!value.is_nodata()) {
                result.push_back(value.cell());
            }
        });

        CHECK_CONTAINER_EQ(expected, result);
        result.clear();

        std::for_each(value_begin(ras), value_end(ras), [&result](auto& value) {
            result.push_back(value.cell());
        });

        CHECK_CONTAINER_EQ(expected, result);
    }

    SUBCASE("cell iterator")
    {
        Raster ras(RasterMetadata(3, 2, 0), std::vector<T>{{0, 1,
                                                2, 0,
                                                3, 0}});

        std::vector<Cell> expectedCells = {{0, 0}, {0, 1}, {1, 0}, {1, 1}, {2, 0}, {2, 1}};

        std::vector<Cell> cells;
        std::copy(cell_begin(ras), cell_end(ras), std::back_inserter(cells));
        CHECK_CONTAINER_EQ(expectedCells, cells);

        cells.clear();
        for (auto& cell : RasterCells(ras)) {
            cells.push_back(cell);
        }

        CHECK_CONTAINER_EQ(expectedCells, cells);
    }

    SUBCASE("assign values bool operator check")
    {
        Raster ras(RasterMetadata(3, 3, 8), std::vector<T>{{8, 1, 2, 0, 8, 7, 4, 3, 8}});
        Raster expected(RasterMetadata(3, 3, 8), std::vector<T>{{8, 0, 0, 0, 8, 0, 0, 0, 8}});

        std::for_each(optional_value_begin(ras), optional_value_end(ras), [](auto& value) {
            if (value) {
                value = 0;
            }
        });

        CHECK_RASTER_EQ(expected, ras);
    }

    SUBCASE("assign values bool operator check")
    {
        Raster ras(RasterMetadata(3, 3, 8), std::vector<T>{{8, 1, 2, 0, 8, 7, 4, 3, 8}});
        Raster expected(RasterMetadata(3, 3, 8), std::vector<T>{{8, 8, 8, 8, 8, 8, 8, 8, 8}});

        std::for_each(optional_value_begin(ras), optional_value_end(ras), [](auto& value) {
            if (value) {
                value.reset();
            }
        });

        CHECK_RASTER_EQ(expected, ras);
    }

    SUBCASE("assign nodata value operator check")
    {
        Raster ras(RasterMetadata(3, 3, 8), std::vector<T>{{8, 1, 2, 5, 8, 7, 4, 3, 8}});
        Raster expected(RasterMetadata(3, 3, 8), std::vector<T>{{0, 1, 2, 5, 0, 7, 4, 3, 0}});

        std::for_each(optional_value_begin(ras), optional_value_end(ras), [](auto& value) {
            if (!value) {
                value = 0;
            }
        });

        CHECK_RASTER_EQ(expected, ras);
    }

    SUBCASE("assign nodata value is nodata")
    {
        Raster ras(RasterMetadata(3, 3, 8), std::vector<T>{{8, 1, 2, 5, 8, 7, 4, 3, 8}});
        Raster expected(RasterMetadata(3, 3, 8), std::vector<T>{{0, 1, 2, 5, 0, 7, 4, 3, 0}});

        std::for_each(optional_value_begin(ras), optional_value_end(ras), [](auto& value) {
            if (value.is_nodata()) {
                value = 0;
            }
        });

        CHECK_RASTER_EQ(expected, ras);
    }
}

TEST_CASE("raster iterator")
{
    SUBCASE("missing value iterator vector")
    {
        std::vector<int> ras(std::vector<int>{{0, 1, 2, 0, 3, 0}});
        RasterMetadata meta(2, 3, 0);

        auto span = make_raster_span(ras, meta);

        std::vector<int> expectedData = {1, 2, 3};
        std::vector<int> result;

        std::for_each(optional_value_begin(span), optional_value_end(span), [&](auto& value) {
            if (value) {
                result.push_back(*value);
            }
        });

        CHECK_CONTAINER_EQ(expectedData, result);
        CHECK(std::distance(optional_value_begin(span), optional_value_end(span)) == 6);
    }

    SUBCASE("missing value iterator vector float")
    {
        std::vector<float> ras(std::vector<float>{{0, 1, 2, 0, 3, 0}});
        RasterMetadata meta(2, 3, 0);

        auto span = make_raster_span(ras, meta);

        std::vector<float> expectedData = {1, 2, 3};
        std::vector<float> result;

        std::for_each(optional_value_begin(span), optional_value_end(span), [&](auto& value) {
            if (value) {
                result.push_back(*value);
            }
        });

        // TODO: currently fails because the value proxy assumes nan values for floats
        // CHECK_CONTAINER_EQ(expectedData, result);
        inf::Log::error("TODO: Fix this test");
        CHECK(std::distance(optional_value_begin(span), optional_value_end(span)) == 6);
    }

    SUBCASE("missing value iterator dereference")
    {
        std::vector<int> ras(std::vector<int>{{0, 1, 2, 0, 3, 0}});
        RasterMetadata meta(2, 3, 0);

        auto span      = make_raster_span(ras, meta);
        auto constSpan = make_raster_span(std::as_const(ras), meta);

        std::vector<int> expectedData = {1, 2, 3};
        std::vector<int> result;

        std::for_each(optional_value_begin(span), optional_value_end(span), [&](auto& value) {
            if (value) {
                result.push_back(*value);
            }
        });
        CHECK_CONTAINER_EQ(expectedData, result);

        result.clear();
        std::for_each(optional_value_begin(constSpan), optional_value_end(constSpan), [&](auto& value) {
            if (value) {
                result.push_back(*value);
            }
        });
        CHECK_CONTAINER_EQ(expectedData, result);
    }

    SUBCASE("sparse raster")
    {
        std::vector<int> data(std::vector<int>{{-1, 0, 1, -1, 5, -1}});
        SparseRaster<int> raster(RasterMetadata(3, 2, -1), data);

        std::vector<int> result(data.size());
        std::copy(raster.begin(), raster.end(), result.begin());

        CHECK_CONTAINER_EQ(data, result);
        CHECK(data.size() == std::distance(raster.begin(), raster.end()));
    }

    SUBCASE("sparse raster nodata in the middle")
    {
        std::vector<int> data(std::vector<int>{{4, -1, -1, -1, -1, 5}});
        SparseRaster<int> raster(RasterMetadata(3, 2, -1), data);

        std::vector<int> result(data.size());
        std::copy(raster.begin(), raster.end(), result.begin());
        CHECK_CONTAINER_EQ(data, result);
        CHECK(data.size() == std::distance(raster.begin(), raster.end()));
    }

    SUBCASE("sparse raster only nodata")
    {
        std::vector<int> data(std::vector<int>{{-1, -1, -1, -1, -1, -1}});

        SparseRaster<int> raster(RasterMetadata(3, 2, -1), data);

        std::vector<int> result(data.size());
        std::copy(raster.begin(), raster.end(), result.begin());

        CHECK_CONTAINER_EQ(data, result);
    }

    SUBCASE("sparse raster only data")
    {
        std::vector<int> data(std::vector<int>{{1, 2, 3, 4, 5, 6}});

        SparseRaster<int> raster(RasterMetadata(3, 2, -1), data);

        std::vector<int> result(data.size());
        std::copy(raster.begin(), raster.end(), result.begin());

        CHECK_CONTAINER_EQ(data, result);
    }

    SUBCASE("only data values")
    {
        std::vector<int> data(std::vector<int>{{4, -1, -1, -1, -1, 5}});
        SparseRaster<int> raster(RasterMetadata(3, 2, -1), data);

        std::vector<int> result;
        std::copy(value_begin(raster), value_end(raster), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<int>({4, 5}), result);
    }

    SUBCASE("only data values in the middle")
    {
        std::vector<int> data(std::vector<int>{{-1, 1, 2, 3, 4, -1}});
        SparseRaster<int> raster(RasterMetadata(3, 2, -1), data);

        std::vector<int> result;
        std::copy(value_begin(raster), value_end(raster), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<int>({1, 2, 3, 4}), result);
    }
}
}
