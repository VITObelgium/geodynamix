#include "gdx/test/testbase.h"

#include "gdx/algo/cast.h"
#include "gdx/algo/normalise.h"

#include <numeric>
#include <random>

namespace gdx::test {

TEST_CASE_TEMPLATE("Normalize", TypeParam, RasterTypes)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

    if (!typeSupported<T>()) return;

    RasterMetadata meta(3, 3);

    SUBCASE("normalise")
    {
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                0.0, 32.0, 32.0,
                                64.0, 64.0, 64.0,
                                96.0, 96.0, 128.0}));

        ResultRasterType expected(meta, std::vector<uint8_t>{
                                            0, 64, 64,
                                            127, 127, 127,
                                            191, 191, 254});

        ResultRasterType result(meta);
        normalise(raster, result);
        CHECK_RASTER_EQ(expected, result);
    }
}
}