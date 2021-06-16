#include "gdx/test/testbase.h"

#include "gdx/algo/clip.h"
#include "infra/crs.h"

namespace gdx::test {

TEST_CASE_TEMPLATE("Clip raster", TypeParam, UnspecializedRasterTypes)
{
    RasterMetadata meta(4, 4, -1.0);
    meta.xll      = 0.0;
    meta.yll      = 0.0;
    meta.cellSize = 100.0;
    meta.set_projection_from_epsg(inf::crs::epsg::BelgianLambert72);

    SUBCASE("clip edges")
    {
        using FloatRaster = typename TypeParam::template type<float>;

        FloatRaster raster(meta, std::vector<float>{
                                     -1.f, 1.0f, 2.0f, 3.0f,
                                     4.f, 5.f, 6.0f, 7.0f,
                                     8.f, 9.f, -1.0f, 11.0f,
                                     12.f, 13.f, 14.f, -1.f});

        FloatRaster expected(meta, std::vector<float>{
                                       -1.f, -1.f, -1.f, -1.f,
                                       -1.f, 5.f, 6.0f, -1.f,
                                       -1.f, 9.f, -1.0f, -1.f,
                                       -1.f, -1.f, -1.f, -1.f});

        std::vector<inf::Coordinate> polygon = {{
            {100, 100},
            {100, 300},
            {300, 300},
            {300, 100},
        }};

        clip_raster(raster, polygon, inf::crs::epsg::BelgianLambert72);

        CHECK_RASTER_EQ(expected, raster);
    }
}
}
