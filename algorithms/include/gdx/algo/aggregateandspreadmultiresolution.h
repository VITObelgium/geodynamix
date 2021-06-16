#pragma once

#include "gdx/algo/aggregatemultiresolution.h"
#include "gdx/algo/multiresolution.h"
#include "gdx/algo/weighteddistribution.h"
#include "gdx/exception.h"
#include "infra/span.h"

#include <vector>

namespace gdx {

/*! line weightedDistribution but 
 *  the landuse map and zones map may have a higher resolution.
 *  Returns result as gdx raster at the resolution of the target extent.
 *  Only floating point result rasters make sense.
 */

template <typename ResultType, template <typename> typename RasterType, typename IntType, typename WeightType, typename AmountType>
RasterType<ResultType> aggregate_and_spread_multi_resolution(
    const RasterType<IntType>& landuse_map,
    const std::unordered_map<IntType, WeightType>& weight_per_landuse,
    const RasterType<IntType>& zones,
    const std::unordered_map<IntType, AmountType>& amount_per_zone,
    const RasterMetadata& meta // target extent
)
{
    if (landuse_map.metadata().rows != zones.metadata().rows || landuse_map.metadata().cols != zones.metadata().cols) {
        throw InvalidArgument("aggregateAndSpreadMultiResolution: landuse map and zone map should have equal extent");
    }
    if (landuse_map.metadata().rows < meta.rows || landuse_map.metadata().rows % meta.rows != 0) {
        throw InvalidArgument("aggregateAndSpreadMultiResolution: landuse map and zone map should have size compatible with target extent");
    }
    if (landuse_map.metadata().cols < meta.cols || landuse_map.metadata().cols % meta.cols != 0) {
        throw InvalidArgument("aggregateAndSpreadMultiResolution: landuse map and zone map should have size compatible with target extent");
    }
    const int factor = landuse_map.metadata().rows / meta.rows;
    if (factor != landuse_map.metadata().cols / meta.cols) {
        throw InvalidArgument("aggregateAndSpreadMultiResolution: landuse map and zone map should have size compatible with target extent");
    }

    auto weights = convertCategoriesToWeights<float>(landuse_map, weight_per_landuse);
    auto result  = weighted_distribution<ResultType>(zones, weights, amount_per_zone, false);
    return deflate_equal_sum<ResultType>(result, factor);
}
}
