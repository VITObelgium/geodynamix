#include "gdx/algo/category.h"
#include "gdx/test/testbase.h"
#include "testconfig.h"

namespace gdx::test {

TEST_CASE_TEMPLATE("Category", TypeParam, UnspecializedRasterTypes)
{
    using FloatRaster = typename TypeParam::template type<float>;
    using IntRaster   = typename TypeParam::template type<int32_t>;

    RasterMetadata meta(1, 5, 0.0, 0.0, 100.0, -9999.0);

    SUBCASE("categorySum")
    {
        const IntRaster clusters(meta, std::vector<int32_t>{1, 1, 1, 0, 2});
        const FloatRaster values(meta, std::vector<float>{1.1f, 1.2f, 1.3f, 2.1f, 3.1f});
        FloatRaster actual = gdx::category_sum(clusters, values);

        FloatRaster expected(meta, std::vector<float>{3.6f, 3.6f, 3.6f, 0.0f, 3.1f});
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5);
    }

    SUBCASE("categorySumNodataIs0")
    {
        meta.nodata = 0.0;

        const IntRaster clusters(meta, std::vector<int32_t>{1, 0, 1, 0, 2});
        const FloatRaster values(meta, 1);
        FloatRaster actual = gdx::category_sum(clusters, values);

        FloatRaster expected(meta, std::vector<float>{2.f, 0.f, 2.f, 0.f, 1.f});
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5);
    }

    SUBCASE("categoryMax")
    {
        const IntRaster clusters(meta, std::vector<int32_t>{1, 1, 1, 0, 2});
        const FloatRaster values(meta, std::vector<float>{1.1f, 1.2f, 1.3f, 2.1f, 3.1f});
        FloatRaster actual = gdx::category_max(clusters, values);

        FloatRaster expected(meta, std::vector<float>{1.3f, 1.3f, 1.3f, 0.0f, 3.1f});
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5);
    }

    SUBCASE("categoryMin")
    {
        const IntRaster clusters(meta, std::vector<int32_t>{1, 1, 1, 0, 2});
        const FloatRaster values(meta, std::vector<float>{1.1f, 1.2f, 1.3f, 2.1f, 3.1f});
        FloatRaster actual = gdx::category_min(clusters, values);

        FloatRaster expected(meta, std::vector<float>{1.1f, 1.1f, 1.1f, 0.0f, 3.1f});
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5);
    }

    SUBCASE("categoryMode")
    {
        const IntRaster clusters(meta, std::vector<int32_t>{1, 1, 1, 1, 2});
        const FloatRaster values(meta, std::vector<float>{1.1f, 1.2f, 1.3f, 1.2f, 3.1f});
        FloatRaster actual = gdx::category_mode(clusters, values);

        FloatRaster expected(meta, std::vector<float>{1.2f, 1.2f, 1.2f, 1.2f, 3.1f});
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5);
    }

    SUBCASE("filterAnd")
    {
        const IntRaster clusters(meta, std::vector<int32_t>{1, 1, 1, 0, 2});
        const FloatRaster values(meta, std::vector<float>{1.1f, 1.2f, 0.0f, 1.2f, 3.1f});
        IntRaster actual = gdx::category_filter_and(clusters, values);

        IntRaster expected(meta, std::vector<int32_t>{0, 0, 0, 0, 2});
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_EQ(expected, actual);
    }

    SUBCASE("filterOr")
    {
        const IntRaster clusters(meta, std::vector<int32_t>{1, 1, 1, 0, 2});
        const FloatRaster values(meta, std::vector<float>{1.1f, 1.2f, 0.0f, 1.2f, 0.0f});
        IntRaster actual = gdx::category_filter_or(clusters, values);

        IntRaster expected(meta, std::vector<int32_t>{1, 1, 1, 0, 0});
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_EQ(expected, actual);
    }

    SUBCASE("filterNot")
    {
        const IntRaster clusters(meta, std::vector<int32_t>{1, 1, 1, 0, 2});
        const FloatRaster values(meta, std::vector<float>{1.1f, 1.2f, 0.0f, 1.2f, 0.0f});
        IntRaster actual = gdx::category_filter_not(clusters, values);

        IntRaster expected(meta, std::vector<int32_t>{0, 0, 0, 0, 2});
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_EQ(expected, actual);
    }

    SUBCASE("categorySumInBuffer")
    {
        const IntRaster clusters(meta, std::vector<int32_t>{1, 1, 0, 0, 2});
        const FloatRaster values(meta, std::vector<float>{1.1f, 1.2f, 1.3f, 2.1f, 2.2f});
        FloatRaster actual = gdx::category_sum_in_buffer(clusters, values, 100.0f);

        FloatRaster expected(meta, std::vector<float>{3.6f, 3.6f, 0, 0, 4.3f});
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5);
    }
}
}
