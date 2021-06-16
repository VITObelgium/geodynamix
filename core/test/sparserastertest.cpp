#include "gdx/test/testbase.h"

#include "gdx/sparseraster.h"

#include <type_traits>

namespace gdx::test {

TEST_CASE("sparse raster")
{
    SparseRaster<int> raster(RasterMetadata(3, 2, -99), std::vector<int>{-99, 0, 1, -99, 5, -99});

    SUBCASE("nodataChecks")
    {
        CHECK(raster.is_nodata(0, 0));
        CHECK_FALSE(raster.is_nodata(0, 1));
        CHECK_FALSE(raster.is_nodata(1, 0));
        CHECK(raster.is_nodata(1, 1));
        CHECK_FALSE(raster.is_nodata(2, 0));
        CHECK(raster.is_nodata(2, 1));

        CHECK(raster.is_nodata(0));
        CHECK_FALSE(raster.is_nodata(1));
        CHECK_FALSE(raster.is_nodata(2));
        CHECK(raster.is_nodata(3));
        CHECK_FALSE(raster.is_nodata(4));
        CHECK(raster.is_nodata(5));
    }

    SUBCASE("element access")
    {
        CHECK(raster(0, 1) == 0);
        CHECK(raster(1, 0) == 1);
        CHECK(raster(2, 0) == 5);

        raster(2, 0) = 6;
        CHECK(raster(2, 0) == 6);
    }

    SUBCASE("compare")
    {
        SparseRaster<int> copy = raster.copy();
        CHECK(raster == copy);

        copy(2, 0) = 6;
        CHECK(raster != copy);
    }

    SUBCASE("multiply")
    {
        SparseRaster<int> expected(RasterMetadata(3, 2, -99), std::vector<int>{-99, 0, 2, -99, 10, -99});
        CHECK(expected == raster * 2);

        raster *= 2;
        CHECK(expected == raster);
    }

    SUBCASE("add")
    {
        SparseRaster<int> expected(RasterMetadata(3, 2, -99), std::vector<int>{-99, 1, 2, -99, 6, -99});
        CHECK(expected == raster + 1);

        raster += 1;
        CHECK(expected == raster);
    }

    SUBCASE("subtract")
    {
        SparseRaster<int> expected(RasterMetadata(3, 2, -99), std::vector<int>{-99, -1, 0, -99, 4, -99});
        CHECK(expected == raster - 1);

        raster -= 1;
        CHECK(expected == raster);
    }

    SUBCASE("negate")
    {
        SparseRaster<int> expected(RasterMetadata(3, 2, -99), std::vector<int>{-99, 0, -1, -99, -5, -99});
        CHECK(expected == -raster);
    }
}
}
