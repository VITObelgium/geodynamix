#pragma once

#include "gdx/cell.h"
#include "gdx/cpupredicates-private.h"
#include "gdx/exception.h"
#include "gdx/nodatapredicates-private.h"
#include "gdx/rasterchecks.h"
#include "gdx/rasteriterator.h"
#include "gdx/rastermetadata.h"
#include "gdx/simd.h"
#include "infra/cast.h"
#include "infra/span.h"
#include "infra/string.h"
#include "rasterutils-private.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244 4242 4127 4005)
#endif
#include <Vc/Allocator>
#include <Vc/common/simdize.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <algorithm>
#include <cassert>
#include <type_traits>
#include <vector>

namespace gdx {

template <typename T>
class DenseRaster
{
public:
    using value_type                          = T;
    using size_type                           = std::size_t;
    using data_type                           = std::vector<T, Vc::Allocator<T>>;
    using nodata_type                         = std::optional<value_type>;
    using pointer                             = T*;
    using const_pointer                       = const T*;
    using iterator                            = pointer;
    using const_iterator                      = const_pointer;
    static constexpr bool raster_type_has_nan = std::numeric_limits<T>::has_quiet_NaN;
    static constexpr bool with_nodata         = true;
    static constexpr T NaN                    = std::numeric_limits<T>::quiet_NaN();

    static constexpr bool has_nan()
    {
        return raster_type_has_nan;
    }

    static constexpr bool simd_supported()
    {
        return !(std::is_same_v<uint8_t, T> || std::is_same_v<int64_t, T> || std::is_same_v<uint64_t, T>);
    }

    DenseRaster() = default;

    DenseRaster(int32_t rows, int32_t cols)
    : _meta(rows, cols)
    , _data(rows * cols)
    {
    }

    explicit DenseRaster(RasterMetadata meta)
    : _meta(std::move(meta))
    , _data(size_t(_meta.rows) * size_t(_meta.cols))
    {
        init_nodata_values();
    }

    DenseRaster(int32_t rows, int32_t cols, T fillValue)
    : DenseRaster(RasterMetadata(rows, cols), fillValue)
    {
    }

    DenseRaster(const RasterMetadata& meta, T fillValue)
    : _meta(meta)
    , _data(size_t(_meta.rows) * size_t(_meta.cols))
    {
        if constexpr (raster_type_has_nan) {
            // make sure we fill tha raster with NaNs if the fill value is the nodata value
            if (_meta.nodata.has_value() && fillValue == static_cast<T>(*_meta.nodata)) {
                fillValue = NaN;
            }
        }

        fill(fillValue);
    }

    DenseRaster(int32_t rows, int32_t cols, std::span<const T> data)
    : DenseRaster(RasterMetadata(rows, cols), data)
    {
    }

    DenseRaster(const RasterMetadata& meta, std::span<const T> data)
    : _meta(meta)
    , _data(size_t(_meta.rows) * size_t(_meta.cols))
    {
        throw_on_datasize_mismatch(meta.rows, meta.cols, data.size());
        std::copy(data.begin(), data.end(), _data.data());
        init_nodata_values();
    }

    DenseRaster(const RasterMetadata& meta, data_type&& data)
    : _meta(meta)
    , _data(data)
    {
        throw_on_datasize_mismatch(meta.rows, meta.cols, data.size());
        init_nodata_values();
    }

    DenseRaster(DenseRaster<T>&&) noexcept   = default;
    DenseRaster(const DenseRaster<T>& other) = delete;

    DenseRaster& operator=(DenseRaster<T>&&)            = default;
    DenseRaster& operator=(const DenseRaster<T>& other) = delete;

    void resize_and_fill(int32_t rows, int32_t cols, value_type value)
    {
        resize(rows, cols);
        fill(value);
    }

    void resize(int32_t rows, int32_t cols)
    {
        _meta.rows = rows;
        _meta.cols = cols;
        _data.resize(size_t(rows) * size_t(cols));
    }

    void resize(int32_t rows, int32_t cols, std::optional<double> nodata)
    {
        resize(rows, cols);
        _meta.nodata = nodata;
    }

    void set_metadata(RasterMetadata meta)
    {
        if (int64_t(meta.rows) * int64_t(meta.cols) != ssize()) {
            throw InvalidArgument("Cannot change metadata: invalid size");
        }

        _meta = std::move(meta);
    }

    DenseRaster<T> copy() const
    {
        DenseRaster<T> dst(_meta);
        dst._data = _data;
        return dst;
    }

    auto begin()
    {
        return _data.begin();
    }

    auto begin() const
    {
        return cbegin();
    }

    auto cbegin() const
    {
        return _data.cbegin();
    }

    auto end()
    {
        return _data.end();
    }

    auto end() const
    {
        return cend();
    }

