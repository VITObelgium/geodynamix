#pragma once

#include "infra/color.h"
#include "infra/colormap.h"
#include "infra/span.h"

#include "gdx/algo/algorithm.h"
#include "gdx/algo/normalise.h"

#include <functional>
#include <optional>

namespace gdx {

/*! Apply a provided colormap to the provided raster
 * Nodata values will be transparent
 * \input the raster to process
 * \cmap the color map definition
 * \min 
 * \max
 * \return a vector with a color for each cell
 */
template <template <typename> typename RasterType, typename T>
void apply_colormap(const RasterType<T>& input, std::span<inf::Color> output, const inf::ColorMap& cmap, std::optional<T> min = std::optional<T>(), std::optional<T> max = std::optional<T>())
{
    if (input.size() != output.size()) {
        throw InvalidArgument("colormap: raster sizes must match {} vs {}", input.size(), output.size());
    }

    T rangeMin, rangeMax;
    if (min.has_value() && max.has_value()) {
        rangeMin = min.value();
        rangeMax = max.value();
    } else {
        auto [minValue, maxValue] = gdx::minmax(input);
        rangeMin                  = min.value_or(minValue);
        rangeMax                  = max.value_or(maxValue);
    }

    std::transform(optional_value_begin(input), optional_value_end(input), output.begin(), [&cmap, rangeMin, rangeMax](auto value) {
        if (!value) {
            return inf::Color();
        }

        return cmap.get_color(remap_to_byte(*value, rangeMin, rangeMax, 0, 255));
    });
}

/*! Convenience overload that allocates the resulting color data
 */
template <template <typename> typename RasterType, typename T>
std::vector<inf::Color> apply_colormap(const RasterType<T>& input, const inf::ColorMap& cmap, std::optional<T> min = std::optional<T>(), std::optional<T> max = std::optional<T>())
{
    std::vector<inf::Color> result(input.size());
    apply_colormap(input, result, cmap, min, max);
    return result;
}

}
