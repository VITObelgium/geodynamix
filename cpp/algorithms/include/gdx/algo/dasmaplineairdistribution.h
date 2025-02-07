#pragma once

#include "gdx/algo/aggregatemultiresolution.h"
#include "gdx/algo/weighteddistribution.h"
#include "gdx/exception.h"

#include <map>

namespace gdx {

/*! Distributes amounts over zones, proportional to the weight_per_landuse times proxy.
 *  Returns a gdx raster with the result.
 *  Only floating point result rasters make sense.
 */

template <typename ResultType, template <typename> typename RasterType, typename IntType, typename WeightType, typename AmountType, typename ProxyType>
RasterType<ResultType> dasMapLineairDistribution(
    const RasterType<IntType>& landuse_map,
    const std::unordered_map<IntType, WeightType>& weight_per_landuse,
    const RasterType<IntType>& zones,
    const std::unordered_map<IntType, AmountType>& amounts, // zone amounts
    const RasterType<ProxyType>& proxy                      // proxy map : for example a
)
{
    auto weight_map = convertCategoriesToWeights<float>(landuse_map, weight_per_landuse);
    return weighted_distribution<ResultType>(zones, weight_map * proxy, amounts, true);
}
}