    auto cend() const
    {
        return _data.cend();
    }

    const value_type* data() const noexcept
    {
        return _data.data();
    }

    value_type* data() noexcept
    {
        return _data.data();
    }

    bool has_nodata() const noexcept
    {
        if (_meta.nodata.has_value()) {
            if constexpr (raster_type_has_nan) {
                return std::any_of(begin(), end(), [](T value) { return std::isnan(value); });
            } else {
                return std::any_of(begin(), end(), [nod = static_cast<T>(*_meta.nodata)](T value) { return value == nod; });
            }
        }

        return false;
    }

    std::optional<T> nodata() const noexcept
    {
        return inf::optional_cast<T>(_meta.nodata);
    }

    std::size_t size() const noexcept
    {
        return _data.size();
    }

    std::ptrdiff_t ssize() const noexcept
    {
        assert(_data.size() <= std::size_t(std::numeric_limits<std::ptrdiff_t>::max()));
        return static_cast<std::ptrdiff_t>(_data.size());
    }

    bool empty() const noexcept
    {
        return _data.size() == 0;
    }

    void collapse_data()
    {
        // no collapse needed for non floating point types
        if constexpr (has_nan()) {
            if (auto nod = nodata(); nod.has_value() && !std::isnan(*nod)) {
                if constexpr (simd_supported()) {
                    simd::for_each(begin(), end(), [nodata = *nod](auto& value) {
                        value(std::isnan(value)) = nodata;
                    });
                } else {
                    std::transform(begin(), end(), begin(), [nodata = *nod](T value) {
                        return std::isnan(value) ? nodata : value;
                    });
                }
            }
        }
    }

    const RasterMetadata& metadata() const noexcept
    {
        return _meta;
    }

    void set_projection(int32_t epsg)
    {
        _meta.set_projection_from_epsg(epsg);
    }

    void clear_projection()
    {
        _meta.projection.clear();
    }

    void set_nodata(double newValue)
    {
        if constexpr (!raster_type_has_nan) {
            if (std::isnan(newValue)) {
                throw InvalidArgument("Nodata value cannot be NaN for integral rasters");
            }
        }

        _meta.nodata = newValue;
    }

    void replace_nodata(T newValue)
    {
        const auto dataSize = _data.size();
        for (std::size_t i = 0; i < dataSize; ++i) {
            if (is_nodata(i)) {
                _data[i] = newValue;
            }
        }

        _meta.nodata.reset();
    }

    void turn_value_into_nodata(T value)
    {
        const auto dataSize = _data.size();
        for (std::size_t i = 0; i < dataSize; ++i) {
            if (_data[i] == value) {
                mark_as_nodata(i);
            }
        }
    }

    // assigns the value to all the elements of the raster, even nodata
    void fill(value_type value)
    {
        std::fill(_data.begin(), _data.end(), value);
    }

    // assigns the value to all the elements of the raster, leaving nodata values intact
    void fill_values(value_type value)
    {
        if (auto nod = nodata(); nod.has_value()) {
            if constexpr (simd_supported()) {
                simd::for_each(_data.begin(), _data.end(), [value, nod = *nod](auto& v) {
                    v(v != nod) = value;
                });
            } else {
                std::for_each(begin(), end(), [this, value](auto& v) {
                    if (!is_nodata_value(v)) {
                        v = value;
                    }
                });
            }
        } else {
            return fill(value);
        }
    }

    // Makes all elements of the raster nodata values
    void fill_with_nodata()
    {
        if (_meta.nodata.has_value()) {
            if constexpr (raster_type_has_nan) {
                fill(NaN);
            } else {
                fill(static_cast<T>(*_meta.nodata));
            }
        }
    }

    bool contains_only_nodata() const noexcept
    {
        return nodata().has_value() && std::all_of(_data.begin(), _data.end(), [this](const auto val) {
                   return is_nodata_value(val);
               });
    }

    int32_t rows() const noexcept
    {
        return _meta.rows;
    }

    int32_t cols() const noexcept
    {
        return _meta.cols;
    }

    void mark_as_data(std::size_t /*index*/) noexcept
    {
    }

    void mark_as_data(Cell /*cell*/) noexcept
    {
    }

    void mark_as_data(int32_t /*row*/, int32_t /*col*/) noexcept
    {
    }

    void mark_as_nodata(std::size_t index)
    {
        if (!_meta.nodata.has_value()) {
            throw RuntimeError("mark_as_nodata called without nodata defined");
        }

        if constexpr (raster_type_has_nan) {
            _data[index] = NaN;
        } else {
            _data[index] = static_cast<T>(*_meta.nodata);
        }
    }

    void mark_as_nodata(int32_t row, int32_t col)
    {
        mark_as_nodata(index(row, col));
    }

    void mark_as_nodata(Cell cell)
    {
        mark_as_nodata(cell.r, cell.c);
    }

