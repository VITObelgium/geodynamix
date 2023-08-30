#pragma once

#include "gdx/algo/algorithm.h"
#include "gdx/algo/sum.h"
#include "gdx/exception.h"
#include "infra/span.h"

#include <limits>
#include <unordered_map>
#include <vector>

namespace gdx {

/**
* Distribute the values of the grid among a number of other grids.
*
* /param raster the input raster
* /param proportions a list of proportions by which the values need to be distributed. If the
*                    sum of the proportions equals 1, then the sum of the output equals the input.
* /return a vector of RasterType<ResultType> rasters which contain the output grids.
          If the proportions vector is empty, an empty list is returned.
*/

template <typename ResultType, template <typename> typename RasterType, typename T>
std::vector<RasterType<ResultType>> distribute(const RasterType<T>& raster, std::span<const float> proportions)
{
    std::vector<RasterType<ResultType>> result;
    result.reserve(proportions.size());

    auto metadata = raster.metadata();
    if (!metadata.nodata.has_value()) {
        if constexpr (std::numeric_limits<ResultType>::has_quiet_NaN) {
            metadata.nodata = inf::truncate<double>(std::numeric_limits<ResultType>::quiet_NaN());
        } else {
            metadata.nodata = inf::truncate<double>(std::numeric_limits<ResultType>::max());
        }
    }

    for (float prop : proportions) {
        result.emplace_back(raster.metadata(), inf::truncate<ResultType>(metadata.nodata.value()));
        if (prop > 0.f) {
            gdx::transform(raster, result.back(), [prop](T value) {
                return inf::truncate<ResultType>(value * prop);
            });
        }
    }

    return result;
}

/*! Distributes the provided value over the cells in the raster proportional to the values in the raster.
 * /param valueToDistribute the value to distribute over the cells containing data
 * /param totalValue the total value to distribute the values against, if it is empty the sum of the raster is used
 * /return a raster with the result.
 */
template <typename ResultType, template <typename> typename RasterType, typename T>
RasterType<ResultType> value_distribution(double valueToDistribute, const RasterType<T>& raster, std::optional<double> totalValue = {})
{
    RasterType<ResultType> result(raster.metadata(), inf::truncate<ResultType>(raster.nodata().value()));
    if (!totalValue.has_value()) {
        totalValue = sum(raster);
    }

    gdx::transform(raster, result, [valueToDistribute, sumGrid = totalValue.value()](T value) {
        return inf::truncate<ResultType>(value * valueToDistribute / sumGrid);
    });

    return result;
}

template <typename RasterType>
struct RasterDistributionResult
{
    RasterDistributionResult(const RasterMetadata& meta, typename RasterType::value_type fill)
    : fraction(meta, fill)
    , remainder(meta, fill)
    {
    }

    RasterType fraction;
    RasterType remainder;
};

/**
 * Distribute the values of the grid cell by cell into a fraction and a remainder.
 * /param raster the raster containing the values to distribute
 * /param fractions the fractions to use for each of the cells
 * /return a RasterDistributionResult<RasterType<ResultType>> struct.
 *         The structure contains 2 rasters: The 'fraction' raster and 'remainder' raster.
 *         For each x,y: fraction(x,y) = raster(x,y) * fractions(x,y)
 *                       remainder(x,y) = grid(x,y) * (1.0 - fractions(x,y))
 * 	       if raster(x,y) is missing, fraction(x,y) and remainder(x,y) will also be missing
 * 	       if raster(x,y) is not missing, and fractions(x,y) is missing, fractions(x,y) is supposed to
 * 	       equal zero, so grid(x,y) is copied into remainder(x,y)
 * throws an InvalidValueException if there is a (x,y) for which grid(x,y) is not missing,
 * 	and fractions(x,y) lies outside of [0,1] So if fractions.missing lies within [0,1],
 * 	NO exception is thrown.
 */
template <typename ResultType, template <typename> typename RasterType, typename TInput, typename TFractions>
RasterDistributionResult<RasterType<ResultType>> raster_distribution(const RasterType<TInput>& raster, const RasterType<TFractions>& fractions)
{
    static_assert(std::is_floating_point_v<TFractions>, "Fractions raster should be floating point");

    if (size(raster) != size(fractions)) {
        throw InvalidArgument("Raster size does not match fractions size {} vs {}", size(raster), size(fractions));
    }

    RasterDistributionResult<RasterType<ResultType>> result(raster.metadata(), inf::truncate<ResultType>(raster.nodata().value()));

    for (auto iter = optional_value_begin(raster); iter != optional_value_end(raster); ++iter) {
        if (iter->has_value()) {
            auto cell      = iter->cell();
            float fraction = fractions.is_nodata(cell) ? 0.f : fractions[cell];
            if (fraction < 0.f || fraction > 1.f) {
                throw InvalidArgument("Fractions must be in the range [0.0, 1.0]");
            }

            result.fraction[cell]  = TInput(*iter) * fraction;
            result.remainder[cell] = TInput(*iter) * (1.f - fraction);

            result.fraction.mark_as_data(cell);
            result.remainder.mark_as_data(cell);
        }
    }

    return result;
}

/**
 * Distribute the values of the grid spatially.
 * arguments:
 * \param raster the input raster
 * \param mask a raster containing the mask: that is each cell contains an id of type TMask
 * \param id2location a mapping between ids and Cells
 * \return a raster where the values of the original grid appear summed up in the
 * 	locations identified by the ids in the mask argument.
 * \throw RuntimeError if a location in the id2location argument is outside of the grid's bounds.
 * \throw InvalidArgument if the mask dimensions do not equal the grid's dimensions.
 * \throw RuntimeError if an id was not found in the id2location mapping.
 * \throw RuntimeError if a value could not be distributed because the mask did not contain
 * 	an id at the value's location.
 */
template <template <typename> typename RasterType, typename TData, typename TMask>
RasterType<TData> distribute(const RasterType<TData>& raster, const RasterType<TMask>& mask, const std::unordered_map<TMask, Cell>& id2location)
{
    if (size(raster) != size(mask)) {
        throw InvalidArgument("Raster size does not match mask size {} vs {}", size(raster), size(mask));
    }

    std::unordered_map<TMask, TData> sums;

    RasterType<TData> result(raster.metadata(), raster.nodata().value());

    for (auto iter = optional_value_begin(raster); iter != optional_value_end(raster); ++iter) {
        if (!iter->is_nodata()) {
            auto cell = iter->cell();
            if (mask.is_nodata(cell)) {
                throw RuntimeError("No id present in mask at location: {}", cell);
            }

            auto id = mask[cell];
            sums[id] += TData(*iter);
        }
    }

    for (auto& [id, sum] : sums) {
        auto iter = id2location.find(id);
        if (iter == id2location.end()) {
            throw RuntimeError("No mapping available for mask value {}", id);
        }

        result.mark_as_data(iter->second);
        result[iter->second] = sum;
    }

    return result;
}

}
