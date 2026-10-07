#pragma once

#include "gdx/algo/nodata.h"
#include "gdx/algo/rasterize.h"
#include "gdx/exception.h"
#include "gdx/rastermetadata.h"

#include "gdx/point.h"
#include "infra/cast.h"
#include "infra/gdalspatialreference.h"
#include "infra/log.h"
#include "infra/span.h"

#include <optional>
#include <utility>

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

    std::optional<inf::gdal::CoordinateTransformer> transformer;
    auto sourceProjection = pointsLayer.projection();
    if (!sourceProjection) {
        inf::Log::warn("add_points: input point layer has no projection information; assuming source and output projections are the same");
    } else if (meta.projection.empty()) {
        inf::Log::warn("add_points: output raster has no projection information; assuming source and output projections are the same");
    }

    if (sourceProjection && !meta.projection.empty()) {
        inf::gdal::SpatialReference targetProjection(meta.projection);
        if (!sourceProjection->is_same(targetProjection)) {
            transformer.emplace(std::move(*sourceProjection), std::move(targetProjection));
        }
    }

    auto fieldIndex = pointsLayer.layer_definition().required_field_index(fieldName);
    RasterType<ResultType> result(resultMeta, inf::truncate<ResultType>(resultMeta.nodata.value()));

    for (const auto& feature : pointsLayer) {
        if (feature.has_geometry()) {
            auto geom = feature.geometry();
            if (geom.type() == inf::gdal::Geometry::Type::Point) {
                auto point = geom.as<inf::gdal::PointCRef>().point();
                if (transformer) {
                    point = transformer->transform(point);
                }
                auto cell = resultMeta.convert_point_to_cell(point);
                if (meta.is_on_map(cell)) {
                    result.add_to_cell(cell, feature.field_as<ResultType>(fieldIndex));
                }
            }
        }
    }

    return result;
}
}
