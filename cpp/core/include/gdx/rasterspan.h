#pragma once

#include "gdx/cell.h"
#include "gdx/rasteriterator.h"
#include "gdx/rastermetadata.h"
#include "infra/cast.h"
#include "infra/span.h"

namespace gdx {

/*!
 * Non owming view on raster data
 * Provides a raster interface on existing continuous data
 * in memory in combination with raster metadata
 *
 * e.g.: Raster span for regular std::vector
 *
 * RasterMetadata meta(2, 2, -1);
 * std::vector<int> data = {1, 2, 3, -1};
 * auto dataSpan = make_raster_span(data, meta);
 * assert(dataSpan(0, 1) == 2);
 * assert(dataSpan.is_nodata(1, 1));
 **/
template <typename T>
class raster_span : public std::span<T>
{
public:
    static constexpr bool with_nodata = true;
    using nodata_type                 = std::decay_t<T>;

    raster_span(typename std::span<T>::pointer data, RasterMetadata&& meta) = delete;
    raster_span(typename std::span<T>::pointer data, const RasterMetadata& meta)
    : std::span<T>(data, meta.rows * meta.cols)
    , _meta(&meta)
    , _nodata(inf::optional_cast<nodata_type>(meta.nodata))
    {
    }

    template <class Container>
    constexpr raster_span(Container& cont, RasterMetadata&& meta) = delete;

    template <class Container>
    constexpr raster_span(Container& cont, const RasterMetadata& meta)
    : std::span<T>(cont)
    , _meta(&meta)
    , _nodata(inf::optional_cast<nodata_type>(meta.nodata))
    {
    }

    template <class Container>
    constexpr raster_span(const Container& cont, RasterMetadata&& meta) = delete;

    template <class Container>
    constexpr raster_span(const Container& cont, const RasterMetadata& meta)
    : std::span<T>(cont)
    , _meta(&meta)
    , _nodata(inf::optional_cast<nodata_type>(meta.nodata))
    {
    }

    raster_span(const raster_span<T>&) = default;
    raster_span(raster_span<T>&&)      = default;

    raster_span& operator=(const raster_span<T>&) = default;
    raster_span& operator=(raster_span<T>&&) = default;

    std::optional<nodata_type> nodata() const noexcept
    {
        return _nodata;
    }

    int32_t rows() const noexcept
    {
        return _meta->rows;
    }

    int32_t cols() const noexcept
    {
        return _meta->cols;
    }

    bool is_nodata(std::size_t index) const noexcept
    {
        if constexpr (std::numeric_limits<T>::has_quiet_NaN) {
            if (_nodata.has_value() && std::isnan(*_nodata)) {
                return std::isnan(this->operator[](index));
            }
        }
        return this->operator[](index) == _nodata;
    }

    bool is_nodata(int row, int col) const noexcept
    {
        return is_nodata(row * _meta->cols + col);
    }

    void mark_as_nodata(std::size_t index) noexcept
    {
        return this->operator[](index) = _nodata;
    }

    void mark_as_nodata(int row, int col) noexcept
    {
        return mark_as_nodata(row * _meta->cols + col);
    }

    void mark_as_data(int) noexcept
    {
    }

    void mark_as_data(int, int) noexcept
    {
    }

    auto operator()(int32_t row, int32_t col)
    {
        return this->operator[](row * _meta->cols + col);
    }

    auto operator()(int32_t row, int32_t col) const
    {
        return this->operator[](row * _meta->cols + col);
    }

    auto& operator[](size_t index) noexcept
    {
        return (std::span<T>::data())[index];
    }

    const auto& operator[](size_t index) const noexcept
    {
        return (std::span<T>::data())[index];
    }

    auto& operator[](const Cell& cell)
    {
        return (*this)[index(cell.r, cell.c)];
    }

    auto& operator[](const Cell& cell) const
    {
        return (*this)[index(cell.r, cell.c)];
    }

    const RasterMetadata& metadata() const
    {
        return *_meta;
    }

private:
    std::size_t index(int32_t row, int32_t col) const
    {
        return row * cols() + col;
    }

    const RasterMetadata* _meta = nullptr;
    std::optional<nodata_type> _nodata;
};

template <typename Container, typename T = typename Container::value_type>
auto make_raster_span(const Container& cont, RasterMetadata&& meta) = delete;

template <typename Container, typename T = typename Container::value_type>
auto make_raster_span(const Container& cont, const RasterMetadata& meta)
{
    return raster_span<const T>(cont, meta);
}

template <typename Container, typename T = typename Container::value_type>
auto make_raster_span(Container& cont, RasterMetadata&& meta) = delete;

template <typename Container, typename T = typename Container::value_type>
auto make_raster_span(Container& cont, const RasterMetadata& meta)
{
    return raster_span<T>(cont, meta);
}

template <typename T>
raster_span<T> make_raster_span(T* data, const RasterMetadata& meta)
{
    return raster_span<T>(data, meta);
}

template <typename T>
auto begin(const raster_span<T>& s)
{
    return s.begin();
}

template <typename T>
auto end(const raster_span<T>& s)
{
    return s.end();
}

template <typename T>
auto data(const raster_span<T>& s)
{
    return s.data();
}

template <typename T>
auto size(const raster_span<T>& s)
{
    return s.size();
}

template <typename T>
auto optional_value_begin(raster_span<T> rasterData)
{
    return MissingValueIterator<std::remove_const_t<T>, std::is_const_v<T>>(data(rasterData), 0, rasterData.cols(), rasterData.nodata());
}

template <typename T>
auto optional_value_end(raster_span<T> rasterData)
{
    return MissingValueIterator<std::remove_const_t<T>, std::is_const_v<T>>(data(rasterData), size(rasterData), rasterData.cols(), rasterData.nodata());
}

template <typename T>
auto value_begin(raster_span<T> rasterData)
{
    constexpr bool isConst = std::is_const_v<T>;
    using ValueFilter      = NodataValueFilterPolicy<std::remove_cv_t<T>>;

    return RasterIterator<std::remove_const_t<T>, isConst, ValueFilter>(rasterData.data(), rasterData.rows(), rasterData.cols(), ValueFilter(rasterData.nodata()), AllLocationsFilterPolicy());
}

template <typename T>
auto value_end(raster_span<T> /*rasterData*/)
{
    constexpr bool isConst = std::is_const_v<T>;
    return RasterIterator<std::remove_const_t<T>, isConst, NodataValueFilterPolicy<std::remove_cv_t<T>>>();
}

}
