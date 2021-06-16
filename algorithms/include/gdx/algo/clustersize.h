#pragma once

#include "gdx/algo/clusterutils.h"
#include "gdx/rasterchecks.h"

#include <cassert>
#include <limits>
#include <vector>

namespace gdx {

template <template <typename> typename RasterType, typename T>
RasterType<int32_t> cluster_size(const RasterType<T>& ras, ClusterDiagonals diagonals)
{
    show_warning_if_clustering_on_floats(ras);

    const auto rows = ras.rows();
    const auto cols = ras.cols();

    int32_t nodata  = -9999;
    auto resultMeta = ras.metadata();
    if (resultMeta.nodata.has_value()) {
        // Use -9999 as nodata as this cannot conflict with a result from the sum
        resultMeta.nodata = nodata;
    }

    RasterType<int32_t> result(resultMeta, 0);
    RasterType<uint8_t> mark(ras.metadata(), s_markTodo);
    std::vector<Cell> clusterCells;
    FiLo<Cell> border(rows, cols);
    uint32_t clusterCount = 0;

    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            if (ras.is_nodata(r, c)) {
                result.mark_as_nodata(r, c);
                continue;
            }

            if (ras(r, c) == 0) {
                result(r, c) = 0;
            } else if (ras(r, c) > 0 && mark(r, c) == s_markTodo) {
                ++clusterCount;
                int32_t sum = 0;
                clusterCells.clear();
                border.clear();
                const auto clusterValue = ras(r, c);

                // add current cell to the cluster
                insert_cell(Cell(r, c), clusterCells, mark, border);

                while (!border.empty()) {
                    auto cell            = border.pop_head();
                    mark(cell.r, cell.c) = s_markDone;
                    ++sum;

                    // add the four neighbouring cells if they have the same value
                    visit_neighbour_cells(cell, rows, cols, [&](const Cell& neighbour) {
                        handle_cell(neighbour, clusterValue, clusterCells, mark, border, ras);
                    });

                    if (diagonals == ClusterDiagonals::Include) {
                        visit_neighbour_diag_cells(cell, rows, cols, [&](const Cell& neighbour) {
                            handle_cell(neighbour, clusterValue, clusterCells, mark, border, ras);
                        });
                    }
                }

                for (auto& cell : clusterCells) {
                    result(cell.r, cell.c) = sum;
                    result.mark_as_data(cell);
                }
            }
        }
    }
    return result;
}

template <typename TResult, template <typename> typename RasterType, typename TCluster, typename TSum>
RasterType<TResult> cluster_sum(const RasterType<TCluster>& ras, const RasterType<TSum>& valueToSum, ClusterDiagonals diagonals)
{
    show_warning_if_clustering_on_floats(ras);
    throw_on_size_mismatch(ras, valueToSum);

    const auto rows = ras.rows();
    const auto cols = ras.cols();

    int32_t nodata  = -9999;
    auto resultMeta = ras.metadata();
    if (resultMeta.nodata.has_value()) {
        // Use -9999 as nodata as this cannot conflict with a result from the sum
        resultMeta.nodata = nodata;
    }

    RasterType<TResult> result(resultMeta, TResult(0));
    RasterType<uint8_t> mark(ras.metadata(), s_markTodo);
    std::vector<Cell> clusterCells;
    FiLo<Cell> border(rows, cols);
    uint32_t clusterCount = 0;

    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            if (ras.is_nodata(r, c)) {
                result.mark_as_nodata(r, c);
                continue;
            }

            if (ras(r, c) == 0) {
                result(r, c) = 0;
            } else if (ras(r, c) > 0 && mark(r, c) == s_markTodo) {
                ++clusterCount;

                clusterCells.clear();
                border.clear();
                const auto clusterValue = ras(r, c);

                // add current cell to the cluster
                insert_cell(Cell(r, c), clusterCells, mark, border);

                while (!border.empty()) {
                    auto cell            = border.pop_head();
                    mark(cell.r, cell.c) = s_markDone;

                    // add the four neighbouring cells if they have the same value
                    visit_neighbour_cells(cell, rows, cols, [&](const Cell& neighbour) {
                        handle_cell(neighbour, clusterValue, clusterCells, mark, border, ras);
                    });

                    if (diagonals == ClusterDiagonals::Include) {
                        visit_neighbour_diag_cells(cell, rows, cols, [&](const Cell& neighbour) {
                            handle_cell(neighbour, clusterValue, clusterCells, mark, border, ras);
                        });
                    }
                }

                TResult sum = 0;
                for (auto& cell : clusterCells) {
                    sum += static_cast<TResult>(valueToSum[cell]);
                }

                for (auto& cell : clusterCells) {
                    result(cell.r, cell.c) = sum;
                    result.mark_as_data(cell);
                }
            }
        }
    }
    return result;
}

}
