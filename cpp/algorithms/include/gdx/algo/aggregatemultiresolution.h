#pragma once

#include "gdx/exception.h"
#include "multiresolution.h"

#include <map>

namespace gdx {

template <typename ResultType, template <typename> typename RasterType, typename IntType, typename FloatType>
RasterType<ResultType> convertCategoriesToWeights(
    const RasterType<IntType>& categories_map,
    const std::unordered_map<IntType, FloatType>& weight // the weights per landuse
)
{
    RasterType<ResultType> result(categories_map.metadata());
    if constexpr (!result.raster_type_has_nan) {
        throw InvalidArgument("convertCategoriesToWeights: makes only sense with floating point rasters");
    }
    result.set_nodata(result.NaN);
    result.fill(result.NaN);
    const int size = int(categories_map.size());
    for (int i = 0; i < size; ++i) {
        if (!categories_map.is_nodata(i)) {
            if (auto iter = weight.find(categories_map[i]); iter != weight.end()) {
                result[i] = static_cast<ResultType>(iter->second);
            }
        }
    }
    return result;
}

/*! Assignes weights per landuse.  Landuse is a higher resolution map.
 *  Returns a gdx raster with the normal result (weights are summed in the aggregation).
 *  Only floating point result rasters make sense.
 */

template <typename ResultType, template <typename> typename RasterType, typename IntType, typename FloatType>
RasterType<ResultType> aggregate_multi_resolution(
    const RasterType<IntType>& landuse_map,                     // higher resolution
    const std::unordered_map<IntType, FloatType>& weight_table, // the weight_tables_table per landuse_map
    const RasterMetadata& meta                                  // result aggregated extent
)
{
    if (landuse_map.metadata().rows < meta.rows || landuse_map.metadata().rows % meta.rows != 0) {
        throw InvalidArgument("aggregateMultiResolution: landuse_map map should have compatible size");
    }
    if (landuse_map.metadata().cols < meta.cols || landuse_map.metadata().cols % meta.cols != 0) {
        throw InvalidArgument("aggregateMultiResolution: landuse_map map should have compatible size");
    }
    const int factor = landuse_map.metadata().rows / meta.rows;
    if (factor != landuse_map.metadata().cols / meta.cols) {
        throw InvalidArgument("aggregateMultiResolution: landuse_map map should have compatible size");
    }

    auto weights_map = convertCategoriesToWeights<float>(landuse_map, weight_table);
    return deflate_equal_sum(weights_map, factor);
}
}
