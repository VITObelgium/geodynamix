#pragma once

#include "gdx/algo/algorithm.h"
#include "gdx/algo/sum.h"
#include "gdx/cell.h"
#include "gdx/exception.h"
#include "gdx/rastermetadata.h"

#include "infra/algo.h"
#include "infra/cast.h"

#include <map>
#include <set>

namespace gdx {

/*! Helper function for weightedDistribution.
 */

struct WeightsInZoneInfo
{
    double sum_weights     = 0;
    int count_data_cells   = 0; // number of data cells in this zone
    int count_nodata_cells = 0; // number of nodata cells in this zone
};

template <template <typename> typename RasterType, typename ZoneType, typename FloatType, typename AmountType>
std::unordered_map<ZoneType, WeightsInZoneInfo> sum_weights_per_zone(const RasterType<ZoneType>& zones,
    const RasterType<FloatType>& weights,
    const std::unordered_map<ZoneType, AmountType>& amounts)
{
    std::unordered_map<ZoneType, WeightsInZoneInfo> result;
    const int size = int(zones.size());
    if (zones.size() != weights.size()) {
        throw InvalidArgument("weighted distribution: raster sizes should match {} {}", zones.size(), weights.size());
    }

    for (int i = 0; i < size; ++i) {
        if (!zones.is_nodata(i)) {
            auto z = zones[i];
            if (z < 0) {
                throw InvalidArgument("weighted distribution: zone raster should be non-negative");
            }
            auto iter = result.find(z);
            if (iter == result.end()) {
                result[z] = WeightsInZoneInfo();
                iter      = result.find(z);
            }
            if (!weights.is_nodata(i)) {
                auto w = weights[i];
                if (w < 0) {
                    throw InvalidArgument("weighted distribution: weight raster should be non-negative");
                }
                ++iter->second.count_data_cells;
                iter->second.sum_weights += w;
            } else {
                ++iter->second.count_nodata_cells;
            }
        }
    }

    for (auto& amount : amounts) {
        if (result.find(amount.first) == result.end() && amount.second != 0) {
            throw InvalidArgument("weighted distribution: amount ({}) for zone that is not on the zoning raster", amount.first);
        }
    }

    return result;
}

/*! Distributes an amount per zone proportional to a weight per cell raster.
 *  Returns a gdx raster with the result.
 *  Only floating point result rasters make sense.
 */

template <typename ResultType, template <typename> typename RasterType, typename ZoneType, typename WeightType, typename AmountType>
RasterType<ResultType> weighted_distribution(
    const RasterType<ZoneType>& zones,
    const RasterType<WeightType>& weights,
    const std::unordered_map<ZoneType, AmountType>& amounts,
    const bool zero_is_nodata // true in most weiss algo's except in aggregateandspreadmultiresolution
)
{
    static_assert(RasterType<ResultType>::raster_type_has_nan, "weighted_distribution: makes only sense with floating point rasters");

    // some checks on the input and some preprocessing of the input.
    const int size = int(zones.size());
    if (zones.metadata().rows != weights.metadata().rows || zones.metadata().cols != weights.metadata().cols) {
        throw InvalidArgument("weighted distribution: zones raster and weights raster should have equal extent");
    }

    auto nodataValue = zero_is_nodata ? ResultType(0) : RasterType<ResultType>::NaN;
    RasterType<ResultType> result(inf::copy_metadata_replace_nodata(zones.metadata(), nodataValue), nodataValue);

    auto sum_weights = sum_weights_per_zone<RasterType, ZoneType, WeightType>(zones, weights, amounts);

    // the real work
    for (int i = 0; i < size; ++i) {
        if (!zones.is_nodata(i)) {
            const ZoneType zone = zones[i];
            auto* amount        = inf::find_in_map(amounts, zone);
            if (!amount) {
                continue;
            }

            if (sum_weights[zone].sum_weights > 0) {
                if (!weights.is_nodata(i)) {
                    result.mark_as_data(i);
                    result[i] = static_cast<ResultType>(*amount * weights[i] / float(sum_weights[zone].sum_weights));
                }
            } else if (sum_weights[zone].count_data_cells > 0) {
                // All weights are ZERO.  spread amount evenly over the data cells
                if (!weights.is_nodata(i)) {
                    result.mark_as_data(i);
                    result[i] = static_cast<ResultType>(*amount / float(sum_weights[zone].count_data_cells));
                }
            } else {
                assert(sum_weights[zone].count_nodata_cells > 0);
                // All weights are NODATA.  spread amount evenly over ALL NODATA CELLS
                result.mark_as_data(i);
                result[i] = static_cast<ResultType>(*amount / float(sum_weights[zone].count_nodata_cells));
            }
            if (zero_is_nodata) {
                if (result[i] == 0) {
                    result.mark_as_nodata(i);
                }
            }
        } else {
            result.mark_as_nodata(i);
        }
    }
    return result;
}

}
