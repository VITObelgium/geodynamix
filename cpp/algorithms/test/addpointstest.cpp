#include "gdx/algo/addpoints.h"
#include "gdx/test/testbase.h"

#include "testconfig.h"

namespace gdx::test {

using namespace inf;

TEST_CASE("AddPoints.addPointsWeiss")
{
    RasterMetadata meta(5, 5, 0, 0, 100, 0);
    inf::gdal::VectorDataSet shapes = inf::gdal::VectorDataSet::open(file::u8path(TEST_DATA_DIR) / "addpoints.shp", gdal::VectorType::ShapeFile);
    auto actual                     = gdx::add_points<float, DenseRaster>(shapes.layer(0), "value", meta);

    DenseRaster<float> expected(meta, std::vector<float>{
                                          0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0,
                                          100.0f, 0, 0, 0, 0,
                                          10.0f, 1001.0f, 0, 0, 0});

    CHECK_RASTER_EQ(expected, actual);
}
}
