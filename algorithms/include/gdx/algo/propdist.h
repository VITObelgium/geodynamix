#pragma once

#include "gdx/algo/rasterize.h"
#include "gdx/algo/rasterizelineantialiased.h"
#include "gdx/exception.h"

#include <vector>

namespace gdx {

/*! Performs rasterization of lines on a certain attribute.
 *  Returns a gdx raster with the result.
 */

template <typename ResultType>
ResultType proportional_distribution_aa(
    inf::gdal::Layer linesLayer,
    const inf::GeoMetadata& meta,
    const std::string& fieldName)
{
    return rasterize_lines_anti_aliased<ResultType>(linesLayer, meta, fieldName, true, false);
}

template <typename ResultType, template <typename> typename RasterType>
RasterType<ResultType> proportional_distribution(
    inf::gdal::VectorDataSet& shapes,
    const inf::GeoMetadata& meta,
    const std::string& fieldName)
{
    RasterizeOptions<ResultType> options;
    options.attribute   = fieldName;
    options.add         = true;
    options.meta        = meta;
    options.meta.nodata = 0;
    return rasterize<RasterType<ResultType>>(shapes, options);
}
}
