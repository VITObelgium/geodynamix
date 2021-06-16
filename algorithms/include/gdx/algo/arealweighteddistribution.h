#pragma once

#include "gdx/exception.h"
#include "weighteddistribution.h"

#include <map>

namespace gdx {

/*! Like weightedDistribution, but with weight == 1.
 *  Returns a gdx raster with the result.
 *  Only floating point result rasters make sense.
 */

template <typename ResultType, template <typename> typename RasterType, typename IntType, typename AmountType>
RasterType<ResultType> areal_weighted_distribution(
    const RasterType<IntType>& zones,
    const std::unordered_map<IntType, AmountType>& amounts)
{
    RasterType<float> weights(zones.metadata());
    weights.fill(1.0f);
    return weighted_distribution<ResultType>(zones, weights, amounts, true);
}
}
