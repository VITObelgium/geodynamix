#pragma once

#include "dasmap.h"
#include "gdx/exception.h"
#include "multiresolution.h"

#include <vector>

namespace gdx {

/*! Multi Resolution Dasymetric mapping.  Landuse map and zone map may be at finer resolution.
 *  Returns a gdx raster with the result.
 *  Only floating point result rasters make sense.
 */

template <typename ResultType, template <typename> typename RasterType, typename IntType, typename FloatType>
RasterType<ResultType> dasymetric_mapping_multiresolution(
    const RasterType<IntType>& landuse_map, // landuses are identified in the code with variable 'k'
    const std::vector<FloatType>& Ck,       // [k] : landuse weights.  Set all to 1 to have zero effect.
    const RasterType<IntType>& zone_map,    // zones are identified in the code with variable 'z'
    const std::vector<FloatType>& Gz,       // [z] : the amount that has to be mapped per zone
    const RasterMetadata& meta              // target extent
)
{
    if (landuse_map.metadata().rows != zone_map.metadata().rows || landuse_map.metadata().cols != zone_map.metadata().cols) {
        throw InvalidArgument("landuse map and zone map should have equal size in multiresolution dasymatrix mapping");
    }
    if (landuse_map.metadata().rows < meta.rows || landuse_map.metadata().rows % meta.rows != 0) {
        throw InvalidArgument("inputs map should be compatible with target extent in multiresolution dasymatrix mapping");
    }
    if (landuse_map.metadata().cols < meta.cols || landuse_map.metadata().cols % meta.cols != 0) {
        throw InvalidArgument("inputs map should be compatible with target extent in multiresolution dasymatrix mapping");
    }
    const int factor = landuse_map.metadata().rows / meta.rows;
    if (factor != landuse_map.metadata().cols / meta.cols) {
        throw InvalidArgument("inputs map should be compatible with target extent in multiresolution dasymatrix mapping");
    }

    RasterType<ResultType>&& result = dasMap<ResultType, RasterType, IntType, FloatType>(landuse_map, Ck, zone_map, Gz);
    return deflate_equal_sum<ResultType, RasterType>(result, factor);
}
}
