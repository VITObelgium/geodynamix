#include "gdx/algo/sum.h"
#include "gdx/test/testbase.h"

#include <numeric>
#include <random>

namespace gdx::test {

static_assert(!has_sum_member_v<MaskedRaster<float>>, "Invalid sum member detected in MaskedRaster");
#ifdef GDX_ENABLE_SIMD
static_assert(has_sum_member_v<DenseRaster<float>>, "No sum member detected in MaskedRaster");
#endif

TEST_CASE_TEMPLATE("Sum", TypeParam, RasterTypes)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    double nod = 100.0;
    RasterMetadata meta(3, 3, nod);

    SUBCASE("sum")
    {
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                1.0, 2.0, 3.0,
                                4.0, 4.0, 4.0,
                                5.0, 6.0, 7.0}));

        CHECK(sum(raster) == 36.0);
    }

    SUBCASE("sumNoData")
    {
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                nod, 2.0, 3.0,
                                4.0, nod, 4.0,
                                5.0, 6.0, nod}));

        CHECK(sum(raster) == 24.0);
    }

    SUBCASE("sumValueOnlyNans")
    {
        Raster raster(meta, static_cast<T>(nod));
        CHECK(sum(raster) == 0.0);
    }
}
}
