#include "gdx/rasterarea.h"
#include "gdx/test/testbase.h"

#include <doctest/doctest.h>
#include <vector>

namespace gdx::test {

TEST_CASE_TEMPLATE("raster areas", TypeParam, RasterTypes)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;

    SUBCASE("cell neighbours square")
    {
        // clang-format off
            auto data = convertTo<T>(std::vector<double>{{
                -1,  1,  2,  3,  4,
                5,  6,  7,  8, -1,
                10, -1, -1, -1, 14,
                15, -1, -1, -1, 19,
            }});
        // clang-format on

        Raster raster(RasterMetadata(4, 5, -1), data);

        std::vector<T> result;
        auto area = neighbouring_cells_square(raster, Cell(1, 1), 1);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({1, 2, 5, 7, 10}), result);
        result.clear();

        area = neighbouring_cells_square(raster, Cell(0, 0), 1);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({1, 5, 6}), result);
        result.clear();

        area = neighbouring_cells_square(raster, Cell(0, 3), 1);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({2, 4, 7, 8}), result);
        result.clear();

        area = neighbouring_cells_square(raster, Cell(0, 4), 1);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({3, 8}), result);
        result.clear();

        area = neighbouring_cells_square(raster, Cell(3, 2), 1);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK(result.empty());
        result.clear();

        area = neighbouring_cells_square(raster, Cell(3, 4), 1);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({14}), result);
        result.clear();

        area = neighbouring_cells_square(raster, Cell(3, 0), 1);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({10}), result);
        result.clear();

        area = neighbouring_cells_square(raster, Cell(1, 2), 2);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({1, 2, 3, 4, 5, 6, 8, 10, 14, 15, 19}), result);
        result.clear();
    }

    SUBCASE("cell neighbours square iterator cell")
    {
        Raster raster(RasterMetadata(4, 5, -1), convertTo<T>(std::vector<double>(4 * 5, 9)));

        std::vector<Cell> result;
        auto area = neighbouring_cells_square(raster, Cell(1, 1), 1);
        for (auto iter = area.begin(); iter != area.end(); ++iter) {
            result.push_back(iter->cell());
        }

        CHECK(result == std::vector<Cell>({{0, 0}, {0, 1}, {0, 2}, {1, 0}, {1, 2}, {2, 0}, {2, 1}, {2, 2}}));
    }

    SUBCASE("cell neighbours circular")
    {
        // clang-format off
        auto data = convertTo<T>(std::vector<double>{{
            -1,  1,  2,  3,  4, 0,
                5,  6,  7,  8, -1, 0,
            10, -1,  0, -1, 14, 0,
            15, -1, -1, 18, 19, 0,
            20, 21, 22, 23, 24, 0,
        }});
        // clang-format on

        Raster raster(RasterMetadata(5, 6, -1), data);

        // radius = 1
        std::vector<T> result;
        auto area = neighbouring_cells_circular(raster, Cell(2, 2), 1);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({7}), result);
        result.clear();

        // radius = 2
        area = neighbouring_cells_circular(raster, Cell(2, 2), 2);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({2, 6, 7, 8, 10, 14, 18, 22}), result);
        result.clear();

        // radius = 2, edge case
        area = neighbouring_cells_circular(raster, Cell(1, 1), 2);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({1, 2, 5, 7, 8, 10, 0}), result);
        result.clear();

        // Top left, big radius
        area = neighbouring_cells_circular(raster, Cell(0, 0), 100);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({1, 2, 3, 4, 0, 5, 6, 7, 8, 0, 10, 0, 14, 0, 15, 18, 19, 0, 20, 21, 22, 23, 24, 0}), result);
        result.clear();

        // Bottom right, big radius
        area = neighbouring_cells_circular(raster, Cell(4, 5), 100);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({1, 2, 3, 4, 0, 5, 6, 7, 8, 0, 10, 0, 14, 0, 15, 18, 19, 0, 20, 21, 22, 23, 24}), result);
        result.clear();
    }

    SUBCASE("circular area")
    {
        // clang-format off
        auto data = convertTo<T>(std::vector<double>{{
            -1,  1,  2,  3,  4, 0,
             5,  6,  7,  8, -1, 0,
            10, -1,  0, -1, 14, 0,
            15, -1, -1, 18, 19, 0,
            20, 21, 22, 23, 24, 0,
        }});
        // clang-format on

        Raster raster(RasterMetadata(5, 6, -1), data);

        // radius = 1
        std::vector<T> result;
        auto area = cells_circular(raster, Cell(2, 2), 1);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({7, 0}), result);
        result.clear();

        // radius = 2
        area = cells_circular(raster, Cell(2, 2), 2);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({2, 6, 7, 8, 10, 0, 14, 18, 22}), result);
        result.clear();

        // radius = 2, edge case
        area = cells_circular(raster, Cell(1, 1), 2);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({1, 2, 5, 6, 7, 8, 10, 0}), result);
        result.clear();

        // Top left, big radius
        area = cells_circular(raster, Cell(0, 0), 100);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({1, 2, 3, 4, 0, 5, 6, 7, 8, 0, 10, 0, 14, 0, 15, 18, 19, 0, 20, 21, 22, 23, 24, 0}), result);
        result.clear();

        // Bottom right, big radius
        area = cells_circular(raster, Cell(4, 5), 100);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({1, 2, 3, 4, 0, 5, 6, 7, 8, 0, 10, 0, 14, 0, 15, 18, 19, 0, 20, 21, 22, 23, 24, 0}), result);

        result.clear();
    }

    SUBCASE("neigbouring circular area")
    {
        // clang-format off
        auto data = convertTo<T>(std::vector<double>{{
            -1,  1,  2,  3,  4, 0,
            5,  6,  7,  8, -1, 0,
            10, -1,  0, -1, 14, 0,
            15, -1, -1, 18, 19, 0,
            20, 21, 22, 23, 24, 0,
        }});
        // clang-format on

        Raster raster(RasterMetadata(5, 6, -1), data);

        // radius = 1
        std::vector<T> result;
        auto area = neighbouring_cells_circular(raster, Cell(2, 2), 1);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({7}), result);
        result.clear();

        // radius = 2
        area = neighbouring_cells_circular(raster, Cell(2, 2), 2);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({2, 6, 7, 8, 10, 14, 18, 22}), result);
        result.clear();

        // radius = 2, edge case
        area = neighbouring_cells_circular(raster, Cell(1, 1), 2);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({1, 2, 5, 7, 8, 10, 0}), result);
        result.clear();

        // Top left, big radius
        area = neighbouring_cells_circular(raster, Cell(0, 0), 100);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({1, 2, 3, 4, 0, 5, 6, 7, 8, 0, 10, 0, 14, 0, 15, 18, 19, 0, 20, 21, 22, 23, 24, 0}), result);
        result.clear();

        // Bottom right, big radius
        area = neighbouring_cells_circular(raster, Cell(4, 5), 100);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({1, 2, 3, 4, 0, 5, 6, 7, 8, 0, 10, 0, 14, 0, 15, 18, 19, 0, 20, 21, 22, 23, 24}), result);
        result.clear();
    }

    SUBCASE("raster subarea")
    {
        // clang-format off
        auto data = convertTo<T>(std::vector<double>{{
            -1,  1,  2,  3,  4,
             5,  6,  7,  8, -1,
            10, -1, -1, -1, 14,
            15, -1, -1, -1, 19,
        }});
        // clang-format on

        Raster raster(RasterMetadata(4, 5, -1), data);

        std::vector<T> result;
        // Center area
        auto area = sub_area(raster, Cell(1, 1), 2, 3);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({6, 7, 8}), result);
        result.clear();

        // Full area
        area = sub_area(raster, Cell(0, 0), 4, 5);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({1, 2, 3, 4, 5, 6, 7, 8, 10, 14, 15, 19}), result);
        result.clear();

        // Top left area
        area = sub_area(raster, Cell(0, 0), 3, 2);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({1, 5, 6, 10}), result);
        result.clear();

        // Bottom right area, too large
        area = sub_area(raster, Cell(2, 3), 4, 4);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({14, 19}), result);
        result.clear();

        // Single cell
        area = sub_area(raster, Cell(1, 2), 1, 1);
        std::copy(area.begin(), area.end(), std::back_inserter(result));
        CHECK_CONTAINER_EQ(std::vector<T>({7}), result);
        result.clear();
    }

    SUBCASE("fill raster subarea")
    {
        // clang-format off
        auto data = convertTo<T>(std::vector<double>{{
            -1,  1,  2,  3,  4,
             5,  6,  7,  8, -1,
            10, 11, -1, 13, 14,
            15, -1, -1, -1, 19,
        }});

        Raster expected(RasterMetadata(4, 5, -1), convertTo<T>(std::vector<double>{{
            -1,  1,  2,  3,  4,
             5,  0,  0,  0, -1,
            10,  0, -1,  0, 14,
            15, -1, -1, -1, 19,
        }}));
        // clang-format on

        Raster raster(RasterMetadata(4, 5, -1), data);

        std::vector<T> result;

        auto area = sub_area(raster, Cell(1, 1), 2, 3);
        std::fill(area.begin(), area.end(), T(0));
        CHECK_RASTER_EQ(expected, raster);
    }
}
}
