#include "gdx/test/testbase.h"

#include "gdx/algo/rasterize.h"
#include "infra/crs.h"

namespace gdx::test {

using namespace inf;

TEST_CASE_TEMPLATE("RasterizePolygons", TypeParam, UnspecializedRasterTypes)
{
    using DoubleRaster = typename TypeParam::template type<double>;

    SUBCASE("rasterize_polygon")
    {
        // Json with a few polygons on the 2x2 grid
        // - Polygon 1: in the center covering every pixel with 1/4 of its surface
        // - Polygon 2: completely inside the bottom left pixel
        // - Polygon 3: 1/4 in the top right pixel, the rest outside of the grid

        const char* geoJson = R"json(
        {
          "type": "FeatureCollection",
          "features": [
            {
              "type": "Feature",
              "properties": {
                "prop1": 200.0,
                "prop2": 9.0
              },
              "geometry": {
                "coordinates": [ [ [50, 150], [150, 150], [150, 50.0], [50, 50], [50, 150] ] ],
                "type": "Polygon"
              }
            },
            {
              "type": "Feature",
              "properties": {
                "prop1": 10.0,
                "prop2": 20.0
              },
              "geometry": {
                "coordinates": [ [ [50, 50], [60, 50], [60, 40.0], [50, 40], [50, 50] ] ],
                "type": "Polygon"
              }
            },
            {
              "type": "Feature",
              "properties": {
                "prop1": 100.0,
                "prop2": 200.0
              },
              "geometry": {
                "coordinates": [ [ [150, 250], [250, 250], [250, 150.0], [150, 150], [150, 250] ] ],
                "type": "Polygon"
              }
            }
          ],
          "crs": {
            "type": "name",
            "properties": {
              "name": "urn:ogc:def:crs:EPSG:31370"
            }
          }
        }
        )json";

        RasterizePolygonOptions opts;
        opts.outputMeta = RasterMetadata(2, 2, 0.0, 0.0, 100.0, -9999.0);
        opts.outputMeta.set_projection_from_epsg(crs::epsg::BelgianLambert72);

        double polygon1Val = 0, polygon2Val = 0, polygon3Val = 0;

        SUBCASE("prop1")
        {
            polygon1Val = 200.0 / 4.0;
            polygon2Val = 10.0;
            polygon3Val = 100.0 / 4.0;

            opts.burnValue = "prop1";
        }

        SUBCASE("prop2")
        {
            polygon1Val = 9.0 / 4.0;
            polygon2Val = 20.0;
            polygon3Val = 200.0 / 4.0;

            opts.burnValue = "prop2";
        }

        DoubleRaster expected(opts.outputMeta, std::vector<double>{
                                                   polygon1Val, polygon1Val + polygon3Val,
                                                   polygon1Val + polygon2Val, polygon1Val});

        auto jsonPath = fs::u8path(geoJson);
        auto ds       = gdal::VectorDataSet::open(jsonPath, gdal::VectorType::GeoJson);
        auto actual   = gdx::rasterize_polygons<DoubleRaster>(ds, opts);

        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
    }
}
}