    std::optional<value_type> optional_value(std::size_t index) const noexcept
    {
        if (is_nodata(index)) {
            return std::optional<value_type>();
        } else {
            return _data[index];
        }
    }

    template <typename VarType>
    std::optional<VarType> optional_value_as(std::size_t index) const noexcept
    {
        if (is_nodata(index)) {
            return std::optional<VarType>();
        } else {
            return static_cast<VarType>(_data[index]);
        }
    }

    bool is_nodata_value(T value) const noexcept
    {
        if constexpr (raster_type_has_nan) {
            return std::isnan(value);
        } else {
            if (_meta.nodata.has_value()) {
                return value == *_meta.nodata;
            } else {
                return false;
            }
        }
    }

    bool is_nodata(std::size_t index) const noexcept
    {
        if (_meta.nodata.has_value()) {
            if constexpr (raster_type_has_nan) {
                return std::isnan(_data[index]);
            } else {
                return _data[index] == static_cast<T>(*_meta.nodata);
            }
        }

        return false;
    }

    bool is_nodata(const Cell& cell) const noexcept
    {
        return is_nodata(cell.r, cell.c);
    }

    bool is_nodata(int32_t r, int32_t c) const noexcept
    {
        if (_meta.nodata.has_value()) {
            if constexpr (raster_type_has_nan) {
                return std::isnan(_data[index(r, c)]);
            } else {
                return _data[index(r, c)] == static_cast<T>(*_meta.nodata);
            }
        }

        return false;
    }

    bool tolerant_equal_to(const DenseRaster<T>& other, value_type tolerance = std::numeric_limits<value_type>::epsilon()) const noexcept
    {
        if (_meta != other._meta) {
            return false;
        }

        return tolerant_data_equal_to(other, tolerance);
    }

    bool tolerant_data_equal_to(const DenseRaster<T>& other, value_type relTolerance = value_type(1e-05)) const noexcept
    {
        throw_on_size_mismatch(*this, other);

        cpu::float_equal_to<T> comp(relTolerance);

        const auto dataSize = size();
        for (std::size_t i = 0; i < dataSize; ++i) {
            if (is_nodata(i) != other.is_nodata(i)) {
                return false;
            }

            if (!is_nodata(i) && !comp(_data[i], other[i])) {
                return false;
            }
        }

        return true;
    }

    /* Add the value to the cell, if the cell is nodata it will become data with the provided value */
    void add_to_cell(Cell c, T value)
    {
        if (is_nodata(c)) {
            (*this)[c] = value;
        } else {
            (*this)[c] += value;
        }
    }

    bool operator==(const DenseRaster<T>& other) const noexcept
    {
        throw_on_size_mismatch(*this, other);

        const auto dataSize = size();
        for (std::size_t i = 0; i < dataSize; ++i) {
            if (is_nodata(i) != other.is_nodata(i)) {
                return false;
            }

            if (!is_nodata(i) && (_data[i] != other[i])) {
                return false;
            }
        }

        return true;
    }

    bool operator!=(const DenseRaster<T>& other) const noexcept
    {
        return !(*this == other);
    }

    DenseRaster<uint8_t> not_equals(const DenseRaster<T>& other) const noexcept
    {
        throw_on_size_mismatch(*this, other);
        return perform_binary_operation<nodata::not_equal_to>(other);
    }

    template <typename TValue>
    DenseRaster<uint8_t> not_equals(TValue value) const
    {
        static_assert(std::is_scalar_v<TValue>, "Arithmetic operation called with non scalar type");
        return perform_unary_operation<nodata::not_equal_to>(value);
    }

    template <typename TOther>
    auto operator+(const DenseRaster<TOther>& other) const
    {
        return perform_raster_operation<std::plus<>>(other);
    }

    template <typename TValue>
    auto operator+(TValue value) const
    {
        static_assert(std::is_scalar_v<TValue>, "Arithmetic operation called with non scalar type");
        return perform_scalar_operation<std::plus<>>(value);
    }

    template <typename TValue>
    DenseRaster<T>& operator+=(TValue value)
    {
        return perform_scalar_operation_inplace<std::plus<>>(value);
    }

    //! Add values of the other raster to this raster
    // - Nodata values of this raster will not be assigned
    // - Nodata values of the other raster will become nodata in the result
    template <typename TOther>
    DenseRaster<T>& operator+=(const DenseRaster<TOther>& other)
    {
        return perform_raster_operation_in_place<std::plus<>>(other);
    }

