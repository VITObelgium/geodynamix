#include "gdx/test/testbase.h"

#include "gdx/algo/rasterizelineantialiased.h"

namespace gdx::test {

using namespace inf;

TEST_CASE_TEMPLATE("RasterizeLineAntiAliased", TypeParam, UnspecializedRasterTypes)
{
    using DoubleRaster = typename TypeParam::template type<double>;

    SUBCASE("rasterizeSegmentAntiAliasedTest")
    {
        RasterMetadata meta(5, 5, 0.0, 0.0, 100.0, -9999.0);
        DoubleRaster actual(meta, 0);
        double xStart = 50.0, yStart = 50.0, xEnd = 350.0, yEnd = 150.0;
        std::vector<std::pair<Cell, float /*brightness*/>> locs;
        details::rasterize_segment_anti_aliased(xStart, yStart, xEnd, yEnd, meta, locs);
        for (auto& loc : locs) {
            int ry = loc.first.r, cx = loc.first.c;
            actual(ry, cx) += loc.second;
        }

        DoubleRaster expected(meta, std::vector<double>{
                                        0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0,
                                        0, 1.0 / 3.0, 2.0 / 3.0, 0.5, 0,
                                        0.5, 2.0 / 3.0, 1.0 / 3.0, 0, 0});
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
    }

    SUBCASE("rasterizeSegmentsAntiAliasedTest")
    {
        RasterMetadata meta(5, 5, 0.0, 0.0, 100.0, -9999.0);
        std::vector<std::vector<float>> raster;
        raster.resize(meta.rows);
        for (int ry = 0; ry < meta.rows; ++ry) {
            raster[ry].assign(meta.cols, 0.0f);
        }
        std::vector<std::vector<Point<float>>> endPoints = {{{50.0f, 50.0f}, {350.0f, 150.0f}, {350.0f, 350.0f}}};
        float value                                      = 5.0f;
        details::rasterize_segments_anti_aliased(endPoints, value, false, meta, raster);
        endPoints = {{{50.0f, 350.0f}, {200.0f, 350.0f}}};
        value     = 7.0f;
        details::rasterize_segments_anti_aliased(endPoints, value, false, meta, raster);
        DoubleRaster actual(meta);
        for (int ry = 0; ry < meta.rows; ++ry) {
            for (int cx = 0; cx < meta.cols; ++cx) {
                actual(ry, cx) = raster[ry][cx];
            }
        }

        DoubleRaster expected(meta, std::vector<double>{
                                        0, 0, 0, 0, 0,
                                        7 * 0.5, 7.0 * 1.0, 0, 5.0 * 0.5, 0,
                                        0, 0, 0, 5.0 * 1.0, 0,
                                        0, 5.0 * 1.0 / 3.0, 5.0 * 2.0 / 3.0, 5.0 * 1.0, 0,
                                        5 * 0.5, 5.0 * 2.0 / 3.0, 5.0 * 1.0 / 3.0, 0, 0});
        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
    }
}
}