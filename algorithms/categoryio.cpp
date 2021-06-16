#include "gdx/algo/categoryio.h"
#include "gdx/exception.h"
#include "gdx/log.h"
#include "infra/cast.h"
#include "infra/gdal.h"
#include "infra/string.h"

#include <fmt/format.h>
#include <functional>
#include <ogrsf_frmts.h>

namespace gdx::detail {

namespace gdal = inf::gdal;

std::vector<std::vector<float>> read_tab_data_row_based(const fs::path& fileName)
{
    // Files not ending in csv need to be prepended with CSV: to be treated as such
    return read_csv_data_row_based(fmt::format("CSV:{}", fileName));
}

std::variant<std::vector<std::vector<int32_t>>, std::vector<std::vector<float>>>
read_tab_data_row_based_v(const fs::path& fileName, int32_t keyColumns, int32_t index)
{
    // Files not ending in csv need to be prepended with CSV: to be treated as such
    return read_csv_data_row_based_v(fmt::format("CSV:{}", fileName), keyColumns, index);
}

std::vector<std::vector<float>> read_tab_data_column_based(const fs::path& fileName)
{
    // Files not ending in csv need to be prepended with CSV: to be treated as such
    return read_csv_data_column_based(fmt::format("CSV:{}", fileName));
}

std::vector<std::vector<float>> read_csv_data_row_based(const fs::path& fileName)
{
    std::vector<std::vector<float>> result;

    auto dataSet = gdal::VectorDataSet::open(fileName, gdal::VectorType::Csv);
    if (dataSet.layer_count() != 1) {
        throw RuntimeError("Only tab files with a single band are currently supported");
    }

    for (auto& feature : dataSet.layer(0)) {
        std::vector<float> row;
        row.reserve(feature.field_count());

        for (auto& field : feature) {
            if (std::holds_alternative<int32_t>(field)) {
                row.push_back(static_cast<float>(std::get<int32_t>(field)));
            } else if (std::holds_alternative<int64_t>(field)) {
                row.push_back(static_cast<float>(std::get<int64_t>(field)));
            } else if (std::holds_alternative<double>(field)) {
                row.push_back(static_cast<float>(std::get<double>(field)));
            } else if (std::holds_alternative<std::string_view>(field)) {
                row.push_back(static_cast<float>(std::atof(std::get<std::string_view>(field).data())));
            } else {
                Log::warn("Unrecognized csv entry");
            }
        }

        result.emplace_back(std::move(row));
    }

    return result;
}

static void handle_string(const std::string& tmp, bool only_int, std::vector<float>& row, bool& float_required)
{
    float v;
    try {
        v = inf::str::to_float_value(tmp);
    } catch (...) {
        throw RuntimeError("The mapping table can only contain integer keys (and NaN).  Use tab seperator only.");
    }
    if (std::isnan(v)) {
        row.push_back(-9999.0f);
    } else {
        if (v != int32_t(v)) {
            if (only_int) {
                throw RuntimeError("The mapping table can only contain integer keys (and NaN).  Use tab seperator only.");
            } else {
                float_required = true;
            }
        }
        row.push_back(v);
    }
}

std::variant<std::vector<std::vector<int32_t>>, std::vector<std::vector<float>>>
read_csv_data_row_based_v(const fs::path& fileName, int32_t keyColumns, int32_t index)
{
    std::vector<std::vector<float>> float_result;
    bool float_required = false;

    auto dataSet = gdal::VectorDataSet::open(fileName, gdal::VectorType::Csv);
    if (dataSet.layer_count() != 1) {
        throw RuntimeError("Only tab files with a single band are currently supported");
    }

    for (auto& feature : dataSet.layer(0)) {
        std::vector<float> row;
        row.reserve(feature.field_count());

        int col = 0;
        for (auto& field : feature) {
            if (col < keyColumns) {
                if (std::holds_alternative<int32_t>(field)) {
                    row.push_back(static_cast<float>(std::get<int32_t>(field)));
                } else if (std::holds_alternative<std::string_view>(field)) {
                    std::string tmp = std::get<std::string_view>(field).data();
                    handle_string(tmp, true, row, float_required);
                } else {
                    throw RuntimeError("The mapping table can only contain int32 keys (use NaN to match with nodata)");
                }
            } else {
                if (std::holds_alternative<int32_t>(field)) {
                    row.push_back(static_cast<float>(std::get<int32_t>(field)));
                } else if (std::holds_alternative<int64_t>(field)) {
                    row.push_back(static_cast<float>(std::get<int64_t>(field)));
                    float_required = true;
                } else if (std::holds_alternative<double>(field)) {
                    row.push_back(static_cast<float>(std::get<double>(field)));
                    float_required = true;
                } else if (std::holds_alternative<std::string_view>(field)) {
                    std::string tmp = std::get<std::string_view>(field).data();
                    handle_string(tmp, false, row, float_required);
                } else {
                    throw RuntimeError("The mapping table contains unrecognisable value");
                }
            }
            col += 1;
        }

        float_result.emplace_back(std::move(row));
    }
    for (int i = 0; i < int(float_result.size()); ++i) {
        if (inf::truncate<int>(float_result[i].size()) < keyColumns + 1) {
            throw RuntimeError("The mapping table size should match the number of rasters provided");
        }
        if (float_result[i].size() != float_result[0].size()) {
            throw RuntimeError("All mapping table rows should have the same number of columns");
        }
        if (index < 1 || inf::truncate<int>(float_result[i].size()) < keyColumns + index) {
            throw RuntimeError("The mapping table does not have a data column with index {}", index);
        }
    }
    std::variant<std::vector<std::vector<int32_t>>, std::vector<std::vector<float>>> result;
    if (float_required) {
        for (int i = 0; i < int(float_result.size()); ++i) {
            float_result[i][keyColumns] = float_result[i][keyColumns + index - 1];
            float_result[i].resize(keyColumns + 1);
        }
        result = float_result;
    } else {
        std::vector<std::vector<int32_t>> int_result;
        int_result.resize(float_result.size());
        for (int i = 0; i < int(float_result.size()); ++i) {
            int_result[i].resize(keyColumns + 1);
            for (int j = 0; j < keyColumns; ++j) {
                int_result[i][j] = int32_t(float_result[i][j]);
            }
            int_result[i][keyColumns] = int32_t(float_result[i][keyColumns + index - 1]);
        }
        result = int_result;
    }

    return result;
}

std::vector<std::vector<float>> read_csv_data_column_based(const fs::path& fileName)
{
    std::vector<std::vector<float>> result;

    auto dataSet = gdal::VectorDataSet::open(fileName, gdal::VectorType::Csv);
    if (dataSet.layer_count() != 1) {
        throw RuntimeError("Only tab files with a single band are currently supported");
    }

    for (auto& feature : dataSet.layer(0)) {
        if (feature.field_count() > static_cast<int32_t>(result.size())) {
            result.resize(feature.field_count());
        }

        int index = 0;
        for (auto& field : feature) {
            if (std::holds_alternative<int32_t>(field)) {
                result[index].push_back(static_cast<float>(std::get<int32_t>(field)));
            } else if (std::holds_alternative<int64_t>(field)) {
                result[index].push_back(static_cast<float>(std::get<int64_t>(field)));
            } else if (std::holds_alternative<double>(field)) {
                result[index].push_back(static_cast<float>(std::get<double>(field)));
            } else if (std::holds_alternative<std::string_view>(field)) {
                auto doubleStr = std::get<std::string_view>(field);
                char* end;
                double value = std::strtod(doubleStr.data(), &end);
                if (errno == ERANGE || end == doubleStr.data()) {
                    throw RuntimeError("Invalid value in mapping file: '{}'", doubleStr);
                }
                result[index].push_back(static_cast<float>(value));
            } else {
                Log::warn("Unrecognized csv entry");
            }

            ++index;
        }
    }

    return result;
}
}
