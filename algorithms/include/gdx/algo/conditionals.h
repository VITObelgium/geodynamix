#pragma once

#include "infra/cast.h"

namespace gdx {

template <
    template <typename> typename RasterType1, typename T1,
    template <typename> typename RasterType2, typename T2>
RasterType2<T2> if_then(const RasterType1<T1>& condition, const RasterType2<T2>& thenRaster)
{
    if (condition.size() != thenRaster.size()) {
        throw RuntimeError("If: Incompatible raster sizes if {}x{} then {}x{}",
                           condition.rows(), condition.cols(),
                           thenRaster.rows(), thenRaster.cols());
    }

    RasterType2<T2> result = thenRaster.copy();
    if (!result.nodata().has_value()) {
        result.set_nodata(std::numeric_limits<T2>::max());
    }

    for (std::size_t i = 0; i < condition.size(); ++i) {
        if (!condition[i]) {
            result.mark_as_nodata(i);
        }
    }

    return result;
}

template <
    template <typename> typename RasterType1, typename T1,
    template <typename> typename RasterType2, typename T2,
    template <typename> typename RasterType3, typename T3>
auto if_then_else(const RasterType1<T1>& condition, const RasterType2<T2>& thenRaster, const RasterType3<T3>& elseRaster)
{
    using ResultType = std::common_type_t<T2, T3>;

    if (condition.size() != thenRaster.size() || condition.size() != elseRaster.size()) {
        throw RuntimeError("If: Incompatible raster sizes if {}x{} then {}x{} else {}x{}",
                           condition.rows(), condition.cols(),
                           thenRaster.rows(), thenRaster.cols(),
                           elseRaster.rows(), elseRaster.cols());
    }

    auto meta = thenRaster.metadata();
    if (!meta.nodata.has_value()) {
        meta.nodata = elseRaster.metadata().nodata;

        if (!meta.nodata.has_value()) {
            meta.nodata = inf::truncate<double>(std::numeric_limits<ResultType>::max());
        }
    }

    RasterType2<ResultType> result(meta);

    for (std::size_t i = 0; i < condition.size(); ++i) {
        if (condition.is_nodata(i)) {
            result.mark_as_nodata(i);
            continue;
        }

        if (condition[i]) {
            if (thenRaster.is_nodata(i)) {
                result.mark_as_nodata(i);
            } else {
                result[i] = static_cast<ResultType>(thenRaster[i]);
            }
        } else {
            if (elseRaster.is_nodata(i)) {
                result.mark_as_nodata(i);
            } else {
                result[i] = static_cast<ResultType>(elseRaster[i]);
            }
        }
    }

    return result;
}

template <
    template <typename> typename RasterType, typename T1,
    typename T2,
    typename T3>
auto if_then_else(const RasterType<T1>& condition, std::optional<T2> thenValue, std::optional<T3> elseValue)
{
    using ResultType = std::common_type_t<T2, T3>;

    auto meta = condition.metadata();

    RasterType<ResultType> result(meta);
    if (!meta.nodata.has_value() && (!thenValue.has_value() || !elseValue.has_value())) {
        meta.nodata = inf::truncate<double>(std::numeric_limits<ResultType>::max());
    }

    for (std::size_t i = 0; i < condition.size(); ++i) {
        if (condition.is_nodata(i)) {
            result.mark_as_nodata(i);
            continue;
        }

        if (condition[i]) {
            if (thenValue.has_value()) {
                result[i] = static_cast<ResultType>(*thenValue);
            } else {
                result.mark_as_nodata(i);
            }
        } else {
            if (elseValue.has_value()) {
                result[i] = static_cast<ResultType>(*elseValue);
            } else {
                result.mark_as_nodata(i);
            }
        }
    }

    return result;
}
}
