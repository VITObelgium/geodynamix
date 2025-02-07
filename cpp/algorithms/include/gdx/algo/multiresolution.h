#pragma once

#include "gdx/cell.h"
#include "gdx/exception.h"
#include "gdx/rastermetadata.h"

#include <vector>

namespace gdx {

/*! Inflate resolution by replacing each cell by NxN smaller cells with the same value.
 *  Always use this for categoric maps.
 *  Applicable also for numeric maps with unit of measure independant of the cell size.
 *  Returns a gdx raster with the result.
 */

template <typename ValueType, template <typename> typename RasterType>
RasterType<ValueType> inflate(const RasterType<ValueType>& inmap, const int inflate_factor)
{
    if (inflate_factor <= 0) {
        throw InvalidArgument("inflate: inflate factor should be > 0");
    }
    RasterMetadata meta(inmap.metadata());
    meta.rows *= inflate_factor;
    meta.cols *= inflate_factor;
    meta.cellSize /= inflate_factor;
    RasterType<ValueType> result(meta);
    const int rows = int(inmap.metadata().rows);
    const int cols = int(inmap.metadata().cols);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const ValueType v = inmap(r, c);
            for (int rr = 0; rr < inflate_factor; ++rr) {
                for (int cc = 0; cc < inflate_factor; ++cc) {
                    result(r * inflate_factor + rr, c * inflate_factor + cc) = v;
                }
            }
        }
    }
    return result;
}

/*! Inflate resolution by replacing each cell by NxN smaller cells each with 1/(NxN) smaller value.
 *  So the sum of values on the map stays the same.
 *  Applicable for numeric maps containing an amount per cell.
 *  Only floating point result type maps makes sense.
 *  Returns a gdx raster with the result.
 */

template <typename ResultType, template <typename> typename RasterType, typename ValueType>
RasterType<ResultType> inflate_equal_sum(const RasterType<ValueType>& inmap, const int inflate_factor)
{
    static_assert(RasterType<ValueType>::raster_type_has_nan, "inflate_equal_sum: makes only sense with floating point rasters");

    if (inflate_factor <= 0) {
        throw InvalidArgument("inflate_equal_sum: inflate factor should be > 0");
    }
    if (inmap.metadata().cellSize <= 0) {
        throw InvalidArgument("intflateEqualSum: inmap should have a cellSize");
    }
    RasterMetadata meta(inmap.metadata());
    meta.rows *= inflate_factor;
    meta.cols *= inflate_factor;
    meta.cellSize /= inflate_factor;
    RasterType<ResultType> result(meta);

    const int rows = inmap.rows();
    const int cols = inmap.cols();
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const ResultType v = static_cast<ResultType>(inmap(r, c)) / static_cast<ResultType>(inflate_factor * inflate_factor);
            for (int rr = 0; rr < inflate_factor; ++rr) {
                for (int cc = 0; cc < inflate_factor; ++cc) {
                    result(r * inflate_factor + rr, c * inflate_factor + cc) = v;
                }
            }
        }
    }

    return result;
}

/*! Deflate resolution by replacing NxN smaller cells by their sum in one bigger cell.
 *  So the sum of values on the map stays the same.
 *  Applicable for numeric maps containing an amount per cell.
 *  Returns a gdx raster with the result.
 */

template <typename ValueType, template <typename> typename RasterType>
RasterType<ValueType> deflate_equal_sum(
    const RasterType<ValueType>& inmap,
    const int deflate_factor)
{
    static_assert(RasterType<ValueType>::raster_type_has_nan, "deflate_equal_sum: makes only sense with floating point rasters");

    if (deflate_factor <= 0) {
        throw InvalidArgument("deflate_equal_sum: deflate factor should be > 0");
    }
    if (inmap.metadata().rows % deflate_factor != 0) {
        throw InvalidArgument("deflate_equal_sum: inmap rows should be multiple of deflate factor");
    }
    if (inmap.metadata().cols % deflate_factor != 0) {
        throw InvalidArgument("deflate_equal_sum: inmap cols should be multiple of deflate factor");
    }
    if (inmap.metadata().cellSize.x <= 0) {
        throw InvalidArgument("deflate_equal_sum: inmap should have a cellSize");
    }

    RasterMetadata meta(inmap.metadata());
    meta.rows /= deflate_factor;
    meta.cols /= deflate_factor;
    meta.cellSize.x *= deflate_factor;
    meta.cellSize.y *= deflate_factor;
    meta.nodata = std::numeric_limits<double>::quiet_NaN();
    RasterType<ValueType> result(meta, std::numeric_limits<ValueType>::quiet_NaN());

    const int rows = result.rows();
    const int cols = result.cols();
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            ValueType v    = 0;
            int count_data = 0;
            for (int rr = 0; rr < deflate_factor; ++rr) {
                for (int cc = 0; cc < deflate_factor; ++cc) {
                    Cell cell(r * deflate_factor + rr, c * deflate_factor + cc);
                    if (!inmap.is_nodata(cell)) {
                        v += inmap[cell];
                        ++count_data;
                    }
                }
            }
            if (count_data > 0) {
                result(r, c) = v;
            }
        }
    }
    return result;
}

}