    //! Add values of the other raster to this raster
    // - Nodata values of this raster will become the value in the other raster
    // - Nodata values of the other raster will become nodata in the result
    template <typename TOther>
    DenseRaster<T>& add_or_assign(const DenseRaster<TOther>& other)
    {
        throw_on_size_mismatch(*this, other);

        if constexpr (simd_supported() && has_nan() && DenseRaster<TOther>::has_nan() && sizeof(T) == sizeof(TOther)) {
            simd::transform(cbegin(), cend(), other.cbegin(), begin(), [](auto& v1, auto& v2) {
                auto w                              = Vc::simd_cast<Vc::Vector<T, typename std::remove_reference_t<decltype(v1)>::abi>>(v2);
                auto out                            = v1;
                out(Vc::isnan(v1) && !Vc::isnan(w)) = T(0);
                out(!Vc::isnan(w)) += w;
                return out;
            });
        } else {
            const auto dataSize = size();
            for (std::size_t i = 0; i < dataSize; ++i) {
                if (other.is_nodata(i)) {
                    continue;
                }

                if (is_nodata(i)) {
                    _data[i] = static_cast<T>(other[i]);
                } else {
                    _data[i] += static_cast<T>(other[i]);
                }
            }
        }

        return *this;
    }

    template <typename TOther>
    DenseRaster<T>& add_or_assign(TOther value)
    {
        static_assert(std::is_scalar_v<TOther>, "add_or_assign has to be called with a scalar value");

        const auto val = static_cast<T>(value);

        if constexpr (simd_supported() && has_nan()) {
            simd::transform(cbegin(), cend(), begin(), [val](auto& v) {
                auto out          = v;
                out(Vc::isnan(v)) = val;
                out(!Vc::isnan(v)) += val;
                return out;
            });
        } else {
            const auto dataSize = size();
            for (std::size_t i = 0; i < dataSize; ++i) {
                if (is_nodata(i)) {
                    _data[i] = val;
                } else {
                    _data[i] += val;
                }
            }
        }

        return *this;
    }

    DenseRaster<T>
    operator-() const
    {
        if constexpr (std::is_unsigned_v<T>) {
            throw RuntimeError("Minus operator applied to unsigned value");
        } else {
            DenseRaster<T> result(_meta, DenseRaster<T>::data_type(_data));
            std::transform(result.begin(), result.end(), result.begin(), nodata::negate<T>(_meta.nodata));
            return result;
        }
    }

    template <typename TOther>
    auto operator-(const DenseRaster<TOther>& other) const
    {
        return perform_raster_operation<std::minus<>>(other);
    }

    template <typename TValue>
    auto operator-(TValue value) const
    {
        static_assert(std::is_scalar_v<TValue>, "Arithmetic operation called with non scalar type");
        return perform_scalar_operation<std::minus<>>(value);
    }

    template <typename TValue>
    DenseRaster<T>& operator-=(TValue value)
    {
        return perform_scalar_operation_inplace<std::minus<>>(value);
    }

    template <typename TOther>
    DenseRaster<T>& operator-=(const DenseRaster<TOther>& other)
    {
        return perform_raster_operation_in_place<std::minus<>>(other);
    }

    template <typename TOther>
    auto operator*(const DenseRaster<TOther>& other) const
    {
        return perform_raster_operation<std::multiplies<>>(other);
    }

    template <typename TValue>
    auto operator*(TValue value) const
    {
        static_assert(std::is_scalar_v<TValue>, "Arithmetic operation called with non scalar type");
        return perform_scalar_operation<std::multiplies<>>(value);
    }

    template <typename TValue>
    DenseRaster<T>& operator*=(TValue value)
    {
        return perform_scalar_operation_inplace<std::multiplies<>>(value);
    }

    template <typename TOther>
    DenseRaster<T>& operator*=(const DenseRaster<TOther>& other)
    {
        return perform_raster_operation_in_place<std::multiplies<>>(other);
    }

    template <typename TOther>
    auto operator/(const DenseRaster<TOther>& other) const
    {
        return perform_raster_operation<std::divides<>>(other);
    }

    template <typename TValue>
    auto operator/(TValue value) const
    {
        static_assert(std::is_scalar_v<TValue>, "Arithmetic operation called with non scalar type");

        if (value == 0) {
            throw InvalidArgument("Division by zero");
        }

        return perform_scalar_operation<std::divides<>>(value);
    }

    template <typename TValue>
    DenseRaster<T>& operator/=(TValue value)
    {
        return perform_scalar_operation_inplace<std::divides<>>(value);
    }

    template <typename TOther>
    DenseRaster<T>& operator/=(const DenseRaster<TOther>& other)
    {
        return perform_raster_operation_in_place<std::divides<>>(other);
    }

    value_type& operator[](std::size_t index)
    {
        return _data[index];
    }

    value_type operator[](std::size_t index) const
    {
        return _data[index];
    }

    value_type& operator[](const Cell& cell)
    {
        return _data[index(cell.r, cell.c)];
    }

    const value_type& operator[](const Cell& cell) const
    {
        return _data[index(cell.r, cell.c)];
    }

