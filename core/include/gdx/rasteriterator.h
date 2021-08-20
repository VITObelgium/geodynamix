#pragma once

#include "gdx/cell.h"
#include "infra/cast.h"

#include <cassert>
#include <cmath>
#include <iterator>
#include <limits>
#include <optional>

namespace gdx {

namespace detail {
template <typename T>
Cell raster_pointer_to_cell(const T* rasterTopLeft, const T* ptr, int32_t cols)
{
    std::ptrdiff_t offset = ptr - rasterTopLeft;
    auto row              = inf::truncate<int32_t>(offset / cols);
    auto col              = inf::truncate<int32_t>(offset - (row * cols));
    return Cell(row, col);
}
}

template <typename T>
class NodataValueFilterPolicy
{
public:
    NodataValueFilterPolicy() = default;
    NodataValueFilterPolicy(std::optional<T> nodata)
    : _nodata(nodata)
    {
    }

    constexpr bool exclude(const T& data) const noexcept
    {
        if constexpr (std::numeric_limits<T>::has_quiet_NaN) {
            return std::isnan(data);
        }

        return data == _nodata;
    }

    std::optional<T> proxy_construction_arg() const noexcept
    {
        return _nodata;
    }

private:
    std::optional<T> _nodata;
};

template <typename TData, typename TMask, bool is_const>
class NodataMaskFilterPolicy
{
    using pointer      = std::conditional_t<is_const, const TData*, TData*>;
    using mask_pointer = std::conditional_t<is_const, const TMask*, TMask*>;

public:
    NodataMaskFilterPolicy() = default;
    NodataMaskFilterPolicy(pointer data, mask_pointer mask)
    : _dataBegin(data)
    , _maskBegin(mask)
    {
    }

    constexpr bool exclude(const TData& data) const noexcept
    {
        std::ptrdiff_t dataOffset = &data - _dataBegin;
        return _maskBegin && *(_maskBegin + dataOffset);
    }

    mask_pointer proxy_construction_arg() noexcept
    {
        return _maskBegin;
    }

private:
    pointer _dataBegin      = nullptr;
    mask_pointer _maskBegin = nullptr;
};

template <typename T>
class AllValuesFilterPolicy
{
public:
    constexpr bool exclude(const T*) const noexcept
    {
        return false;
    }
};

class AllLocationsFilterPolicy
{
public:
    constexpr bool exclude(Cell) const noexcept
    {
        return false;
    }
};

class SingleCellLocationFilterPolicy
{
public:
    SingleCellLocationFilterPolicy() = default;
    SingleCellLocationFilterPolicy(Cell cell)
    : _cell(cell)
    {
    }

    constexpr bool exclude(Cell cell) const noexcept
    {
        return cell == _cell;
    }

private:
    Cell _cell;
};

template <typename T, bool is_const = false>
struct ValueProxy
{
    using reference     = std::conditional_t<is_const, const T&, T&>;
    using pointer       = std::conditional_t<is_const, const T*, T*>;
    using const_pointer = const T*;

    ValueProxy() = default;
    ValueProxy(pointer data, ptrdiff_t offset, int32_t stride, std::optional<T> nod) noexcept
    : _value(data + offset)
    , _nodata(nod)
    , _dataBegin(data)
    , _stride(stride)
    {
        if constexpr (std::numeric_limits<T>::has_quiet_NaN) {
            _nodata = std::numeric_limits<T>::quiet_NaN();
        }
    }

    bool operator==(const ValueProxy& other) const noexcept
    {
        return _value == other._value;
    }

    operator const T&() const noexcept
    {
        return *_value;
    }

    template <bool ReadOnly = is_const, typename = typename std::enable_if_t<ReadOnly>>
    operator T&() noexcept
    {
        return *_value;
    }

    explicit operator bool() const noexcept
    {
        return !is_nodata();
    }

    bool operator!() const noexcept
    {
        return is_nodata();
    }

    bool has_value() const noexcept
    {
        return !is_nodata();
    }

    template <bool ReadOnly = is_const>
    std::enable_if_t<!ReadOnly, T&> operator*() noexcept
    {
        return *_value;
    }

    std::conditional_t<is_const, reference, const reference> operator*() const noexcept
    {
        return *_value;
    }

    pointer operator->() const
    {
        return _value;
    }

    ValueProxy<T, is_const>& operator=(T val) noexcept
    {
        *_value = val;
        return *this;
    }

    void reset() noexcept
    {
        *_value = *_nodata;
    }

    auto& operator=(const std::optional<T>& val) noexcept
    {
        if (val.has_value()) {
            *_value = *val;
        } else if (_nodata.has_value()) {
            reset();
        } else {
            assert(false);
        }

        return *this;
    }

