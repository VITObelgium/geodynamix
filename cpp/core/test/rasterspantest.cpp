#include "gdx/rasterspan.h"
#include "gdx/maskedraster.h"
#include "gdx/rasteriterator.h"

#include <algorithm>
#include <doctest/doctest.h>

namespace gdx::test {

using namespace doctest;

TEST_CASE("ValueSpanTest.assignVector")
{
    std::vector<int> data = {1, 2, 3, 4};
    RasterMetadata meta(2, 2, 2);

    auto dataSpan = make_raster_span(data, meta);

    CHECK(dataSpan.size() == 4);
    CHECK(dataSpan.nodata() == 2);

    CHECK(dataSpan[0] == 1);
    CHECK(dataSpan[1] == 2);
    CHECK(dataSpan[2] == 3);
    CHECK(dataSpan[3] == 4);
}

TEST_CASE("ValueSpanTest.assignMaskedRaster")
{
    MaskedRaster<int> raster(RasterMetadata(2, 3, 1), std::vector<int>{1, 2, 3, 4, 5, 6});

    auto dataSpan = make_raster_span(raster, raster.metadata());

    CHECK(dataSpan.size() == 6);
    CHECK(dataSpan.nodata() == 1);

    CHECK(dataSpan[0] == 1);
    CHECK(dataSpan[1] == 2);
    CHECK(dataSpan[2] == 3);
    CHECK(dataSpan[3] == 4);
    CHECK(dataSpan[4] == 5);
    CHECK(dataSpan[5] == 6);

    CHECK(dataSpan(0, 0) == 1);
    CHECK(dataSpan(0, 1) == 2);
    CHECK(dataSpan(0, 2) == 3);
    CHECK(dataSpan(1, 0) == 4);
    CHECK(dataSpan(1, 1) == 5);
    CHECK(dataSpan(1, 2) == 6);
}

TEST_CASE("ValueSpanTest.iterateValues")
{
    std::vector<int> data      = {1, 9, 3, 9};
    std::vector<int> valueData = {1, 3};
    RasterMetadata meta(2, 2, 9);

    auto dataSpan = make_raster_span(data, meta);

    CHECK(std::distance(begin(dataSpan), end(dataSpan)) == 4);
    CHECK(std::distance(value_begin(dataSpan), value_end(dataSpan)) == 2);

    CHECK(std::equal(dataSpan.begin(), dataSpan.end(), data.begin()));
}

TEST_CASE("ValueSpanTest.is_nodata")
{
    std::vector<int> data = {1, 9, 3, 9};
    RasterMetadata meta(2, 2, 9);

    auto dataSpan = make_raster_span(data, meta);
    CHECK_FALSE(dataSpan.is_nodata(0));
    CHECK(dataSpan.is_nodata(1));
    CHECK_FALSE(dataSpan.is_nodata(2));
    CHECK(dataSpan.is_nodata(3));

    CHECK_FALSE(dataSpan.is_nodata(0, 0));
    CHECK(dataSpan.is_nodata(0, 1));
    CHECK_FALSE(dataSpan.is_nodata(1, 0));
    CHECK(dataSpan.is_nodata(1, 1));
}
}
