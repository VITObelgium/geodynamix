#pragma once

#include "gdx/denseraster.h"

#include <cinttypes>
#include <variant>

namespace inf {
class ColorMap;
}

namespace gdx {

// Type erased raster
class Raster
{
public:
    using RasterVariant = std::variant<
        DenseRaster<uint8_t>,
        DenseRaster<uint16_t>,
        DenseRaster<uint32_t>,
        DenseRaster<int16_t>,
        DenseRaster<int32_t>,
        DenseRaster<float>,
        DenseRaster<double>>;

    Raster(int32_t rows, int32_t cols, const std::type_info& dataType);
    Raster(int32_t rows, int32_t cols, const std::type_info& dataType, double fillValue);
    Raster(RasterMetadata meta, const std::type_info& dataType);
    Raster(RasterMetadata meta, const std::type_info& dataType, double fillValue);
    Raster(Raster&& other) = default;

    template <class RasterType>
    Raster(RasterType&& ras)
    : _type(typeid(typename RasterType::value_type))
    , _raster(std::forward<RasterType>(ras))
    {
    }

    RasterVariant& get();
    const RasterVariant& get() const;

    const std::type_info& type() const noexcept;
    const RasterMetadata& metadata() const;

    template <typename T>
    typename DenseRaster<T>::value_type* raster_data()
    {
        assert(std::holds_alternative<DenseRaster<T>>(_raster));
        return std::get<DenseRaster<T>>(_raster).data();
    }

    template <typename T>
    const typename DenseRaster<T>::value_type* raster_data() const
    {
        assert(std::holds_alternative<DenseRaster<T>>(_raster));
        return std::get<DenseRaster<T>>(_raster).data();
    }

    Raster copy() const;

    template <typename T>
    DenseRaster<T>& get()
    {
        return std::get<DenseRaster<T>>(_raster);
    }

    template <typename T>
    const DenseRaster<T>& get() const
    {
        return std::get<DenseRaster<T>>(_raster);
    }

    bool equalTo(const Raster& other) const noexcept;
    bool tolerant_equal_to(const Raster& other, double tolerance) const noexcept;
    bool tolerant_data_equal_to(const Raster& other, double tolerance) const noexcept;
    Raster& fill(double value);
    Raster& set_projection(int32_t epsg);
    Raster& replaceValue(double oldValue, double newValue);
    void setValue(int32_t row, int32_t col, double value);
    void turn_value_into_nodata(double value);

    operator bool() const;

    Raster operator+(const Raster& other) const;
    Raster operator+(int32_t value) const;
    Raster operator+(double value) const;

    Raster operator-() const;
    Raster operator-(const Raster& other) const;
    Raster operator-(int32_t value) const;
    Raster operator-(double value) const;

    Raster operator*(const Raster& other) const;
    Raster operator*(int32_t value) const;
    Raster operator*(double value) const;

    Raster operator/(const Raster& other) const;
    Raster operator/(int32_t value) const;
    Raster operator/(double value) const;

    Raster operator!() const;
    Raster operator~() const;
    Raster operator&&(const Raster& other) const;
    Raster operator&(const Raster& other) const;
    Raster operator||(const Raster& other) const;
    Raster operator|(const Raster& other) const;
    Raster operator>(const Raster& other) const;
    Raster operator>(double threshold) const;
    Raster operator>=(const Raster& other) const;
    Raster operator>=(double threshold) const;
    Raster operator<(const Raster& other) const;
    Raster operator<(double threshold) const;
    Raster operator<=(const Raster& other) const;
    Raster operator<=(double threshold) const;
    Raster operator==(const Raster& other) const;
    Raster operator==(int32_t value) const;
    Raster operator==(double value) const;
    Raster operator!=(const Raster& other) const;
    Raster operator!=(int32_t value) const;
    Raster operator!=(double value) const;

    Raster cast(const std::type_info& type) const;

    static Raster read(const fs::path& filepath);
    static Raster read(const fs::path& filepath, const RasterMetadata& extent);
    static Raster read(const fs::path& filepath, const std::type_info& dataType);
    static Raster read(const fs::path& fileName, const std::type_info& type, const RasterMetadata& extent);

    static Raster readFromMemory(const fs::path& path, std::span<const uint8_t> data);
    static Raster readFromMemory(const fs::path& path, std::span<const uint8_t> data, const std::type_info& dataType);

    static double nodataForType(const std::type_info& dataType);

    void write(const fs::path& filepath);
    void write(const fs::path& filepath, const std::type_info& dataType);

    void writeColorMapped(const fs::path& filepath, const inf::ColorMap& cmap) const;
    // void writeColorMapped(const std::string& filepath, const std::type_info& dataType, const ColorMap& cmap) const;

private:
    void throwOnTypeMismatch(const std::type_info& other) const;

    template <typename TOtherRaster>
    void throwOnTypeMismatch() const;

    template <typename TScalar>
    void throwOnScalarTypeMismatch() const;

    const std::type_info& _type;
    RasterVariant _raster;
};

Raster operator+(int32_t lhs, const Raster& rhs);
Raster operator+(double lhs, const Raster& rhs);
Raster operator-(int32_t lhs, const Raster& rhs);
Raster operator-(double lhs, const Raster& rhs);
Raster operator*(int32_t lhs, const Raster& rhs);
Raster operator*(double lhs, const Raster& rhs);
Raster operator/(int32_t lhs, const Raster& rhs);
Raster operator/(double lhs, const Raster& rhs);
}