    value_type& operator()(int32_t row, int32_t col)
    {
        return _data[index(row, col)];
    }

    const value_type& operator()(int32_t row, int32_t col) const
    {
        return _data[index(row, col)];
    }

    DenseRaster<uint8_t> operator!() const
    {
        return perform_unary_operation<nodata::logical_not>();
    }

    template <typename TOther>
    DenseRaster<uint8_t> operator&&(const DenseRaster<TOther>& other) const
    {
        return perform_binary_operation<nodata::logical_and>(other);
    }

    template <typename TOther>
    DenseRaster<uint8_t> operator||(const DenseRaster<TOther>& other) const
    {
        return perform_binary_operation<nodata::logical_or>(other);
    }

    template <typename TOther>
    DenseRaster<uint8_t> operator>(const DenseRaster<TOther>& other) const
    {
        return perform_binary_operation<nodata::greater>(other);
    }

    DenseRaster<uint8_t> operator>(T threshold) const
    {
        return perform_unary_operation<nodata::greater>(threshold);
    }

    template <typename TOther>
    DenseRaster<uint8_t> operator>=(const DenseRaster<TOther>& other) const
    {
        return perform_binary_operation<nodata::greater_equal>(other);
    }

    DenseRaster<uint8_t> operator>=(T threshold) const
    {
        return perform_unary_operation<nodata::greater_equal>(threshold);
    }

    template <typename TOther>
    DenseRaster<uint8_t> operator<(const DenseRaster<TOther>& other) const
    {
        return perform_binary_operation<nodata::less>(other);
    }

    DenseRaster<uint8_t> operator<(T threshold) const
    {
        return perform_unary_operation<nodata::less>(threshold);
    }

    template <typename TOther>
    DenseRaster<uint8_t> operator<=(const DenseRaster<TOther>& other) const
    {
        return perform_binary_operation<nodata::less_equal>(other);
    }

    DenseRaster<uint8_t> operator<=(T threshold) const
    {
        return perform_unary_operation<nodata::less_equal>(threshold);
    }

    void replace(T oldValue, T newValue) noexcept
    {
        std::replace(begin(), end(), oldValue, newValue);
    }

    std::string to_string() const
    {
        if constexpr (std::is_same_v<uint8_t, T>) {
            DenseRaster<uint16_t> copy(_meta);
            std::copy(begin(), end(), copy.begin());
            return copy.to_string();
        } else {
            std::stringstream ss;
            for (int i = 0; i < rows(); ++i) {
                std::span<const T> row(&_data[size_t(i) * cols()], cols());
                ss << inf::str::join(row, ", ") << "\n";
            }

            return ss.str();
        }
    }

    void init_nodata_values()
    {
        if constexpr (raster_type_has_nan) {
            if (auto nodataOpt = nodata(); nodataOpt.has_value() && !std::isnan(*nodataOpt)) {
                simd::for_each(begin(), end(), [nod = *nodataOpt](auto& v) {
                    v(v == nod) = NaN;
                });
            }
        }
    }

    template <typename TResult = T>
    TResult sum() const
    {
        auto result = TResult(0);

        if (!nodata().has_value()) {
            simd::for_each(begin(), end(), [&result](const auto& v) {
                result += v.sum();
            });
        } else {
            if constexpr (raster_type_has_nan) {
                simd::for_each(begin(), end(), [&result](const auto& v) {
                    result += v.sum(!Vc::isnan(v));
                });
            } else {
                simd::for_each(begin(), end(), [&result, nod = *nodata()](const auto& v) {
                    result += v.sum(v != nod);
                });
            }
        }

        return result;
    }

private:
    std::size_t index(int32_t row, int32_t col) const
    {
        return row * cols() + col;
    }

    static void throw_on_datasize_mismatch(int32_t rows, int32_t cols, size_t dataSize)
    {
        if (static_cast<size_t>(size_t(rows) * cols) != dataSize) {
            throw InvalidArgument("Raster data size does not match provided dimensions {} vs {}x{}", dataSize, rows, cols);
        }
    }

    template <typename T1, typename T2>
    static constexpr bool floating_point_simd_supported()
    {
        return DenseRaster<T1>::simd_supported() && DenseRaster<T2>::simd_supported() &&
               DenseRaster<T1>::has_nan() && DenseRaster<T2>::has_nan() && sizeof(T1) == sizeof(T2);
    }

    template <typename T1, typename T2>
    static constexpr bool integral_simd_supported()
    {
        return DenseRaster<T1>::simd_supported() && DenseRaster<T2>::simd_supported() &&
               !DenseRaster<T1>::has_nan() && !DenseRaster<T2>::has_nan() && sizeof(T1) == sizeof(T2);
    }