    bool is_nodata() const noexcept
    {
        assert(_value);

        if constexpr (std::numeric_limits<T>::has_quiet_NaN) {
            return std::isnan(*_value);
        }

        return *_value == _nodata;
    }

    void increment()
    {
        ++_value;
    }

    void increment(int32_t amount)
    {
        _value += amount;
    }

    Cell cell() const noexcept
    {
        if (_value == nullptr) {
            return Cell(-1, -1);
        }

        return detail::raster_pointer_to_cell(_dataBegin, _value, _stride);
    }

    bool is_valid() const noexcept
    {
        return _value != nullptr;
    }

    void invalidate() noexcept
    {
        _value = nullptr;
    }

private:
    pointer _value = nullptr;
    std::optional<T> _nodata;

    const_pointer _dataBegin = nullptr;
    int _stride              = 0;
};

template <typename T, bool is_const>
struct MaskValueProxy
{
    using reference     = std::conditional_t<is_const, const T&, T&>;
    using pointer       = std::conditional_t<is_const, const T*, T*>;
    using const_pointer = const T*;
    using mask_pointer  = std::conditional_t<is_const, const bool*, bool*>;

    MaskValueProxy() = default;
    MaskValueProxy(pointer data, ptrdiff_t offset, int32_t stride, mask_pointer maskData) noexcept
    : _value(data + offset)
    , _mask(maskData + offset)
    , _dataBegin(data)
    , _stride(stride)
    {
    }

    bool operator==(const MaskValueProxy& other) const noexcept
    {
        return _value == other._value;
    }

    operator const T&() const noexcept
    {
        return *_value;
    }

    template <bool ReadOnly = is_const,
        typename            = typename std::enable_if_t<ReadOnly>>
    operator T&() noexcept
    {
        return *_value;
    }

    explicit operator bool() const noexcept
    {
        return !is_nodata();
    }

    bool operator!() const noexcept
    {
        return is_nodata();
    }

    bool has_value() const noexcept
    {
        return !is_nodata();
    }

    template <bool ReadOnly = is_const>
    std::enable_if_t<!ReadOnly, T&> operator*() noexcept
    {
        return *_value;
    }

    std::conditional_t<is_const, reference, const reference> operator*() const noexcept
    {
        return *_value;
    }

    auto& operator=(T val) noexcept
    {
        *_value = val;
        if (_mask) {
            *_mask = false;
        }
        return *this;
    }

    auto& operator=(const std::optional<T>& val) noexcept
    {
        if (val.has_value()) {
            *_value = val.value();
            if (_mask) {
                *_mask = false;
            }
        } else {
            assert(_mask);
            *_mask = true;
        }
        return *this;
    }

    bool is_nodata() const noexcept
    {
        return _mask ? *_mask : false;
    }

    void reset() noexcept
    {
        assert(_mask);
        *_mask = true;
    }

    void increment()
    {
        ++_value;
        if (_mask) {
            ++_mask;
        }
    }

    void increment(int32_t amount)
    {
        _value += amount;
        if (_mask) {
            _mask += amount;
        }
    }

    Cell cell() const noexcept
    {
        if (_value == nullptr) {
            return Cell(-1, -1);
        }

        return detail::raster_pointer_to_cell(_dataBegin, _value, _stride);
    }

    bool is_valid() const noexcept
    {
        return _value != nullptr;
    }

    void invalidate() noexcept
    {
        _value = nullptr;
    }

private:
    pointer _value     = nullptr;
    mask_pointer _mask = nullptr;

    const_pointer _dataBegin = nullptr;
    int32_t _stride          = 0;
};

template <
    typename T,
    bool is_const                 = false,
    typename ValueFilterPolicy    = AllValuesFilterPolicy<T>,
    typename LocationFilterPolicy = AllLocationsFilterPolicy,
    typename ProxyType            = ValueProxy<T, is_const>>
class RasterIterator
{
public:
    using difference_type   = ptrdiff_t;
    using value_type        = std::remove_cv_t<T>;
    using value_type_ptr    = std::conditional_t<is_const, const value_type*, value_type*>;
    using reference         = std::conditional_t<is_const, const ProxyType&, ProxyType&>;
    using pointer           = std::conditional_t<is_const, const ProxyType*, ProxyType*>;
    using iterator_category = std::forward_iterator_tag;

    RasterIterator() = default;

    // Iterates the full raster
    RasterIterator(value_type_ptr data, int32_t rows, int32_t cols, ValueFilterPolicy valueFilter, LocationFilterPolicy locationFilter)
    : RasterIterator(data, rows, cols, cols, data, valueFilter, locationFilter)
    {
    }

