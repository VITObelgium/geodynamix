#include "gdx/rasterarea.h"

#include "gdx/algo/filter.h"
#include "gdx/algo/sum.h"
#include "gdx/test/testbase.h"

#include <random>

namespace gdx::test {

TEST_CASE_TEMPLATE("filter", TypeParam, RasterFloatTypes)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;

    SUBCASE("filterConstant")
    {
        // clang-format off
        const Raster ras(RasterMetadata(2, 3, -1.0), convertTo<T>(std::vector<double>({
            1, 1, 1,
            1, 1, 2,
        })));

        const Raster expected(ras.metadata(), convertTo<T>(std::vector<double>({
            1.0/3.0 + 1.0/4.0 + 1.0/3.0, 1.0/3.0 + 1.0/4.0 + 1.0/3.0 + 1.0 / 4.0, 1.0 / 4.0 + 1.0 / 3.0 + 2.0 / 3.0,
            1.0/3.0 + 1.0/4.0 + 1.0/3.0, 1.0/3.0 + 1.0/4.0 + 2.0/3.0 + 1.0 / 4.0, 1.0 / 4.0 + 2.0 / 3.0 + 1.0 / 3.0,
        })));
        // clang-format on

        auto actual = filter<Raster>(ras, FilterMode::Constant, 1);
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 10e-5);
        CHECK(sum(actual) == Approx(sum(ras)).epsilon(10e-5));
    }

    SUBCASE("averageFilterSquare")
    {
        // clang-format off
        const Raster ras(RasterMetadata(3, 4, -1.0), convertTo<T>(std::vector<double>({
            1,  2, -1,  4,
            5,  6,  7,  8,
            -1, 10, 11, -1,
        })));

        const Raster expected(ras.metadata(), convertTo<T>(std::vector<double>({
            14/4.0, 21/5.0, 27/5.0, 19/3.0,
            24/5.0, 42/7.0, 48/7.0, 30/4.0,
            21/3.0, 39/5.0, 42/5.0, 26/3.0,
        })));
        // clang-format on

        auto actual = average_filter_square<Raster>(ras, 1);
        CHECK_RASTER_NEAR(expected, actual);
    }
}
}