    template <typename BinaryPredicate, typename TOther, typename TResult>
    void fp_simd_raster_operation(const DenseRaster<TOther>& other, DenseRaster<TResult>& result) const
    {
        static_assert(simd_supported() && DenseRaster<TOther>::simd_supported() && DenseRaster<TResult>::simd_supported(), "simd operation called with non supporting types");
        static_assert(has_nan() && DenseRaster<TOther>::has_nan() && DenseRaster<TResult>::has_nan(), "floating point simd operation called with non floating point types");
        using IsDivision = std::conditional_t<std::is_same_v<BinaryPredicate, std::divides<>>, std::true_type, std::false_type>;

        simd::transform(begin(), end(), other.begin(), result.begin(), [](const auto& v1, const auto& v2) {
            auto w1  = Vc::simd_cast<Vc::Vector<TResult, typename std::decay_t<decltype(v1)>::abi>>(v1);
            auto w2  = Vc::simd_cast<Vc::Vector<TResult, typename std::decay_t<decltype(v2)>::abi>>(v2);
            auto res = BinaryPredicate()(w1, w2);
            if constexpr (IsDivision::value) {
                res(w2 == 0) = DenseRaster<TResult>::NaN;
            }

            return res;
        });
    }

    template <typename BinaryPredicate, typename TOther, typename TResult>
    void int_simd_raster_operation(const DenseRaster<TOther>& other, DenseRaster<TResult>& result) const
    {
        static_assert(simd_supported() && DenseRaster<TOther>::simd_supported() && DenseRaster<TResult>::simd_supported(), "simd operation called with non supporting types");
        static_assert(!has_nan() && !DenseRaster<TOther>::has_nan(), "integral simd operation called with non integral types");
        using IsDivision = std::conditional_t<std::is_same_v<BinaryPredicate, std::divides<>>, std::true_type, std::false_type>;

        if (!nodata().has_value() || !other.nodata().has_value()) {
            // fallback to non simd implementation for other combinations
            fallback_raster_operation<BinaryPredicate>(other, result);
            return;
        }

        // when result has nan, the nodata value should also be nan (this is only the case for divisions)
        if constexpr (DenseRaster<TResult>::has_nan()) {
            static_assert(IsDivision::value);
            assert(std::isnan(result.nodata().value()));
        }

        simd::transform(begin(), end(), other.begin(), result.begin(), [nod = result.nodata().value(), nod1 = nodata().value(), nod2 = other.nodata().value()](const auto& v1, const auto& v2) {
            auto w1 = Vc::simd_cast<Vc::Vector<TResult, typename std::decay_t<decltype(v1)>::abi>>(v1);
            auto w2 = Vc::simd_cast<Vc::Vector<TResult, typename std::decay_t<decltype(v2)>::abi>>(v2);

            if constexpr (IsDivision::value) {
                auto mask = w2 == 0;
                w2(mask)  = 1;

                auto out                              = BinaryPredicate()(w1, w2);
                out(w1 == nod1 || w2 == nod2 || mask) = nod;
                return out;
            } else {
                auto out                      = BinaryPredicate()(w1, w2);
                out(w1 == nod1 || w2 == nod2) = nod;
                return out;
            }
        });
    }

    template <typename BinaryPredicate, typename TOther, typename TResult>
    void simd_raster_operation(const DenseRaster<TOther>& other, DenseRaster<TResult>& result) const
    {
        if constexpr (floating_point_simd_supported<T, TOther>()) {
            fp_simd_raster_operation<BinaryPredicate>(other, result);
        } else if constexpr (integral_simd_supported<T, TOther>()) {
            int_simd_raster_operation<BinaryPredicate>(other, result);
        } else {
            fallback_raster_operation<BinaryPredicate>(other, result);
        }
    }

    template <typename BinaryPredicate, typename TOther, typename TResult>
    auto fallback_raster_operation(const DenseRaster<TOther>& other, DenseRaster<TResult>& result) const
    {
        using IsDivision = std::conditional_t<std::is_same_v<BinaryPredicate, std::divides<>>, std::true_type, std::false_type>;
        if (result.nodata().has_value()) {
            auto nod = result.nodata().value();
            if constexpr (DenseRaster<TResult>::has_nan()) {
                nod = DenseRaster<TResult>::NaN;
            }

            for (std::size_t i = 0; i < size(); ++i) {
                if (is_nodata(i) || other.is_nodata(i)) {
                    result[i] = nod;
                } else {
                    if constexpr (IsDivision::value) {
                        if (other[i] == 0) {
                            result.mark_as_nodata(i);
                            continue;
                        }
                    }

                    result[i] = BinaryPredicate()(static_cast<TResult>(_data[i]), static_cast<TResult>(other[i]));
                }
            }
        } else {
            assert(!IsDivision::value);
            assert(!nodata().has_value() && !other.nodata().has_value());
            // the result does not have nodata this means the input rasters also do not have nodata
            std::transform(cbegin(), cend(), other.cbegin(), result.begin(), [](auto& v1, auto& v2) {
                return BinaryPredicate()(static_cast<TResult>(v1), static_cast<TResult>(v2));
            });
        }
    }

