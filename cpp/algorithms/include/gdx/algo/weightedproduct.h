#pragma once

#include "gdx/exception.h"
#include "maximum.h"

#include <limits>
#include <vector>

namespace gdx {

/*! Computes product of maps.  Each map is raised to 'weight' power first.
 *  Returns a gdx raster with the result.
 *  Only floating point result rasters make sense.
 */

template <typename ResultType, template <typename> typename RasterType, typename FloatType, typename WeightType>
RasterType<ResultType> weighted_product(
    const std::vector<std::pair<const RasterType<FloatType>*, WeightType>>& rasters)
{
    // some checks on the input and preprocessing
    if (rasters.size() == 0) {
        throw InvalidArgument("weighted product: at least one raster should be supplied"); // don't know size of result
    }
    for (int i = 1; i < int(rasters.size()); ++i) {
        if (rasters[0].first->size() != rasters[i].first->size()) {
            throw InvalidArgument("weighted product: size of rasters should all be equal");
        }
    }

    auto meta   = rasters[0].first->metadata();
    meta.nodata = std::numeric_limits<ResultType>::quiet_NaN();
    RasterType<ResultType> result(meta, static_cast<ResultType>(1));
    if constexpr (!result.raster_type_has_nan) {
        throw InvalidArgument("weightedProduct: makes only sense with floating point result rasters");
    }

    size_t size = result.size();

    // the real work
    for (auto& [raster, weight] : rasters) {
        for (size_t i = 0; i < size; ++i) {
            // nodata's are not skipped in weighted product (are skipped in weighted sum)
            result[i] *= static_cast<ResultType>(pow((*raster)[i], weight));

            if (std::isnan(result[i])) {
                result.mark_as_nodata(i);
            }
        }
    }
    ResultType max_value = maximum(result);
    if (max_value > 0) {
        result /= max_value;
    }

    return result;
}
}
