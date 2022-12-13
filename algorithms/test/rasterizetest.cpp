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
        "coordinates": [
          [
            [
              50,
              150
            ],
            [
              150,
              150
            ],
            [
              150,
              50.0
            ],
            [
              50,
              50
            ],
            [
              50,
              150
            ]
          ]
        ],
        "type": "Polygon"
      },
      "crs": {
        "type": "name",
        "properties": {
            "name": "urn:ogc:def:crs:EPSG:31370"
        }
    }
    }
  ]
}
        )json";

        DoubleRaster expected;

        RasterizePolygonOptions opts;
        opts.outputMeta = RasterMetadata(2, 2, 0.0, 0.0, 100.0, -9999.0);
        opts.outputMeta.set_projection_from_epsg(crs::epsg::BelgianLambert72);

        SUBCASE("prop1")
        {
            auto val = 200.0 / 4.0;

            opts.burnValue = "prop1";
            expected       = DoubleRaster(opts.outputMeta, std::vector<double>{
                                                         val, val,
                                                         val, val});
        }

        SUBCASE("prop2")
        {
            auto val = 9.0 / 4.0;

            opts.burnValue = "prop2";
            expected       = DoubleRaster(opts.outputMeta, std::vector<double>{
                                                         val, val,
                                                         val, val});
        }

        auto jsonPath = fs::u8path(geoJson);
        auto ds       = gdal::VectorDataSet::open(jsonPath, gdal::VectorType::GeoJson);
        ds.layer(0).set_projection_from_epsg(crs::epsg::BelgianLambert72);
        auto actual = gdx::rasterize_polygons<DoubleRaster>(ds, opts);

        CHECK(actual.metadata() == expected.metadata());
        CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
    }
}
}
