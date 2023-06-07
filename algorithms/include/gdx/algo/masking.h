#pragma once

#include "gdx/algo/algorithm.h"
#include "gdx/exception.h"
#include "infra/span.h"

#include <algorithm>
#include <optional>
#include <unordered_map>

namespace gdx {

template <typename RasterType, typename MaskType>
void apply_mask_in_place(RasterType& ras, const MaskType& mask)
{
    gdx::for_each_optional_value(ras, mask, [&](auto& rasterValue, auto& maskValue) {
        if (maskValue.is_nodata()) {
            rasterValue.reset();
        }
    });
}

template <typename RasterType, typename MaskType>
RasterType apply_mask(const RasterType& ras, const MaskType& mask)
{
    RasterType result = ras.copy();
    apply_mask_in_place(result, mask);
    return result;
}

template <typename RasterType, typename MaskType>
void erase_outside_mask(RasterType& ras, const MaskType& mask)
{
    int size = int(mask.size());
    if (size != ras.size()) {
        throw InvalidArgument("erase outside mask : size of mask {} should match size of raster {}", size, ras.size());
    }
    if (ras.nodata().has_value()) {
        for (int32_t j = 0; j < size; ++j) {
            if (mask[j] == 0 || mask.is_nodata(j)) {
                ras[j] = ras.NaN;
                ras.mark_as_nodata(j);
            }
        }
    } else {
        for (int32_t j = 0; j < size; ++j) {
            if (mask[j] == 0 || mask.is_nodata(j)) {
                ras[j] = 0;
            }
        }
    }
}

/**
 * Apply a mask to the grid.
 * 	/param raster the input raster
 * 	/param mask a raster containing mask values
 * 	/param maskFunction	a function which accepts a value in the mask grid as argument and
 * 					returns true when the associated grid cell must be copied, and
 * 					false when the associated grid cell must be skipped.
 * /return a raster where each value of the grid, where the maskFunction returned
 * 	       true is present, and every other value of the input is not present.
 * /throw InvalidArgument if the mask dimensions do not equal the grid's dimensions.
 */
template <template <typename> typename RasterType, typename TRaster, typename TMask, typename MaskFunc>
RasterType<TRaster> mask(const RasterType<TRaster>& raster, const RasterType<TMask>& mask, MaskFunc&& maskFunction)
{
    if (size(raster) != size(mask)) {
        throw InvalidArgument("Raster size does not match mask size {} vs {}", size(raster), size(mask));
    }

    RasterType<TRaster> result = raster.copy();

    for (auto iter = optional_value_begin(mask); iter != optional_value_end(mask); ++iter) {
        auto cell = iter->cell();
        if (iter->is_nodata()) {
            if (!maskFunction(mask.nodata().value())) {
                result.mark_as_nodata(cell);
            }
        } else {
            if (!maskFunction(TMask(*iter))) {
                result.mark_as_nodata(cell);
            }
        }
    }

    return result;
}

template <template <typename> typename RasterType, typename TRaster, typename TMask>
RasterType<TRaster> inside_mask(const RasterType<TRaster>& raster, const RasterType<TMask>& mask)
{
    if (size(raster) != size(mask)) {
        throw InvalidArgument("Raster size does not match mask size {} vs {}", size(raster), size(mask));
    }

    RasterType<TRaster> result = raster.copy();

    for (auto iter = optional_value_begin(mask); iter != optional_value_end(mask); ++iter) {
        if (iter->is_nodata()) {
            // the mask has no data for this cell, so clear the output cell
            result.mark_as_nodata(iter->cell());
        }
    }

    return result;
}

template <template <typename> typename RasterType, typename TRaster, typename TMask>
RasterType<TRaster> outside_mask(const RasterType<TRaster>& raster, const RasterType<TMask>& mask)
{
    if (size(raster) != size(mask)) {
        throw InvalidArgument("Raster size does not match mask size {} vs {}", size(raster), size(mask));
    }

    RasterType<TRaster> result = raster.copy();

    for (auto iter = optional_value_begin(mask); iter != optional_value_end(mask); ++iter) {
        if (!iter->is_nodata()) {
            // the mask has data for this cell, so clear the output cell
            result.mark_as_nodata(iter->cell());
        }
    }

    return result;
}

/*!
 * \brief Apply a mask to the grid and create the sum of all values in the mask.
 * \param ras the input raster
 * \param mask a raster containing mask values
 * \param maskValues an array with mask values for which sums are made
 * \return a unordered_map of value, sum pairs
 */
template <typename SumType, template <typename> typename RasterType, typename TData, typename TMask>
auto sum_mask(const RasterType<TData>& ras, const RasterType<TMask>& mask)
{
    if (size(ras) != size(mask)) {
        throw InvalidArgument("sumMask: raster sizes must match {} vs {}", size(ras), size(mask));
    }

    std::unordered_map<TMask, SumType> result;
    gdx::for_each_data_value(ras, mask, [&result](TData rasterValue, TMask maskValue) {
        result[maskValue] += static_cast<SumType>(rasterValue);
    });

    return result;
}

/**
 * Apply a mask to the grid and sum of all values in the mask (operator+ is used).
 * arguments:
 * \param raster the input data
 * \param mask a raster containing mask values
 * \param maskValues a vector with mask values for which sums are made
 * \return	a map of value, sum pairs
 * \throw an InvalidArgument if the mask dimensions do not equal the grid's dimensions.
 */
template <typename SumType, template <typename> typename RasterType, typename TData, typename TMask>
std::unordered_map<TMask, SumType> sum_mask(const RasterType<TData>& raster, const RasterType<TMask>& mask, std::span<const typename RasterType<TMask>::value_type> maskValues)
{
    if (size(raster) != size(mask)) {
        throw InvalidArgument("Raster size does not match mask size {} vs {}", size(raster), size(mask));
    }

    std::unordered_map<TMask, SumType> result;
    for (auto& val : maskValues) {
        result.emplace(val, SumType(0));
    }

    gdx::for_each_data_value(raster, mask, [&](TData rasterValue, TMask maskValue) {
        auto iter = result.find(maskValue);
        if (iter != result.end()) {
            iter->second += static_cast<SumType>(rasterValue);
        }
    });

    return result;
}

template <typename CountType, template <typename> typename RasterType, typename TMask>
std::unordered_map<TMask, CountType> count_mask(const RasterType<TMask>& mask)
{
    std::unordered_map<TMask, CountType> result;

    for (auto iter = value_begin(mask); iter != value_end(mask); ++iter) {
        result[TMask(*iter)] += CountType(1);
    }

    return result;
}

/** Cells in the raster will be set to 1 for values in the mask that are > 0
 * /param mask data to check for cells bigger then 0
 * /param raster in which cells will be set to 1
 */
template <template <typename> typename RasterType, typename TMask, typename TData>
void include_mask(const RasterType<TMask>& mask, RasterType<TData>& raster)
{
    if (size(raster) != size(mask)) {
        throw InvalidArgument("Raster size does not match mask size {} vs {}", size(raster), size(mask));
    }

    for (auto iter = value_begin(mask); iter != value_end(mask); ++iter) {
        if (TMask(*iter) > 0) {
            const auto cell = iter->cell();
            raster.mark_as_data(cell);
            raster[cell] = TData(1);
        }
    }
}

template <template <typename> typename RasterType, typename TMask, typename TData>
void exclude_mask(const RasterType<TMask>& mask, RasterType<TData>& raster)
{
    if (size(raster) != size(mask)) {
        throw InvalidArgument("Raster size does not match mask size {} vs {}", size(raster), size(mask));
    }

    for (auto iter = value_begin(mask); iter != value_end(mask); ++iter) {
        if (TMask(*iter) > 0) {
            raster.mark_as_nodata(iter->cell());
        }
    }
}

}
