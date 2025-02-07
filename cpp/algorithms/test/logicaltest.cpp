#include "gdx/algo/logical.h"
#include "gdx/test/testbase.h"

#include <numeric>
#include <random>

namespace gdx::test {

TEST_CASE_TEMPLATE("Logical operations", TypeParam, RasterTypes)
{
    using Raster = typename TypeParam::raster;

    double nodata      = 100.0;
    double nodataOrNan = nodata;
    if constexpr (Raster::raster_type_has_nan) {
        nodataOrNan = std::numeric_limits<double>::quiet_NaN();
    }

    RasterMetadata meta(3, 3, nodataOrNan);

    SUBCASE("all")
    {
        using T      = typename TypeParam::value_type;
        using Raster = typename TypeParam::raster;
        if (!typeSupported<T>()) return;

        CHECK_FALSE(all_of(Raster(2, 2, convertTo<T>(std::vector<double>{1.0, 1.0, 1.0, 0.0}))));
        CHECK(all_of(Raster(2, 2, convertTo<T>(std::vector<double>{1.0, 1.0, 1.0, 1.0}))));

        RasterMetadata meta(2, 2);
        meta.nodata = 100;

        CHECK(all_of(Raster(meta, convertTo<T>(std::vector<double>{
                                      100.0, 100.0,
                                      100.0, 100.0}))));
    }

    SUBCASE("any")
    {
        using T      = typename TypeParam::value_type;
        using Raster = typename TypeParam::raster;
        if (!typeSupported<T>()) return;

        CHECK_FALSE(any_of(Raster(2, 2, convertTo<T>(std::vector<double>{0.0, 0.0, 0.0, 0.0}))));
        CHECK(any_of(Raster(2, 2, convertTo<T>(std::vector<double>{0.0, 0.0, 0.0, 1.0}))));
        CHECK(any_of(Raster(2, 2, convertTo<T>(std::vector<double>{1.0, 1.0, 1.0, 1.0}))));

        RasterMetadata meta(2, 2);
        meta.nodata = 100;

        CHECK_FALSE(any_of(Raster(meta, convertTo<T>(std::vector<double>{
                                            100.0, 0.0,
                                            100.0, 0.0}))));
    }
}
}