    // Iterates a sub area of the raster
    RasterIterator(value_type_ptr data, int32_t rows, int32_t cols, int32_t stride, value_type_ptr begin, ValueFilterPolicy valueFilter, LocationFilterPolicy locationFilter)
    : _data(data)
    , _end(begin + ((rows - 1) * stride) + cols)
    , _current(data, begin - data, stride, valueFilter.proxy_construction_arg())
    , _cols(cols)
    , _stride(stride)
    , _valueFilter(valueFilter)
    , _locationFilter(locationFilter)
    {
        assert(_stride >= _cols);
        skip_values();
    }

    bool operator==(const RasterIterator& iter) const
    {
        return _current == iter._current;
    }

    bool operator!=(const RasterIterator& iter) const
    {
        return !(*this == iter);
    }

    reference operator*() noexcept
    {
        return _current;
    }

    pointer operator->() noexcept
    {
        return &_current;
    }

    RasterIterator& operator++()
    {
        increment();
        skip_values();
        return *this;
    }

    RasterIterator operator++(int)
    {
        RasterIterator<T> iter = *this;
        increment();
        skip_values();
        return iter;
    }

    [[deprecated("use cell method on the value proxy")]] Cell cell() const noexcept
    {
        return _current.cell();
    }

private:
    void increment()
    {
        if (_stride != _cols) {
            ++_currentColOffset;
            if (_currentColOffset >= _cols) {
                _current.increment(_stride - _cols);
                _currentColOffset = 0;
            }
        }

        assert(_current.is_valid());
        _current.increment();

        if (&(*_current) >= _end) {
            _current.invalidate();
        }
    }

    void skip_values()
    {
        while (_current.is_valid() && (_valueFilter.exclude(*_current) || _locationFilter.exclude(_current.cell()))) {
            increment();
        }
    }

    value_type_ptr _data = nullptr; // points to the first cell in the raster
    value_type_ptr _end  = nullptr; // points after the last cell in the raster
    ProxyType _current;             // points to the current iteration cell
    int32_t _cols             = 0;  // the number of columns in the (sub) area to iterate
    int32_t _stride           = 0;  // the number of columns in the full raster
    int32_t _currentColOffset = 0;
    ValueFilterPolicy _valueFilter;       // excludes cells based on value
    LocationFilterPolicy _locationFilter; // excludes cells based on location
};

/*! Iterator that iterates over each cell in a raster
 *  The value is a Proxy object that behaves like the raster type
 *  Accessing the value is undefined if has_value returns false
 *
 */
template <typename T, bool is_const = false, typename ProxyType = ValueProxy<T, is_const>>
class MissingValueIterator
{
public:
    using difference_type   = ptrdiff_t;
    using value_type        = std::remove_cv_t<T>;
    using value_type_ptr    = std::conditional_t<is_const, const value_type*, value_type*>;
    using reference         = std::conditional_t<is_const, const ProxyType&, ProxyType&>;
    using pointer           = std::conditional_t<is_const, const ProxyType*, ProxyType*>;
    using iterator_category = std::forward_iterator_tag;

    MissingValueIterator() = default;

    template <typename... ProxyArgs>
    MissingValueIterator(value_type_ptr data, ptrdiff_t offset, int32_t stride, ProxyArgs&&... args) noexcept
    : _current(data, offset, stride, std::forward<ProxyArgs>(args)...)
    {
    }

    bool operator==(const MissingValueIterator& other) const noexcept
    {
        return _current == other._current;
    }

    bool operator!=(const MissingValueIterator& iter) const noexcept
    {
        return !(*this == iter);
    }

    reference operator*() noexcept
    {
        return _current;
    }

    pointer operator->() noexcept
    {
        return &_current;
    }

    MissingValueIterator& operator++() noexcept
    {
        _current.increment();
        return *this;
    }

    MissingValueIterator operator++(int) noexcept
    {
        MissingValueIterator<T> iter = *this;
        _current.increment();
        return iter;
    }

