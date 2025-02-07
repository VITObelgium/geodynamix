#pragma once

#include <string>
#include <variant>
#include <vector>

#include "infra/filesystem.h"

namespace gdx::detail {

// Row based (inner vector contains the rows)
std::vector<std::vector<float>> read_tab_data_row_based(const fs::path& fileName);
std::vector<std::vector<float>> read_csv_data_row_based(const fs::path& fileName);
// variant versions of above's
std::variant<std::vector<std::vector<int32_t>>, std::vector<std::vector<float>>>
read_tab_data_row_based_v(const fs::path& fileName, int32_t keyColumns, int32_t dataColumn);
std::variant<std::vector<std::vector<int32_t>>, std::vector<std::vector<float>>>
read_csv_data_row_based_v(const fs::path& fileName, int32_t dataColumns, int32_t dataColumn);

// Column based (inner vector contains the columns)
std::vector<std::vector<float>> read_tab_data_column_based(const fs::path& fileName);
std::vector<std::vector<float>> read_csv_data_column_based(const fs::path& fileName);

} // namespace gdx::io
