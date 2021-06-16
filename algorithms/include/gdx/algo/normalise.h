#pragma once

#include "gdx/exception.h"

#include "gdx/algo/algorithm.h"
#include "gdx/algo/maximum.h"
#include "gdx/algo/minimum.h"
#include "gdx/algo/nodata.h"

namespace gdx {

template <typename T>
float remap_to_float(T value, T min, T max)
{
    assert(min < max);

    const auto rangeWidth = static_cast<float>(max - min);
    return static_cast<float>(value - min) / rangeWidth;
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

// The optional nodata argument can be used to specify the nodata value of the result when you know
// that the nodata value of the input does not fit into a byte
template <
    typename InputRasterType,
    typename OutputRasterType,
    typename TInput  = typename InputRasterType::value_type,
    typename TOutput = typename OutputRasterType::value_type>
void normalise(const InputRasterType& input, OutputRasterType& output)
{
    if (size(input) != size(output)) {
        throw InvalidArgument("normalise: raster sizes must match {} vs {}", size(input), size(output));
    }

    TInput min, max;
    try {
        std::tie(min, max) = minmax(input);
    } catch (const InvalidArgument&) {
        // only nodata values are present
        make_nodata(output);
        return;
    }

    if constexpr (std::is_same_v<float, TOutput>) {
        gdx::transform(input, output, [=](TInput value) {
            return remap_to_float(value, min, max);
        });
    } else if constexpr (std::is_same_v<uint8_t, TOutput>) {
        auto mapEnd = std::numeric_limits<TOutput>::max() - 1;
        gdx::transform(input, output, [=](TInput value) {
            return remap_to_byte(value, min, max, 0, mapEnd);
        });
    } else {
        throw RuntimeError("Only normalise to float or byte is currently supported");
    }
}
}