    template <typename BinaryPredicate, typename TOther, typename TResult>
    void raster_operation(const DenseRaster<TOther>& other, DenseRaster<TResult>& result) const
    {
        constexpr bool simdSupported = simd_supported() && DenseRaster<TOther>::simd_supported();
        if constexpr (simdSupported) {
            simd_raster_operation<BinaryPredicate>(other, result);
        } else {
            // fallback to non simd implementation for other combinations
            fallback_raster_operation<BinaryPredicate>(other, result);
        }
    }

    // Performs a unary operation on all the elements that results in true or false
    template <template <typename> typename BinaryPredicate, typename TOther>
    DenseRaster<uint8_t> perform_unary_operation(TOther value) const
    {
        DenseRaster<uint8_t> result(_meta);
        if (_meta.nodata.has_value()) {
            result.set_nodata(static_cast<double>(std::numeric_limits<uint8_t>::max()));
        }

        auto pred       = BinaryPredicate<T>(_meta.nodata, std::optional<double>());
        const auto size = result.size();
#pragma omp parallel for
        for (std::size_t i = 0; i < size; ++i) {
            result[i] = pred(_data[i], static_cast<T>(value));
        }
        return result;
    }

    template <template <typename> typename UnaryPredicate>
    DenseRaster<uint8_t> perform_unary_operation() const
    {
        DenseRaster<uint8_t> result(_meta);
        if (_meta.nodata) {
            result.set_nodata(static_cast<double>(std::numeric_limits<uint8_t>::max()));
        }

        std::transform(cbegin(), cend(), result.begin(), UnaryPredicate<T>(_meta.nodata));
        return result;
    }

    template <template <typename> typename BinaryPredicate, typename TOther>
    DenseRaster<uint8_t> perform_binary_operation(const DenseRaster<TOther>& other) const
    {
        throw_on_size_mismatch(*this, other);
        using WidestType = decltype(T() * TOther());

        DenseRaster<uint8_t> result(_meta);
        if (_meta.nodata.has_value() || other.metadata().nodata.has_value()) {
            result.set_nodata(std::numeric_limits<uint8_t>::max());
        }

        auto pred       = BinaryPredicate<WidestType>(_meta.nodata, other.metadata().nodata);
        const auto size = result.size();
#pragma omp parallel for
        for (std::size_t i = 0; i < size; ++i) {
            result[i] = pred(static_cast<WidestType>(_data[i]), static_cast<WidestType>(other[i]));
        }
        return result;
    }

    template <typename BinaryPredicate, typename TScalar>
    auto perform_scalar_operation(TScalar scalar) const
    {
        using ResultType = decltype(BinaryPredicate()(T(), TScalar()));
        DenseRaster<ResultType> result(_meta);

        if constexpr (!simd_supported() || sizeof(ResultType) != sizeof(T)) {
            std::transform(begin(), end(), result.begin(), [this, scalar](T value) {
                if (is_nodata_value(value)) {
                    return static_cast<ResultType>(value);
                }

                return BinaryPredicate()(value, scalar);
            });
        } else if (has_nan() || !nodata().has_value()) {
            simd::transform(begin(), end(), result.begin(), [scalar](auto v) {
                using ResultVectorType = Vc::Vector<ResultType, typename decltype(v)::abi>;
                return BinaryPredicate()(Vc::simd_cast<ResultVectorType>(v), scalar);
            });
        } else {
            assert(nodata().has_value());
            simd::transform(begin(), end(), result.begin(), [scalar, nod = *nodata()](auto v) {
                using ResultVectorType = Vc::Vector<ResultType, typename decltype(v)::abi>;
                auto w                 = Vc::simd_cast<ResultVectorType>(v);
                auto out               = BinaryPredicate()(w, scalar);
                out(w == nod)          = nod;
                return out;
            });
        }

        return result;
    }

    template <typename BinaryPredicate, typename TScalar>
    DenseRaster<T>& perform_scalar_operation_inplace(TScalar scalar)
    {
        static_assert(std::is_scalar_v<TScalar>, "Arithmetic operation called with non scalar type");

        if constexpr (!simd_supported()) {
            std::for_each(begin(), end(), [this, scalar](T& value) {
                if (is_nodata_value(value)) {
                    return;
                }

                value = BinaryPredicate()(value, scalar);
            });
        } else if (has_nan() || !nodata().has_value()) {
            simd::for_each(begin(), end(), [scalar](auto& value) {
                value = BinaryPredicate()(value, scalar);
            });
        } else {
            assert(nodata().has_value());
            simd::for_each(begin(), end(), [scalar, nod = *nodata()](auto& value) {
                value(value != nod) = BinaryPredicate()(value, scalar);
            });
        }

        return *this;
    }

