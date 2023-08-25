#include "gdx/algo/categoryio.h"
#include "gdx/test/rasterasserts.h"
#include "testconfig.h"

#include <algorithm>
#include <doctest/doctest.h>

namespace gdx::test {

using namespace inf;
using namespace doctest;

static const std::vector<std::vector<float>> s_expectedRows = {
    {1, 10},
    {2, 20},
    {3, 30},
    {4, 40}};

static const std::vector<std::vector<float>> s_expectedCols = {
    {1, 2, 3, 4},
    {10, 20, 30, 40}};

TEST_CASE("ShapeIOTest")
{
    std::vector<std::vector<float>> data;

    SUBCASE("row based")
    {
        SUBCASE("readTabDataAsRows")
        {
            data = detail::read_tab_data_row_based(file::u8path(TEST_DATA_DIR) / "testtabfile.tab");
        }

        SUBCASE("readCsvDataAsRows")
        {
            data = detail::read_csv_data_row_based(file::u8path(TEST_DATA_DIR) / "testtabfile.csv");
        }

        REQUIRE(data.size() == s_expectedRows.size());
        for (size_t i = 0; i < data.size(); ++i) {
            CHECK_CONTAINER_EQ(data[i], s_expectedRows[i]);
        }
    }

    SUBCASE("col based")
    {
        SUBCASE("readTabDataAsColumns")
        {
            data = detail::read_tab_data_column_based(file::u8path(TEST_DATA_DIR) / "testtabfile.tab");
        }

        SUBCASE("readCsvDataAsColumns")
        {
            data = detail::read_csv_data_column_based(file::u8path(TEST_DATA_DIR) / "testtabfile.csv");
        }

        REQUIRE(data.size() == s_expectedCols.size());
        for (size_t i = 0; i < data.size(); ++i) {
            CHECK_CONTAINER_EQ(data[i], s_expectedCols[i]);
        }
    }
}
}
