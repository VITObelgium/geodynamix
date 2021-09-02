#pragma once

#include "gdx/exception.h"

#include "gdx/algo/algorithm.h"
#include "gdx/algo/maximum.h"
#include "gdx/algo/minimum.h"
#include "gdx/algo/nodata.h"

namespace gdx {

template <typename TResult, typename T>
TResult remap_to_float(T value, T min, T max, TResult mapStart, TResult mapEnd)
{
    static_assert(std::is_floating_point_v<T>, "Floating point input type required");
    assert(min < max);

    const auto rangeWidth  = static_cast<double>(max - min);
    const auto pos         = static_cast<double>(value - min) / rangeWidth;
    const auto mappedWidth = static_cast<double>(mapEnd - mapStart);

    return static_cast<TResult>(mapStart + (mappedWidth * pos));
}

template <typename T>
static uint8_t remap_to_byte(T value, T start, T end, uint8_t mapStart, uint8_t mapEnd)
{
    if (value < start || value > end) {
        return 0;
    }

    if (mapStart == mapEnd) {
        return mapStart;
    }

    const auto rangeWidth = end - start;
    const auto pos        = static_cast<float>(value - start) / static_cast<float>(rangeWidth);

    const auto mapWidth = mapEnd - mapStart;
    return static_cast<uint8_t>(std::round(mapStart + (mapWidth * pos)));
}

/*! rescale the range of values to the range in [0, 1]
 *  Maps the rane [min, max] to [0, 1]
 */
template <
    typename InputRasterType,
    typename OutputRasterType,
    typename TInput  = typename InputRasterType::value_type,
    typename TOutput = typename OutputRasterType::value_type>
void normalise(const InputRasterType& input, OutputRasterType& output, TInput minIn, TInput maxIn, TOutput minOut, TOutput maxOut)
{
    if (size(input) != size(output)) {
        throw InvalidArgument("normalise: raster sizes must match {} vs {}", size(input), size(output));
    }

    if constexpr (std::is_floating_point_v<TOutput>) {
        if (minOut == TOutput(0) && minIn == TInput(0)) {
            // when we map to a range from 0, this is more precise
            output = input * (maxOut / maxIn);
        } else {
            gdx::transform(input, output, [=](TInput value) {
                return remap_to_float<TOutput>(value, minIn, maxIn, minOut, maxOut);
            });
        }
    } else if constexpr (std::is_same_v<uint8_t, TOutput>) {
        gdx::transform(input, output, [=](TInput value) {
            return remap_to_byte(value, minIn, maxIn, minOut, maxOut);
        });
    } else {
        throw RuntimeError("Only normalise to floating point or byte is currently supported");
    }
}

/*! rescale the range of values to the range in [0, 1]
 *  Maps the rane [min, max] to [0, 1]
 */
template <
    typename TOutput,
    template <typename> typename RasterType,
    typename TInput>
RasterType<TOutput> normalise_min_max(const RasterType<TInput>& input, TOutput mapStart, TOutput mapEnd)
{
    auto meta = inf::copy_metadata_replace_nodata(input.metadata(), nodata_cast<TOutput>(input.metadata().nodata));
    RasterType<TOutput> result(meta);

    try {
        auto [min, max] = gdx::minmax(input);
        normalise(input, result, min, max, mapStart, mapEnd);
    } catch (const InvalidArgument&) {
        // only nodata values are present
        make_nodata(result);
    }

    return result;
}

/*! rescale the range of values to the range in [0, 1]
 *  Maps the rane [0, max] to [0, 1]
 */
template <
    typename TOutput,
    template <typename> typename RasterType,
    typename TInput>
RasterType<TOutput> normalise_max(const RasterType<TInput>& input, TOutput mapStart, TOutput mapEnd)
{
    RasterType<TOutput> result(inf::copy_metadata_replace_nodata(input.metadata(), nodata_cast<TOutput>(input.metadata().nodata)));

    try {
        const auto max = gdx::maximum(input);
        normalise(input, result, TInput(0), max, mapStart, mapEnd);
    } catch (const InvalidArgument&) {
        // only nodata values are present
        make_nodata(result);
    }

    return result;
}

/*! rescale the range of values to the range in [mapStart, mapEnd]
 *  Maps the range [0, max] to [mapStart, mapEnd]
 */
template <
    template <typename> typename RasterType,
    typename T>
void normalise_max_in_place(RasterType<T>& input, T mapStart, T mapEnd)
{
    try {
        const auto max = gdx::maximum(input);
        normalise(input, input, T(0), max, mapStart, mapEnd);
    } catch (const InvalidArgument&) {
        // only nodata values are present
    }
}

}