    [[deprecated("use cell method on the value proxy")]] Cell cell() const noexcept
    {
        return _current.cell();
    }

private:
    ProxyType _current;
};

template <typename T>
using ValueSkippingConstIterator = RasterIterator<T, true, NodataValueFilterPolicy<T>, AllLocationsFilterPolicy>;
template <typename T>
using MissingValueConstIterator = MissingValueIterator<T, true>;

// NoData concept for a container type C
// supports skipping over containers that implement the value nodata concept
template <
    typename Container,
    typename std::enable_if_t<std::decay_t<Container>::with_nodata, int>* = nullptr>
auto value_begin(Container&& rasterData)
{
    using T                = typename std::decay_t<Container>::value_type;
    constexpr bool isConst = std::is_const_v<std::remove_reference_t<Container>>;
    using ValueFilter      = NodataValueFilterPolicy<T>;

    return RasterIterator<T, isConst, ValueFilter>(rasterData.data(), rasterData.rows(), rasterData.cols(), ValueFilter(rasterData.nodata()), AllLocationsFilterPolicy());
}

template <typename Container,
    typename std::enable_if_t<std::decay_t<Container>::with_nodata, int>* = nullptr>
auto value_end(Container&& /*rasterData*/)
{
    using T                = typename std::decay_t<Container>::value_type;
    constexpr bool isConst = std::is_const_v<std::remove_reference_t<Container>>;
    return RasterIterator<T, isConst, NodataValueFilterPolicy<T>>();
}

template <
    typename Container,
    typename std::enable_if_t<std::decay_t<Container>::with_nodata, int>* = nullptr>
auto value_cbegin(Container&& data)
{
    using T = typename std::decay_t<Container>::value_type;
    return ValueSkippingConstIterator<T>(data.data(), data.rows(), data.cols(), NodataValueFilterPolicy(data.nodata()), AllLocationsFilterPolicy());
}

template <typename Container,
    typename std::enable_if_t<std::decay_t<Container>::with_nodata, int>* = nullptr>
auto value_cend(const Container& /*rasterData*/)
{
    using T = typename std::decay_t<Container>::value_type;
    return ValueSkippingConstIterator<T>();
}

template <
    typename Container,
    typename std::enable_if_t<std::decay_t<Container>::with_nodata, int>* = nullptr>
auto optional_value_begin(Container& rasterData)
{
    using T = typename std::decay_t<Container>::value_type;
    return MissingValueIterator<T, false>(data(rasterData), 0, rasterData.cols(), rasterData.nodata());
}

template <
    typename Container,
    typename std::enable_if_t<std::decay_t<Container>::with_nodata, int>* = nullptr>
auto optional_value_begin(const Container& rasterData)
{
    using T = typename std::decay_t<Container>::value_type;
    return MissingValueIterator<T, true>(data(rasterData), 0, rasterData.cols(), rasterData.nodata());
}

template <typename Container,
    typename std::enable_if_t<std::decay_t<Container>::with_nodata, int>* = nullptr>
auto optional_value_end(Container&& rasterData)
{
    using T                = typename std::decay_t<Container>::value_type;
    constexpr bool isConst = std::is_const_v<std::remove_reference_t<Container>>;
    return MissingValueIterator<T, isConst>(data(rasterData), size(rasterData), rasterData.cols(), rasterData.nodata());
}

class CellIterator
{
public:
    using difference_type   = int32_t;
    using value_type        = Cell;
    using pointer           = value_type*;
    using reference         = value_type&;
    using iterator_category = std::forward_iterator_tag;

    constexpr CellIterator() = default;
    constexpr CellIterator(int32_t rows, int32_t cols) noexcept
    : _current(0, 0)
    , _rows(rows)
    , _cols(cols)
    {
    }

    constexpr bool operator==(const CellIterator& iter) const noexcept
    {
        return _current == iter._current;
    }

    constexpr bool operator!=(const CellIterator& iter) const noexcept
    {
        return !(*this == iter);
    }

    constexpr const value_type& operator*() const noexcept
    {
        return _current;
    }

    constexpr const value_type* operator->() const noexcept
    {
        return &_current;
    }

    constexpr CellIterator& operator++() noexcept
    {
        increment();
        return *this;
    }

    constexpr CellIterator operator++(int) noexcept
    {
        auto iter = *this;
        increment();
        return iter;
    }

private:
    constexpr void increment() noexcept
    {
        assert(_current.is_valid());

        ++_current.c;
        if (_current.c == _cols) {
            _current.c = 0;
            ++_current.r;
        }

        if (_current.r == _rows) {
            _current = Cell();
        }
    }

    Cell _current;
    int32_t _rows = 0;
    int32_t _cols = 0;
};

class RasterCells
{
public:
    RasterCells(int32_t rows, int32_t cols) noexcept
    : _rows(rows)
    , _cols(cols)
    {
    }

    template <typename Container>
    RasterCells(const Container& cont) noexcept
    : _rows(cont.rows())
    , _cols(cont.cols())
    {
    }

    auto begin() const noexcept
    {
        return CellIterator(_rows, _cols);
    }

    auto end() const noexcept
    {
        return CellIterator();
    }

private:
    int32_t _rows = 0;
    int32_t _cols = 0;
};

template <typename Container>
auto cell_begin(Container&& data)
{
    return CellIterator(data.rows(), data.cols());
}

template <typename Container>
auto cell_end(Container&&)
{
    return CellIterator();
}

template <typename Container>
auto cell_cbegin(Container&& data)
{
    return CellIterator(data.rows(), data.cols());
}

template <typename Container>
auto cell_cend(const Container&)
{
    return CellIterator();
}

}
