#pragma once

#include "gdx/algo/rasterize.h"
#include "gdx/algo/rasterizelineantialiased.h"
#include "gdx/algo/shape.h"
#include "gdx/algo/weighteddistribution.h"
#include "gdx/exception.h"

#include <map>

namespace gdx {

/*! Like weightedDistribution, but as the weight per cell the number of shapes that go over it.
 *  Also like arealWeightedDistribution, but as the weight per cell the number of shapes that go over it.
 *  Returns a gdx raster with the result.
 *  Only floating point result rasters make sense.
 */

template <typename ResultType, template <typename> typename RasterType, typename IntType, typename AmountType>
RasterType<ResultType> linear_distribution_aa(
    const RasterType<IntType>& zones,
    inf::gdal::Layer linesLayer,
    const std::unordered_map<IntType, AmountType>& amounts)
{
    auto weights = rasterize_lines_anti_aliased<RasterType<float>>(linesLayer, zones.metadata(), "", false, false); // avoid RasterType<double>
    return weighted_distribution<ResultType>(zones, weights, amounts, true);
}

template <typename ResultType, template <typename> typename RasterType, typename IntType, typename AmountType>
RasterType<ResultType> linear_distribution(
    const RasterType<IntType>& zones,
    const inf::gdal::VectorDataSet& shapes,
    const std::unordered_map<IntType, AmountType>& amounts)
{
    RasterizeOptions<ResultType> options;
    options.burnValue = 1;
    options.add       = true;
    options.meta      = zones.metadata();
    auto weights      = rasterize<RasterType<float>>(shapes, options); // avoid RasterType<double>
    return weighted_distribution<ResultType>(zones, weights, amounts, true);
}
}
