#pragma once

#include "gdx/exception.h"
#include "gdx/log.h"
#include "infra/filesystem.h"
#include "infra/string.h"

#include <variant>

namespace gdx {

template <typename T>
using MappingData = std::vector<std::vector<T>>;

using MappingDataVariant = std::variant<MappingData<int32_t>, MappingData<float>>;

MappingDataVariant read_mapping_file(const fs::path& mappingFile, int32_t keyColumns, int32_t dataColIndex);
MappingData<float> read_n_mapping_file(const fs::path& mappingFile);

namespace internal {

template <typename T>
std::string optional_to_string(const std::optional<T>& opt)
{
    if (opt) {
        return std::to_string(*opt);
    } else {
        return "nodata";
    }
}

template <typename T, int ElementCount>
std::string values_to_string(const std::array<std::optional<T>, ElementCount>& values)
{
    return inf::str::join(values, ", ", [](const std::optional<T>& opt) {
        return optional_to_string<T>(opt);
    });
}

template <typename T>
void throw_on_mapping_issue(const MappingData<float>& mapping, std::optional<double> nodata)
{
    if constexpr (std::is_floating_point_v<T>) {
        return;
    }

    bool warningGiven = false;

    for (auto& mapList : mapping) {
        for (auto& value : mapList) {
            if (value > static_cast<double>(std::numeric_limits<T>::max())) {
                throw RuntimeError("Mapping value is bigger then input data type");
            }

            if (value < static_cast<double>(std::numeric_limits<T>::lowest())) {
                throw RuntimeError("Mapping value is smaller then input data type");
            }

            auto resultType = static_cast<T>(value);
            if (value - resultType > std::numeric_limits<T>::epsilon()) {
                throw RuntimeError("Floating point mapping values cannot be used on integral raster");
            }

            if (!warningGiven && nodata.has_value() && static_cast<T>(*nodata) == static_cast<T>(value)) {
                warningGiven = true;
                Log::warn("Reclass mapping value matches the resulting rasters nodata value, this can lead to unexpected results");
            }
        }
    }
}

template <typename T, int ElementCount>
std::optional<T> reclass_value(const std::array<std::optional<T>, ElementCount>& values, const MappingData<T>& mapping, bool& warningGiven)
{
    auto iter = std::find_if(mapping.begin(), mapping.end(), [&](const std::vector<T>& mappingValues) {
        for (std::size_t i = 0; i < values.size(); ++i) {
            if (values[i].has_value()) {
                if (values[i] != static_cast<T>(mappingValues[i])) {
                    return false;
                }
            } else if (mappingValues[i] != -9999) { // mapping table key equals NODATA
                return false;
            }
        }

        return true;
    });

    if (iter == mapping.end()) {
        if (!warningGiven) {
            Log::warn("No mapping available for raster values: {}", values_to_string<T, ElementCount>(values));
            warningGiven = true;
        }

        return std::optional<T>();
    }

    auto result = iter->at(values.size());
    if (result == -9999) { // mapping table key lookup result is NODATA
        return std::optional<T>();
    }

    return static_cast<T>(result);
}
}

template <typename ResultType, template <typename> typename RasterType, typename MappingType, typename T>
auto reclass_to(const MappingData<MappingType>& mapping, const RasterType<T>& ras)
{
    RasterType<ResultType> result(ras.metadata());
    ResultType nodata = static_cast<ResultType>(result.has_nan() ? RasterType<ResultType>::NaN : -9999.0);
    result.set_nodata(nodata);

    const auto size   = ras.size();
    bool warningGiven = false;

    for (std::size_t i = 0; i < size; ++i) {
        std::array<std::optional<MappingType>, 1> values = {{ras.template optional_value_as<MappingType>(i)}};

        auto v = internal::reclass_value<MappingType, 1>(values, mapping, warningGiven);
        if (v) {
            result[i] = static_cast<ResultType>(*v);
            result.mark_as_data(i);
        } else {
            result[i] = nodata;
            result.mark_as_nodata(i);
        }
    }

    return result;
}

template <typename ResultType, template <typename> typename RasterType, typename MappingType, typename T1, typename T2>
auto reclass_to(const MappingData<MappingType>& mapping, const RasterType<T1>& ras1, const RasterType<T2>& ras2)
{
    RasterType<ResultType> result(ras1.metadata());
    ResultType nodata = static_cast<ResultType>(result.has_nan() ? result.NaN : -9999.0);
    result.set_nodata(nodata);

    if (ras1.size() != ras2.size()) {
        throw InvalidArgument("Raster sizes should match {} {}", ras1.size(), ras2.size());
    }

    const auto size   = ras1.size();
    bool warningGiven = false;

    for (std::size_t i = 0; i < size; ++i) {
        std::array<std::optional<MappingType>, 2> values = {{ras1.template optional_value_as<MappingType>(i),
            ras2.template optional_value_as<MappingType>(i)}};

        auto v = internal::reclass_value<MappingType, 2>(values, mapping, warningGiven);
        if (v) {
            result[i] = static_cast<ResultType>(*v);
            result.mark_as_data(i);
        } else {
            result[i] = nodata;
            result.mark_as_nodata(i);
        }
    }

    return result;
}

template <typename ResultType, template <typename> typename RasterType, typename MappingType, typename T1, typename T2, typename T3>
auto reclass_to(const MappingData<MappingType>& mapping, const RasterType<T1>& ras1, const RasterType<T2>& ras2, const RasterType<T3>& ras3)
{
    RasterType<ResultType> result(ras1.metadata());
    ResultType nodata = static_cast<ResultType>(result.has_nan() ? result.NaN : -9999.0);
    result.set_nodata(nodata);

    if (ras1.size() != ras2.size() || ras1.size() != ras3.size()) {
        throw InvalidArgument("Raster sizes should match {} {} {}", ras1.size(), ras2.size(), ras3.size());
    }

    const auto size   = ras1.size();
    bool warningGiven = false;

    for (std::size_t i = 0; i < size; ++i) {
        std::array<std::optional<MappingType>, 3> values = {{ras1.template optional_value_as<MappingType>(i),
            ras2.template optional_value_as<MappingType>(i),
            ras3.template optional_value_as<MappingType>(i)}};

        auto v = internal::reclass_value<ResultType, 3>(values, mapping, warningGiven);
        if (v) {
            result[i] = static_cast<ResultType>(*v);
            result.mark_as_data(i);
        } else {
            result[i] = nodata;
            result.mark_as_nodata(i);
        }
    }

    return result;
}

template <typename ResultType, template <typename> typename RasterType, typename T>
RasterType<ResultType> reclass_to(const std::string& mappingFile, const RasterType<T>& ras)
{
    auto mappingTableVariant = read_mapping_file(mappingFile, 1, 1);
    if constexpr (std::is_floating_point_v<ResultType>) {
        if (!std::holds_alternative<MappingData<float>>(mappingTableVariant)) {
            throw RuntimeError("Reclass mapping contents do not match the requested result type");
        }
    }

    if constexpr (std::is_floating_point_v<ResultType>) {
        auto& mappingTable = std::get<MappingData<float>>(mappingTableVariant);
        return reclass_to<ResultType>(mappingTable, ras);
    } else {
        auto& mappingTable = std::get<std::vector<std::vector<int32_t>>>(mappingTableVariant);
        return reclass_to<ResultType>(mappingTable, ras);
    }
}

template <template <typename> typename RasterType, typename T>
auto reclass(const std::string& mappingFile, const RasterType<T>& ras)
{
    auto mapping_table_variant = read_mapping_file(mappingFile, 1, 1);
    std::variant<RasterType<int32_t>, RasterType<float>> result;
    std::visit([&](auto&& mappingTable) {
        using MappingType = typename std::decay_t<decltype(mappingTable.front())>::value_type;
        result            = reclass_to<MappingType>(mappingTable, ras);
    },
        mapping_table_variant);
    return result;
}

template <template <typename> typename RasterType, typename T1, typename T2>
auto reclass(const std::string& mappingFile, const RasterType<T1>& ras1, const RasterType<T2>& ras2)
{
    auto mapping_table_variant = read_mapping_file(mappingFile, 2, 1);
    std::variant<RasterType<int32_t>, RasterType<float>> result;
    std::visit([&](auto&& mappingTable) {
        using MappingType = typename std::decay_t<decltype(mappingTable.front())>::value_type;
        result            = reclass_to<MappingType>(mappingTable, ras1, ras2);
    },
        mapping_table_variant);
    return result;
}

template <template <typename> typename RasterType, typename T1, typename T2, typename T3>
auto reclass(const std::string& mappingFile, const RasterType<T1>& ras1, const RasterType<T2>& ras2, const RasterType<T3>& ras3)
{
    auto mapping_table_variant = read_mapping_file(mappingFile, 3, 1);
    std::variant<RasterType<int32_t>, RasterType<float>> result;
    std::visit([&](auto&& mappingTable) {
        using MappingType = typename std::decay_t<decltype(mappingTable.front())>::value_type;
        result            = reclass_to<MappingType>(mappingTable, ras1, ras2, ras3);
    },
        mapping_table_variant);
    return result;
}

template <template <typename> typename RasterType, typename T>
auto reclassi(const std::string& mappingFile, const RasterType<T>& ras, int32_t index)
{
    auto mapping_table_variant = read_mapping_file(mappingFile, 1, index);
    std::variant<RasterType<int32_t>, RasterType<float>> result;
    std::visit([&](auto&& mappingTable) {
        using MappingType = typename std::decay_t<decltype(mappingTable.front())>::value_type;
        result            = reclass_to<MappingType>(mappingTable, ras);
    },
        mapping_table_variant);
    return result;
}

template <template <typename> typename RasterType, typename T1, typename T2>
auto reclassi(const std::string& mappingFile, const RasterType<T1>& ras1, const RasterType<T2>& ras2, int32_t index)
{
    auto mapping_table_variant = read_mapping_file(mappingFile, 2, index);
    std::variant<RasterType<int32_t>, RasterType<float>> result;
    std::visit([&](auto&& mappingTable) {
        using MappingType = typename std::decay_t<decltype(mappingTable.front())>::value_type;
        result            = reclass_to<MappingType>(mappingTable, ras1, ras2);
    },
        mapping_table_variant);
    return result;
}

template <template <typename> typename RasterType, typename T1, typename T2, typename T3>
auto reclassi(const std::string& mappingFile, const RasterType<T1>& ras1, const RasterType<T2>& ras2, const RasterType<T3>& ras3, int32_t index)
{
    auto mapping_table_variant = read_mapping_file(mappingFile, 3, index);
    std::variant<RasterType<int32_t>, RasterType<float>> result;
    std::visit([&](auto&& mappingTable) {
        using MappingType = typename std::decay_t<decltype(mappingTable.front())>::value_type;
        result            = reclass_to<MappingType>(mappingTable, ras1, ras2, ras3);
    },
        mapping_table_variant);
    return result;
}

template <template <typename> typename RasterType, typename T>
RasterType<T> nreclass(const MappingData<float>& mapping, const RasterType<T>& raster)
{
    auto resultMeta = raster.metadata();
    T nodata        = std::numeric_limits<T>::max();
    if (resultMeta.nodata.has_value()) {
        nodata = static_cast<T>(*resultMeta.nodata);
    } else if constexpr (RasterType<T>::raster_type_has_nan) {
        nodata = RasterType<T>::NaN;
    }

    resultMeta.nodata = nodata;
    internal::throw_on_mapping_issue<T>(mapping, resultMeta.nodata);

    RasterType<T> result(resultMeta, nodata);

    bool warningTriggered        = false;
    int countNANbecauseKeyNAN    = 0;
    int countNANbecauseNoSuchKey = 0;

    for (std::size_t i = 0; i < raster.size(); ++i) {
        if (raster.is_nodata(i)) {
            ++countNANbecauseKeyNAN;
            continue;
        }

        const auto key = raster[i];
        bool found     = false;
        for (std::size_t j = 0; j < mapping.size(); ++j) {
            if (T(mapping[j][0]) < key && key <= T(mapping[j][1])) {
                if (!std::isnan(mapping[j][2])) {
                    result[i] = static_cast<T>(mapping[j][2]);
                    result.mark_as_data(i);
                }

                found = true;
                break;
            }
        }

        if (!found) {
            if (!warningTriggered) {
                warningTriggered = true;
                Log::warn("nreclass : no entry for key {} (has nodata result)", key);
            }

            ++countNANbecauseNoSuchKey;
        }
    }

    if (countNANbecauseNoSuchKey > 0) {
        Log::warn("nreclass : {} times nodata result because input key is not in table", countNANbecauseNoSuchKey);
    }

    if (countNANbecauseKeyNAN > 0) {
        Log::warn("nreclass : {} times nodata result because input key is nodata", countNANbecauseKeyNAN);
    }

    return result;
}

template <template <typename> typename RasterType, typename T>
RasterType<T> nreclass(const std::string& mappingFile, const RasterType<T>& raster)
{
    return nreclass(read_n_mapping_file(mappingFile), raster);
}
}
