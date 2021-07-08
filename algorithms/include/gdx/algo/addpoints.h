#pragma once

#include "gdx/algo/nodata.h"
#include "gdx/algo/rasterize.h"
#include "gdx/exception.h"
#include "gdx/rastermetadata.h"

#include "gdx/point.h"
#include "infra/cast.h"
#include "infra/span.h"

namespace gdx {

/*! Add values to the raster.  Can be used with point, lines of polygon shapes.
 *  Returns a gdx raster with the result.
 */

template <typename ResultType, template <typename> typename RasterType>
RasterType<ResultType> add_points(
    inf::gdal::Layer pointsLayer,
    const std::string& fieldName,
    const RasterMetadata& meta)
{
    auto resultMeta = meta;
    if (!resultMeta.nodata.has_value()) {
        if constexpr (std::is_floating_point_v<ResultType>) {
            resultMeta.nodata = std::numeric_limits<ResultType>::quiet_NaN();
        } else {
            resultMeta.nodata = std::numeric_limits<ResultType>::max();
        }
    }

    auto fieldIndex = pointsLayer.layer_definition().required_field_index(fieldName);
    RasterType<ResultType> result(resultMeta, inf::truncate<ResultType>(resultMeta.nodata.value()));

    for (const auto& feature : pointsLayer) {
        if (feature.has_geometry()) {
            auto geom = feature.geometry();
            if (geom.type() == inf::gdal::Geometry::Type::Point) {
                auto cell = resultMeta.convert_point_to_cell(geom.as<inf::gdal::PointCRef>().point());
                if (meta.is_on_map(cell)) {
                    result.add_to_cell(cell, feature.field_as<ResultType>(fieldIndex));
                }
            }
        }
    }

    return result;
}
}
