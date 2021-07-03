#include "gdx/algo/voronoi.h"
#include "gdx/test/testbase.h"

namespace gdx::test {

using namespace inf;

TEST_CASE_TEMPLATE("Voronoi", TypeParam, RasterTypes)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;

    SUBCASE("voronoi")
    {
        // 10 by 10 raster with cell size = 1 and lower left corner at(0, 0)
        gdx::RasterMetadata meta(10, 10);
        meta.set_cell_size(1.0);
        meta.xll = 0;
        meta.yll = 0;

        std::vector<gdx::Cell> locations =
            {
                gdx::Cell(2, 7),
                gdx::Cell(7, 6),
                gdx::Cell(5, 2),
            };

        auto actual = voronoi<Raster>(meta, locations);

        auto expected = Raster(meta, convertTo<T>(std::vector<double>({2, 2, 2, 0, 0, 0, 0, 0, 0, 0,
                                         2, 2, 2, 0, 0, 0, 0, 0, 0, 0,
                                         2, 2, 2, 2, 0, 0, 0, 0, 0, 0,
                                         2, 2, 2, 2, 2, 0, 0, 0, 0, 0,
                                         2, 2, 2, 2, 2, 0, 0, 0, 0, 0,
                                         2, 2, 2, 2, 2, 1, 1, 1, 1, 0,
                                         2, 2, 2, 2, 1, 1, 1, 1, 1, 1,
                                         2, 2, 2, 2, 1, 1, 1, 1, 1, 1,
                                         2, 2, 2, 1, 1, 1, 1, 1, 1, 1,
                                         2, 2, 2, 1, 1, 1, 1, 1, 1, 1})));

        CHECK_RASTER_EQ(expected, actual);
    }
}
}
