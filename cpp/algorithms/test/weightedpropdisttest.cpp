#include "gdx/algo/weightedpropdist.h"
#include "gdx/test/testbase.h"
#include "testconfig.h"

namespace gdx::test {

TEST_CASE_TEMPLATE("WeightedPropDist", TypeParam, UnspecializedRasterTypes)
{
    using FloatRaster = typename TypeParam::template type<float>;
    using IntRaster   = typename TypeParam::template type<int32_t>;

    SUBCASE("weightedPropDistWeiss")
    {
        RasterMetadata meta(5, 5, 0.0, 0.0, 100.0, -9999.0);
        const IntRaster zones(meta, std::vector<int>{
                                        0, 0, 0, 0, 0,
                                        1, 1, 1, 1, 1,
                                        2, 2, 2, 2, 2,
                                        3, 3, 3, 3, 3,
                                        -9999, -9999, -9999, -9999, -9999});
        inf::gdal::VectorDataSet lines = inf::gdal::VectorDataSet::open(TEST_DATA_DIR "/weightedpropdist.shp", gdal::VectorType::ShapeFile);
        auto actualWeights             = gdx::rasterize_lines_anti_aliased<FloatRaster>(lines.layer(0), meta, "value", true, true);

        meta.nodata = 0.0;
        FloatRaster expectedWeights(meta, std::vector<float>{
                                              0.848388f, 0.850502f, 0.000000f, 0.000000f, 0.000000f,
                                              1.337222f, 0.515066f, 0.000000f, 0.000000f, 0.000000f,
                                              3.714551f, 1.318612f, 0.080368f, 0.000000f, 0.000000f,
                                              3.206186f, 3.125492f, 4.260901f, 3.286228f, 2.006215f,
                                              0.229909f, 0.000000f, 0.000000f, 1.055042f, 2.043183f});
        CHECK(actualWeights.metadata() == expectedWeights.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expectedWeights, actualWeights, 1e-5f);

        const std::unordered_map<int, double> amounts = {{0, 1}, {1, 0}, {2, 100.0f}, {3, 1000.0f}};
        auto actual                                   = gdx::weighted_proportional_distribution_aa<float>(zones, lines.layer(0), "value", amounts);

        meta.nodata = 0.0;
        FloatRaster expected(meta, std::vector<float>{
                                       0.499377965927124f, 0.500622034072876f, 0, 0, 0,
                                       0, 0, 0, 0, 0,
                                       72.6416015625f, 25.786727905273438f, 1.571666955947876f, 0, 0,
                                       201.83705139160156f, 196.7572021484375f, 268.23388671875f, 206.87586975097656f, 126.29603576660156f,
                                       0, 0, 0, 0, 0});
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
    }
}
}