    template <typename BinaryPredicate, typename TOther>
    DenseRaster<T>& perform_raster_operation_in_place(const DenseRaster<TOther>& other)
    {
        throw_on_size_mismatch(*this, other);
        detail::assign_nodata_value(*this, other, *this);

        // Division is special: divide by zero becomes nodata
        using IsDivision = std::conditional_t<std::is_same_v<BinaryPredicate, std::divides<>>, std::true_type, std::false_type>;
        if constexpr (IsDivision::value) {
            if (!_meta.nodata.has_value()) {
                _meta.nodata = detail::nodata_for_type<T>();
            }
        }

        raster_operation<BinaryPredicate>(other, *this);
        return *this;
    }

    template <typename BinaryPredicate, typename TOther>
    auto perform_raster_operation(const DenseRaster<TOther>& other) const
    {
        throw_on_size_mismatch(*this, other);

        using IsDivision = std::conditional_t<std::is_same_v<BinaryPredicate, std::divides<>>, std::true_type, std::false_type>;
        using DivType    = decltype(BinaryPredicate()(1.f, std::common_type_t<T, TOther>()));
        using Type       = decltype(BinaryPredicate()(T(), TOther()));

        using TResult = std::conditional_t<IsDivision::value, DivType, Type>;
        DenseRaster<TResult> result(_meta);

        if constexpr (IsDivision::value) {
            result.set_nodata(DenseRaster<TResult>::NaN);
        } else {
            detail::assign_nodata_value(*this, other, result);
        }

        raster_operation<BinaryPredicate>(other, result);
        return result;
    }

    RasterMetadata _meta;
    data_type _data;
};

template <typename TScalar, typename T, typename = std::enable_if_t<std::is_scalar_v<TScalar>>>
DenseRaster<T> operator+(TScalar lhs, const DenseRaster<T>& rhs)
{
    return rhs + lhs;
}

template <typename TScalar, typename T, typename = std::enable_if_t<std::is_scalar_v<TScalar>>>
auto operator-(TScalar value, const DenseRaster<T>& rhs)
{
    using ResultType = decltype(TScalar() - T());

    DenseRaster<ResultType> result(rhs.metadata());

    std::transform(begin(rhs), end(rhs), begin(result), nodata::minus_scalar_first<ResultType>(rhs.metadata().nodata, static_cast<ResultType>(value)));

    return result;
}

template <typename TScalar, typename T, typename = std::enable_if_t<std::is_scalar_v<TScalar>>>
DenseRaster<T> operator*(TScalar lhs, const DenseRaster<T>& rhs)
{
    return rhs * lhs;
}

template <typename TScalar, typename T, typename = std::enable_if_t<std::is_scalar_v<TScalar>>>
auto operator/(TScalar scalar, const DenseRaster<T>& rhs)
{
    // throw_on_size_mismatch(other);

    //// For nan nodata, standard eigen operator can be used
    // if constexpr (has_nan() && std::is_same_v<T, TOther>) {
    //     // all types are the same, no casts needed
    //     return DenseRaster<T>(_meta, _data / other._data);
    // }

    // return performRasterOperation<nodata::divides>(other);

    using ResultType = decltype(1.0f * T());

    static_assert(std::is_scalar_v<T>, "Arithmetic operation called with non scalar type");
    DenseRaster<ResultType> result(rhs.metadata());
    for (std::size_t i = 0; i < rhs.size(); ++i) {
        auto value = rhs[i];
        if (value == 0) {
            if (!result.nodata().has_value()) {
                throw InvalidArgument("Division by raster that contains 0 values");
            }

            result.mark_as_nodata(i);
        } else {
            result[i] = scalar / static_cast<ResultType>(value);
        }
    }

    return result;
}

template <typename T>
auto cbegin(const DenseRaster<T>& ras)
{
    return ras.data();
}

template <typename T>
auto cend(const DenseRaster<T>& ras)
{
    return ras.cend();
}

template <typename T>
auto begin(DenseRaster<T>& ras)
{
    return ras.begin();
}

template <typename T>
auto begin(const DenseRaster<T>& ras)
{
    return ras.begin();
}

template <typename T>
auto end(DenseRaster<T>& ras)
{
    return ras.end();
}

template <typename T>
auto end(const DenseRaster<T>& ras)
{
    return ras.cend();
}

template <typename T>
const T* data(const DenseRaster<T>& ras)
{
    return ras.data();
}

template <typename T>
T* data(DenseRaster<T>& ras)
{
    return ras.data();
}

template <typename T>
auto size(const DenseRaster<T>& ras)
{
    return ras.size();
}
}
