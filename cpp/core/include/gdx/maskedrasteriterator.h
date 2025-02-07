#pragma once

#include "gdx/cell.h"
#include "gdx/rasteriterator.h"
#include "infra/cast.h"

#include <cassert>
#include <cmath>
#include <iterator>
#include <limits>
#include <optional>

namespace gdx {

template <typename T, bool is_const>
using MissingValueMaskIterator = MissingValueIterator<T, is_const, MaskValueProxy<T, is_const>>;

template <typename T>
using MissingValueMaskConstIterator = MissingValueMaskIterator<T, true>;

template <typename TData, bool is_const, typename TMask = bool>
using MaskSkippingIterator = RasterIterator<TData, is_const, NodataMaskFilterPolicy<TData, TMask, is_const>, AllLocationsFilterPolicy, MaskValueProxy<TData, is_const>>;

template <typename TData, typename TMask = bool>
using MaskSkippingConstIterator = MaskSkippingIterator<TData, true, TMask>;

// NoData concept for a container type C
// C::mask_value_type                                   type of the nodata mask
// const mask_value_type* C::mask() const noexcept      function that returns the mask pointer

// supports skipping over containers that implement the masked nodata concept
template <
    typename Container,
    typename std::enable_if_t<std::is_arithmetic_v<typename std::decay_t<Container>::mask_value_type>, int>* = nullptr>
auto value_begin(Container&& rasterData)
{
    constexpr bool isConst = std::is_const_v<std::remove_reference_t<Container>>;

    using Raster       = std::decay_t<Container>;
    using T            = typename Raster::value_type;
    using TMask        = typename Raster::mask_value_type;
    using NodataPolicy = NodataMaskFilterPolicy<T, TMask, isConst>;

    return MaskSkippingIterator<T, isConst>(
        rasterData.data(),
        rasterData.rows(),
        rasterData.cols(),
        NodataPolicy(rasterData.data(), rasterData.mask()),
        AllLocationsFilterPolicy());
}

template <typename Container,
    typename std::enable_if_t<std::is_arithmetic_v<typename std::decay_t<Container>::mask_value_type>, int>* = nullptr>
auto value_end(Container&& /*rasterData*/)
{
    using T = typename std::decay_t<Container>::value_type;

    constexpr bool isConst = std::is_const_v<std::remove_reference_t<Container>>;
    return MaskSkippingIterator<T, isConst>();
}

template <
    typename Container,
    typename std::enable_if_t<std::is_arithmetic_v<typename std::decay_t<Container>::mask_value_type>, int>* = nullptr>
auto value_cbegin(Container&& data)
{
    using T            = typename std::decay_t<Container>::value_type;
    using TMask        = typename std::decay_t<Container>::mask_value_type;
    using NodataPolicy = NodataMaskFilterPolicy<T, TMask, true>;

    return MaskSkippingConstIterator<T>(data.data(), data.rows(), data.cols(), NodataPolicy(data.data(), data.mask()), AllLocationsFilterPolicy());
}

template <typename Container,
    typename std::enable_if_t<std::is_arithmetic_v<typename std::decay_t<Container>::mask_value_type>, int>* = nullptr>
auto value_cend(const Container& /*data*/)
{
    using T = typename std::decay_t<Container>::value_type;
    return MaskSkippingConstIterator<T>();
}

// supports skipping over containers that implement the masked nodata concept
template <
    typename Container,
    typename std::enable_if_t<std::is_arithmetic_v<typename std::decay_t<Container>::mask_value_type>, int>* = nullptr>
auto optional_value_begin(const Container& rasterData)
{
    using T = typename std::decay_t<Container>::value_type;
    return MissingValueMaskIterator<T, true>(data(rasterData), 0, rasterData.cols(), rasterData.mask());
}

template <
    typename Container,
    typename std::enable_if_t<std::is_arithmetic_v<typename std::decay_t<Container>::mask_value_type>, int>* = nullptr>
auto optional_value_begin(Container& rasterData)
{
    using T = typename std::decay_t<Container>::value_type;
    return MissingValueMaskIterator<T, false>(data(rasterData), 0, rasterData.cols(), rasterData.mask());
}

template <
    typename Container,
    typename std::enable_if_t<std::is_arithmetic_v<typename std::decay_t<Container>::mask_value_type>, int>* = nullptr>
auto optional_value_end(const Container& rasterData)
{
    using T = typename std::decay_t<Container>::value_type;
    return MissingValueMaskIterator<T, true>(data(rasterData), size(rasterData), rasterData.cols(), nullptr);
}

template <
    typename Container,
    typename std::enable_if_t<std::is_arithmetic_v<typename std::decay_t<Container>::mask_value_type>, int>* = nullptr>
auto optional_value_end(Container& rasterData)
{
    using T = typename std::decay_t<Container>::value_type;
    return MissingValueMaskIterator<T, false>(data(rasterData), size(rasterData), rasterData.cols(), nullptr);
}
}
