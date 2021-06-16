#pragma once

#include "gdx/rastermetadata.h"
#include "infra/cast.h"

#include <limits>

namespace gdx::detail {

template <typename T>
constexpr double nodata_for_type() noexcept
{
    if (std::numeric_limits<T>::has_quiet_NaN) {
        return double(std::numeric_limits<T>::quiet_NaN());
    }

    if (std::is_unsigned_v<T>) {
        return double(std::numeric_limits<T>::lowest());
    } else {
        return double(std::numeric_limits<T>::max());
    }
}

template <typename ResultType>
double find_best_nodata_value(const RasterMetadata& meta1, const RasterMetadata& meta2)
{
    if (meta1.nodata.has_value()) {
        if (inf::fits_in_type<ResultType>(*meta1.nodata)) {
            return *meta1.nodata;
        }
    }

    if (meta2.nodata.has_value()) {
        if (inf::fits_in_type<ResultType>(*meta2.nodata)) {
            return *meta2.nodata;
        }
    }

    return nodata_for_type<ResultType>();
}

template <typename InputRaster1, typename InputRaster2, typename ResultRaster>
void assign_nodata_value(const InputRaster1& in1, const InputRaster2& in2, ResultRaster& result)
{
    using TResult = typename ResultRaster::value_type;

    if (result.nodata().has_value() || (!in1.nodata().has_value() && !in2.nodata().has_value())) {
        return;
    }

    if (!in1.nodata().has_value() && in2.nodata().has_value()) {
        result.set_nodata(find_best_nodata_value<TResult>(result.metadata(), in2.metadata()));
        return;
    }

    result.set_nodata(find_best_nodata_value<TResult>(result.metadata(), in1.metadata()));
}

}
