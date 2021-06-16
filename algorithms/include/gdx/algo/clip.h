#pragma once

#include "gdx/algo/algorithm.h"
#include "gdx/exception.h"
#include "infra/conversion.h"
#include "infra/coordinate.h"
#include "infra/gdal.h"
#include "infra/span.h"

namespace gdx {

template <typename RasterType>
void clip_raster(RasterType& ras, inf::gdal::Polygon& polygon)
{
    const auto& meta = ras.metadata();

    auto transformedPolygon = polygon.clone();
    inf::gdal::SpatialReference dstCrs(ras.metadata().projection);
    transformedPolygon.transform_to(dstCrs);

    gdx::for_each_data_value(ras, [&](auto& value) {
        const auto point = meta.convert_cell_centre_to_xy(value.cell());
        if (!transformedPolygon.intersects(point)) {
            value.reset();
        }
    });
}

template <typename RasterType>
void clip_raster(RasterType& ras, std::span<const inf::Coordinate> polygon, int32_t polygonCrs)
{
    OGRLinearRing linearRing;

    for (auto coord : polygon) {
        auto point = inf::to_point(coord);
        linearRing.addPoint(point.x, point.y);
    }

    linearRing.closeRings();

    inf::gdal::SpatialReference srcCrs(polygonCrs);
    OGRPolygon geom;
    geom.addRing(&linearRing);
    geom.assignSpatialReference(srcCrs.get());

    inf::gdal::Polygon poly(&geom);

    return clip_raster<RasterType>(ras, poly);
}

}
