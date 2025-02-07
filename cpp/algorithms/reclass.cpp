#include "gdx/algo/reclass.h"
#include "gdx/algo/categoryio.h"
#include <variant>

namespace gdx {

std::variant<std::vector<std::vector<int32_t>>, std::vector<std::vector<float>>>
read_mapping_file(const fs::path& mappingFile, int32_t keyColumns)
{
    return detail::read_tab_data_row_based_v(mappingFile, keyColumns, 1);
}

std::variant<std::vector<std::vector<int32_t>>, std::vector<std::vector<float>>>
read_mapping_file(const fs::path& mappingFile, int32_t keyColumns, int32_t index)
{
    return detail::read_tab_data_row_based_v(mappingFile, keyColumns, index);
}

std::vector<std::vector<float>> read_n_mapping_file(const fs::path& mappingFile)
{
    auto data = detail::read_tab_data_row_based(mappingFile);
    for (auto& row : data) {
        if (row.size() != 3) {
            throw RuntimeError("Each row in the nreclass mapping file should contain 3 values");
        }

        // Compatibility with old behavior, replace -9999 values with nan
        std::replace(row.begin(), row.end(), -9999.0f, std::numeric_limits<float>::quiet_NaN());
    }

    return data;
}
}
