#pragma once

#include "gdx/algo/rasterize.h"
#include "gdx/algo/rasterizelineantialiased.h"
#include "gdx/algo/weighteddistribution.h"
#include "gdx/exception.h"

#include <map>

namespace gdx {

/*! Like weightedDistribution, but as the weight per cell the incremental rasterized value of lines that go over it.
 *  Returns a gdx raster with the result.
 *  Only floating point result rasters make sense.
 */

template <typename ResultType, template <typename> typename RasterType, typename IntType, typename AmountType>
RasterType<ResultType> weighted_proportional_distribution_aa(
    const RasterType<IntType>& zones,
    inf::gdal::Layer linesLayer,
    const std::string& fieldName,
    const std::unordered_map<IntType, AmountType>& amounts)
{
    // only works for line shapes
    auto weights = rasterize_lines_anti_aliased<RasterType<float>>(linesLayer, zones.metadata(), fieldName, true, true); // avoid RasterType<double>
    return weighted_distribution<ResultType>(zones, weights, amounts, true);
}

template <typename ResultType, template <typename> typename RasterType, typename IntType, typename AmountType>
RasterType<ResultType> weighted_proportional_distribution(
    const RasterType<IntType>& zones,
    const inf::gdal::VectorDataSet& shapes,
    const std::string& fieldName,
    const std::unordered_map<IntType, AmountType>& amounts)
{
    // use this one for non-line shapes
    RasterizeOptions<float> options;
    options.values.push_back("-a");
    options.values.push_back(fieldName);
    options.values.push_back("-add");
    options.meta = zones.metadata();
    auto weights = rasterize<RasterType<float>>(shapes, options); // avoid RasterType<double>
    return weighted_distribution<ResultType>(zones, weights, amounts, true);
}
}
