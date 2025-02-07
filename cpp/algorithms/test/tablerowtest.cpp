#include "gdx/algo/tablerow.h"
#include "gdx/test/testbase.h"

#include <numeric>
#include <random>

namespace gdx::test {

TEST_CASE_TEMPLATE("Tablerow", TypeParam, UnspecializedRasterTypes)
{
    using FloatRaster = typename TypeParam::template type<float>;
    using IntRaster   = typename TypeParam::template type<int32_t>;

    SUBCASE("tableRowTestsFloatInt")
    {
        RasterMetadata meta(1, 5, 0.0, 0.0, 100.0, -9999.0);
        IntRaster categories(meta, std::vector<int>{1, 1, 1, 2, 0});
        FloatRaster values(meta, std::vector<float>{0.0f, 12.0f, 36.0f, 31.0f, 0.0f});

        std::pair<std::vector<std::string>, std::vector<std::string>> actual;
        std::vector<std::string> expected;

        actual   = table_row_impl(values, categories, gdx::Operation::Sum);
        expected = {"0.000000", "48.000000", "31.000000"};
        CHECK(actual.second == expected);

        actual   = table_row_impl(values, categories, gdx::Operation::Count);
        expected = {"1", "3", "1"};
        CHECK(actual.second == expected);

        actual   = table_row_impl(values, categories, gdx::Operation::CountNonZero);
        expected = {"2", "1"}; // because the count for cat '0' is zero, it is omitted
        CHECK(actual.second == expected);

        actual   = table_row_impl(values, categories, gdx::Operation::Min);
        expected = {"0.000000", "0.000000", "31.000000"};
        CHECK(actual.second == expected);

        actual   = table_row_impl(values, categories, gdx::Operation::MinNonZero);
        expected = {"12.000000", "31.000000"}; // because non nonzero values for cat '0'
        CHECK(actual.second == expected);

        actual   = table_row_impl(values, categories, gdx::Operation::Max);
        expected = {"0.000000", "36.000000", "31.000000"};
        CHECK(actual.second == expected);

        actual   = table_row_impl(values, categories, gdx::Operation::Average);
        expected = {"0.000000", "16.000000", "31.000000"};
        CHECK(actual.second == expected);

        actual   = table_row_impl(values, categories, gdx::Operation::AverageInCat1);
        expected = {"16.000000"};
        CHECK(actual.second == expected);
    }
}
}