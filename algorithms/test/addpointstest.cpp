#include "gdx/algo/addpoints.h"
#include "gdx/test/testbase.h"

#include "infra/crs.h"
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

TEST_CASE("AddPoints.reprojectsPoints")
{
    const Point<double> wgs84Point(4.5, 50.9);
    const auto projectedPoint = gdal::convert_point_projected(crs::epsg::WGS84, crs::epsg::BelgianLambert72, wgs84Point);

    RasterMetadata meta(3, 3, projectedPoint.x - 150.0, projectedPoint.y - 150.0, 100.0, 0.0);
    meta.set_projection_from_epsg(crs::epsg::BelgianLambert72);

    auto driver = gdal::VectorDriver::create(gdal::VectorType::Memory);
    auto ds     = driver.create_dataset("points");
    gdal::SpatialReference sourceProjection(crs::epsg::WGS84);
    auto layer = ds.create_layer("points", sourceProjection, gdal::Geometry::Type::Point);
    auto field = gdal::FieldDefinition::create<double>("value");
    layer.create_field(field);

    gdal::Feature feature(layer.layer_definition());
    OGRPoint point(wgs84Point.x, wgs84Point.y);
    feature.set_geometry(gdal::PointCRef(&point));
    feature.set_field("value", 12.0);
    layer.create_feature(feature);

    auto actual = gdx::add_points<float, DenseRaster>(layer, "value", meta);
    DenseRaster<float> expected(meta, std::vector<float>{
                                          0, 0, 0,
                                          0, 12.0f, 0,
                                          0, 0, 0});
    CHECK_RASTER_EQ(expected, actual);
}
}
