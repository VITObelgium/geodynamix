#pragma once

#include "gdx/exception.h"
#include "infra/cast.h"

#include <cassert>
#include <vector>

namespace gdx {

/*! Computes sum of maps.  Each map is multiplied with a 'weight' first.
 *  Returns a gdx raster with the result.
 *  Only floating point result rasters make sense.
 */

template <typename ResultType, template <typename> typename RasterType, typename FloatType, typename WeightType>
RasterType<ResultType> weighted_sum(const std::vector<std::pair<const RasterType<FloatType>*, WeightType>>& rasters)
{
    static_assert(RasterType<ResultType>::raster_type_has_nan, "weightedSum: makes only sense with floating point rasters");

    // some checks on the input and preprocessing
    if (rasters.empty()) {
        throw InvalidArgument("weighted sum: at least one raster should be supplied");
    }

    for (std::size_t i = 1; i < rasters.size(); ++i) {
        if (rasters[0].first->size() != rasters[i].first->size()) {
            throw InvalidArgument("weighted sum: size of rasters should all be equal");
        }
    }

    WeightType sum_weight = 0;
    for (std::size_t i = 0; i < rasters.size(); ++i) {
        sum_weight += rasters[i].second;
    }

    auto resultMeta   = rasters[0].first->metadata();
    resultMeta.nodata = std::numeric_limits<double>::quiet_NaN();
    RasterType<ResultType> result(resultMeta, 0);
    assert(!result.is_nodata(0)); // check if all is initialised to data

    const int size = inf::truncate<int>(result.size());

    for (auto& iter : rasters) {
        const RasterType<FloatType>& raster = *(iter.first);
        const WeightType w                  = (iter.second);
        for (int i = 0; i < size; ++i) {
            if (!raster.is_nodata(i)) { // nodata's are skipped with weighted sum (not in weighted product)
                result[i] += static_cast<ResultType>(raster[i] * w);
            }
        }
    }

    for (int i = 0; i < size; ++i) {
        if (result[i] == 0) {
            result.mark_as_nodata(i);
        }
    }

    if (sum_weight > 0) {
        result /= inf::truncate<ResultType>(sum_weight);
    }

    return result;
}
}
