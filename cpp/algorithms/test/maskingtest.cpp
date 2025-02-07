#include "gdx/test/testbase.h"

#include "gdx/algo/cast.h"
#include "gdx/algo/masking.h"

#include <numeric>
#include <random>

namespace gdx::test {

TEST_CASE_TEMPLATE("Masking", TypeParam, RasterTypes)
{
    using T              = typename TypeParam::value_type;
    using TMask          = int32_t;
    using Raster         = typename TypeParam::raster;
    using MaskRasterType = decltype(raster_cast<TMask>(std::declval<Raster>()));

    if (!typeSupported<T>()) return;

    RasterMetadata meta = RasterMetadata(3, 3, 255.0);

    SUBCASE("mask")
    {
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                0.0, 32.0, 32.0,
                                64.0, 255.0, 64.0,
                                96.0, 96.0, 255.0}));

        MaskRasterType mask(meta, convertTo<TMask>(std::vector<double>{
                                      255.0, 255.0, 1.0,
                                      1.0, 1.0, 2.0,
                                      255.0, 2.0, 3.0}));

        Raster expectedFilter1(meta, convertTo<T>(std::vector<double>{
                                         255.0, 255.0, 32.0,
                                         64, 255.0, 255.0,
                                         255.0, 255.0, 255.0}));

        auto result = gdx::mask(raster, mask, [](TMask value) {
            return value == TMask(1);
        });
        CHECK_RASTER_EQ(expectedFilter1, result);

        Raster expectedFilter2(meta, convertTo<T>(std::vector<double>{
                                         255.0, 255.0, 255.0,
                                         255.0, 255.0, 64.0,
                                         255.0, 96.0, 255.0}));

        result = gdx::mask(raster, mask, [](TMask value) {
            return value == TMask(2);
        });

        CHECK_RASTER_EQ(expectedFilter2, result);

        // Allow all the nodata mask values to pass
        Raster expectedFilter3(meta, convertTo<T>(std::vector<double>{
                                         0.0, 32.0, 255.0,
                                         255.0, 255.0, 255,
                                         96.0, 255, 255.0}));

        result = gdx::mask(raster, mask, [](TMask value) {
            return value == 255.0;
        });

        CHECK_RASTER_EQ(expectedFilter3, result);
    }

    SUBCASE("insideMask")
    {
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                0.0, 32.0, 32.0,
                                64.0, 255.0, 64.0,
                                96.0, 96.0, 255.0}));

        MaskRasterType mask(meta, convertTo<TMask>(std::vector<double>{
                                      255.0, 255.0, 1.0,
                                      1.0, 1.0, 2.0,
                                      255.0, 2.0, 3.0}));

        Raster expected(meta, convertTo<T>(std::vector<double>{
                                  255.0, 255.0, 32.0,
                                  64, 255.0, 64.0,
                                  255.0, 96.0, 255.0}));

        CHECK_RASTER_EQ(expected, gdx::inside_mask(raster, mask));
    }

    SUBCASE("outsideMask")
    {
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                0.0, 32.0, 32.0,
                                64.0, 255.0, 64.0,
                                96.0, 96.0, 255.0}));

        MaskRasterType mask(meta, convertTo<TMask>(std::vector<double>{
                                      255.0, 255.0, 1.0,
                                      1.0, 1.0, 2.0,
                                      255.0, 2.0, 3.0}));

        Raster expected(meta, convertTo<T>(std::vector<double>{
                                  0.0, 32.0, 255.0,
                                  255, 255.0, 255.0,
                                  96.0, 255.0, 255.0}));

        CHECK_RASTER_EQ(expected, gdx::outside_mask(raster, mask));
    }

    SUBCASE("sumMask")
    {
        Raster raster(meta, convertTo<T>(std::vector<double>{
                                0.0, 32.0, 32.0,
                                64.0, 255.0, 64.0,
                                96.0, 96.0, 255.0}));

        MaskRasterType mask(meta, convertTo<TMask>(std::vector<double>{
                                      255.0, 255.0, 1.0,
                                      1.0, 1.0, 2.0,
                                      255.0, 3.0, 2.0}));

        std::vector<TMask> values;
        values.push_back(1);

        std::unordered_map<TMask, double> expected = {
            {1, 96},
        };

        auto result = gdx::sum_mask<double>(raster, mask, std::span<const TMask>(values));
        CHECK_CONTAINER_EQ(expected, result);

        values.push_back(3);
        expected = {
            {1, 96},
            {3, 96},
        };

        result = gdx::sum_mask<double>(raster, mask, std::span<const TMask>(values));
        CHECK_CONTAINER_EQ(expected, result);

        values.push_back(2);
        expected = {
            {1, 96},
            {2, 64},
            {3, 96},
        };

        result = gdx::sum_mask<double>(raster, mask, std::span<const TMask>(values));
        CHECK_CONTAINER_EQ(expected, result);
    }
}
}