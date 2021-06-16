#pragma once

#include "gdx/algo/clusterutils.h"
#include "gdx/exception.h"

#include <cassert>
#include <limits>
#include <map>
#include <vector>

namespace gdx {

template <template <typename> typename RasterType, typename T>
RasterType<int32_t> cluster_id(const RasterType<T>& ras, ClusterDiagonals diagonals)
{
    show_warning_if_clustering_on_floats(ras);

    const auto rows = ras.rows();
    const auto cols = ras.cols();

    auto resultMeta = ras.metadata();
    // use -9999 as nodata as it cannot clash with a cluster id
    int32_t nodata = -9999;
    if (resultMeta.nodata.has_value()) {
        resultMeta.nodata = nodata;
    }

    RasterType<int32_t> result(resultMeta);
    RasterType<uint8_t> mark(ras.rows(), ras.cols(), s_markTodo);
    std::vector<Cell> clusterCells;
    FiLo<Cell> border(rows, cols);

    int32_t clusterId = 0;
    for (int32_t r = 0; r < ras.rows(); ++r) {
        for (int32_t c = 0; c < ras.cols(); ++c) {
            if (ras.is_nodata(r, c)) {
                result.mark_as_nodata(r, c);
                continue;
            }

            if (ras(r, c) == 0) {
                result(r, c) = 0;
            } else if (ras(r, c) > 0 && mark(r, c) == s_markTodo) {
                ++clusterId;

                clusterCells.clear();
                border.clear();

                const auto clusterValue = ras(r, c);

                // add current cell to the cluster
                insert_cell(Cell(r, c), clusterCells, mark, border);

                while (!border.empty()) {
                    auto cell = border.pop_head();

                    visit_neighbour_cells(cell, rows, cols, [&](const Cell& neighbour) {
                        handle_cell(neighbour, clusterValue, clusterCells, mark, border, ras);
                    });

                    if (diagonals == ClusterDiagonals::Include) {
                        visit_neighbour_diag_cells(cell, rows, cols, [&](const Cell& neighbour) {
                            handle_cell(neighbour, clusterValue, clusterCells, mark, border, ras);
                        });
                    }
                }

                for (const auto& cell : clusterCells) {
                    mark(cell.r, cell.c)   = s_markDone;
                    result(cell.r, cell.c) = clusterId;
                }
            }
        }
    }

    return result;
}

template <template <typename> typename RasterType, typename T>
RasterType<int32_t> fuzzy_cluster_id(const RasterType<T>& ras, float radiusInMeter)
{
    const auto rows = ras.rows();
    const auto cols = ras.cols();

    float radius             = radiusInMeter / static_cast<float>(ras.metadata().cellSize);
    const auto radiusInCells = static_cast<int>(radius);
    const auto radius2       = static_cast<int32_t>(radius * radius);

    auto resultMeta = ras.metadata();
    // use -9999 as nodata as it cannot clash with a cluster id
    int32_t nodata = -9999;
    if (resultMeta.nodata.has_value()) {
        resultMeta.nodata = nodata;
    }

    RasterType<int32_t> result(resultMeta);
    RasterType<uint8_t> mark(ras.rows(), ras.cols(), s_markDone);

    for (std::size_t i = 0; i < ras.size(); ++i) {
        if (ras.is_nodata(i)) {
            mark[i] = s_markDone;
            result.mark_as_nodata(i);
            continue;
        }

        if (ras[i] > 0) {
            mark[i] = s_markTodo;
        } else {
            result[i] = 0;
        }
    }

    int32_t clusterId = 0;
    FiLo<Cell> border(rows, cols);

    for (int32_t r = 0; r < ras.rows(); ++r) {
        for (int32_t c = 0; c < ras.cols(); ++c) {
            if (mark(r, c) == s_markTodo) {
                ++clusterId;

                border.clear();
                border.push_back(Cell(r, c));
                mark(r, c) = s_markBorder;
                while (!border.empty()) {
                    auto cell              = border.pop_head();
                    mark(cell.r, cell.c)   = s_markDone;
                    result(cell.r, cell.c) = clusterId;

                    const int r0 = (cell.r - radiusInCells < 0 ? 0 : cell.r - radiusInCells);
                    const int c0 = (cell.c - radiusInCells < 0 ? 0 : cell.c - radiusInCells);
                    const int r1 = (cell.r + radiusInCells > rows - 1 ? rows - 1 : cell.r + radiusInCells);
                    const int c1 = (cell.c + radiusInCells > cols - 1 ? cols - 1 : cell.c + radiusInCells);

                    for (int32_t rr = r0; rr <= r1; ++rr) {
                        const auto dr  = rr - cell.r;
                        const auto dr2 = dr * dr;

                        auto markIndex = rr * cols + c0;
                        for (int32_t cc = c0; cc <= c1; ++markIndex, ++cc) {
                            if (mark[markIndex] == s_markTodo) {
                                const int dc = cc - cell.c;
                                if (dr2 + dc * dc <= radius2) {
                                    mark[markIndex] = s_markBorder;
                                    border.push_back(Cell(rr, cc));
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return result;
}

template <template <typename> typename RasterType>
void handle_cell_with_obstacles_straight(const Cell cell,
    const RasterType<int32_t>& catMap,
    const int clusterValue,
    const RasterType<uint8_t>& obstacleMap,
    RasterType<uint8_t>& mark,
    FiLo<Cell>& border)
{
    if ((catMap[cell] == clusterValue) && (mark[cell] == s_markTodo)) {
        if (!obstacleMap[cell]) {
            insert_cell(cell, mark, border);
        }
    }
}

template <template <typename> typename RasterType>
void handle_cell_with_obstacles_diag(const Cell oldCell, const Cell cell,
    const RasterType<int32_t>& catMap,
    const int clusterValue,
    const RasterType<uint8_t>& obstacleMap,
    RasterType<uint8_t>& mark,
    FiLo<Cell>& border)
{
    if ((catMap[cell] == clusterValue) && (mark[cell] == s_markTodo)) {
        if ((!obstacleMap[cell]) && ((!obstacleMap(oldCell.r, cell.c)) || (!obstacleMap(cell.r, oldCell.c)))) {
            insert_cell(cell, mark, border);
        }
    }
}

template <template <typename> typename RasterType>
void compute_cluster_id_of_obstacle_cell(int32_t row, int32_t col, RasterType<int32_t>& clusterIdMap, const RasterType<uint8_t>& obstacleMap, std::vector<int32_t>& clusterSize)
{
    const auto rows = clusterIdMap.rows();
    const auto cols = clusterIdMap.cols();

    // assign to cluster with largest nr of neighbors
    // in case of equal number of neighbors, assign to SMALLEST neighboring cluster
    std::map<int32_t, int32_t> countNeighbors; // neighboring clusterId --> nr of neighbors
    for (int32_t r = row - 1; r <= row + 1; ++r) {
        for (int32_t c = col - 1; c <= col + 1; ++c) {
            if (0 <= r && r < rows && 0 <= c && c < cols) {
                if (obstacleMap.is_nodata(r, c) || obstacleMap(r, c) == 0) {
                    const int clusterId = clusterIdMap(r, c);
                    if (clusterId > 0) {
                        const auto& iter = countNeighbors.find(clusterId);
                        if (iter != countNeighbors.end()) {
                            ++iter->second;
                        } else {
                            countNeighbors[clusterId] = 1;
                        }
                    }
                }
            }
        }
    }

    int clusterId = -1;
    int mostCount = 9;

    for (auto iter = countNeighbors.begin(); iter != countNeighbors.end(); ++iter) {
        if (clusterId == -1 || mostCount < iter->second) {
            clusterId = iter->first;
            mostCount = iter->second;
        } else if ((mostCount == iter->second) && (clusterSize[clusterId] > clusterSize[iter->first])) {
            clusterId = iter->first;
        }
    }

    if (clusterId > 0) {
        clusterIdMap(row, col) = clusterId;
        clusterIdMap.mark_as_data(row, col);
        ++clusterSize[clusterId];
    } else if ((!obstacleMap.is_nodata(row, col)) && obstacleMap(row, col) > 0) {
        clusterIdMap(row, col) = 0;
        clusterIdMap.mark_as_data(row, col);
    }
}

template <template <typename> typename RasterType>
RasterType<int32_t> cluster_id_with_obstacles(
    const RasterType<int32_t>& catMap,
    const RasterType<uint8_t>& obstacleMap)
{
    if (catMap.size() != obstacleMap.size()) {
        throw InvalidArgument("Raster, cathegory and obstacle map dimensions should be the same");
    }

    const auto rows = catMap.rows();
    const auto cols = catMap.cols();

    auto resultMeta = catMap.metadata();
    // use -9999 as nodata as it cannot clash with a cluster id
    int32_t nodata = -9999;
    if (resultMeta.nodata.has_value()) {
        resultMeta.nodata = nodata;
    }

    RasterType<int32_t> result(resultMeta, nodata);
    RasterType<uint8_t> mark(rows, cols, s_markTodo);

    int32_t clusterId = 0;
    FiLo<Cell> border(rows, cols);

    std::vector<int32_t> clusterSize(rows * cols, 0);
    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            if (!catMap.is_nodata(r, c) && !obstacleMap.is_nodata(r, c)) {
                if (catMap(r, c) > 0 && mark(r, c) == s_markTodo && !obstacleMap(r, c)) {
                    ++clusterId;
                    border.clear();
                    const auto clusterValue = catMap(r, c);
                    insert_cell(Cell(r, c), mark, border);

                    while (!border.empty()) {
                        auto cell            = border.pop_head();
                        mark(cell.r, cell.c) = s_markDone;
                        result[cell]         = clusterId;
                        result.mark_as_data(cell);

                        visit_neighbour_cells(cell, rows, cols, [&](const Cell& neighbour) {
                            handle_cell_with_obstacles_straight(neighbour, catMap, clusterValue, obstacleMap, mark, border);
                        });

                        visit_neighbour_diag_cells(cell, rows, cols, [&](const Cell& neighbour) {
                            handle_cell_with_obstacles_diag(cell, neighbour, catMap, clusterValue, obstacleMap, mark, border);
                        });
                    }
                }
            }
        }
    }

    // give remaining cells under obstacles a clusterId
    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            if (!catMap.is_nodata(r, c) && !obstacleMap.is_nodata(r, c)) {
                if (catMap(r, c) > 0 && obstacleMap(r, c) > 0) {
                    assert(mark(r, c) == s_markTodo);
                    compute_cluster_id_of_obstacle_cell(r, c, result, obstacleMap, clusterSize);
                }
            }
        }
    }

    return result;
}

template <template <typename> typename RasterType>
bool is_blocked(Cell from, bool diagonal, Cell to, const RasterType<uint8_t>& obstacle)
{
    if (diagonal) {
        return obstacle[to] || (obstacle(from.r, to.c) && obstacle(to.r, from.c));
    } else {
        // straight
        return obstacle[to] != 0;
    }
}

template <template <typename> typename RasterType>
bool is_blocked_path(Cell from, Cell to, const RasterType<uint8_t>& obstacle)
{
    int row = from.r;
    int col = from.c;
    while (row != to.r || col != to.c) {
        int dr = to.r - row;
        if (dr > 1) dr = 1;
        if (dr < -1) dr = -1;
        int dc = to.c - col;
        if (dc > 1) dc = 1;
        if (dc < -1) dc = -1;
        bool diagonal = ((std::abs(dr) + std::abs(dc)) > 1);

        if (row + dr == to.r && col + dc == to.c) {
            return is_blocked(Cell(row, col), diagonal, Cell(row + dr, col + dc), obstacle);
        } else {
            if (is_blocked(Cell(row, col), diagonal, Cell(row + dr, col + dc), obstacle)) {
                return true;
            }
        }
        row += dr;
        col += dc;
    }

    return false;
}

template <template <typename> typename RasterType>
void compute_fuzzy_cluster_id_with_obstacles_rc(Cell cell,
    const RasterType<int32_t>& items, const RasterType<int32_t>& backgroundId,
    const RasterType<uint8_t>& obstacles, int nRows, int nCols,
    float radius, int clusterId, RasterType<uint8_t>& mark, FiLo<Cell>& border,
    RasterType<int32_t>& result)
{
    assert(mark[cell] == s_markTodo);
    mark[cell] = s_markBorder;
    assert(border.empty());
    border.push_back(cell);
    int r0, c0, r1, c1;
    while (border.size() > 0) {
        Cell c = border.pop_head();
        assert(mark[c] == s_markBorder);
        mark[c] = s_markDone;
        assert(result[cell] == 0);
        result[c] = clusterId;
        result.mark_as_data(c);

        r0 = c.r - int(radius + 0.5f);
        c0 = c.c - int(radius + 0.5f);
        r1 = c.r + int(radius + 0.5f);
        c1 = c.c + int(radius + 0.5f);
        r0 = std::max(r0, 0);
        c0 = std::max(c0, 0);
        r1 = std::min(r1, nRows - 1);
        c1 = std::min(c1, nCols - 1);

        for (int rr = r0; rr <= r1; ++rr) {
            for (int cc = c0; cc <= c1; ++cc) {
                const int dr = rr - c.r;
                const int dc = cc - c.c;
                if (dr * dr + dc * dc <= int(radius * radius)) {
                    const Cell clcl(rr, cc);
                    if ((items[clcl] == items[cell]) && (backgroundId[clcl] == backgroundId[cell]) && (mark[clcl] == s_markTodo)) {
                        if (!is_blocked_path(c, clcl, obstacles)) {
                            assert(result[clcl] == 0);
                            mark[clcl] = s_markBorder;
                            border.push_back(clcl);
                        }
                    }
                }
            }
        }
    }
}

template <template <typename> typename RasterType>
RasterType<int32_t> fuzzy_cluster_id_with_obstacles(const RasterType<int32_t>& items, const RasterType<uint8_t>& obstacles, float radiusInMeter)
{
    auto backgroundId = cluster_id_with_obstacles(RasterType<int32_t>(items.metadata(), 1), obstacles);

    const auto rows    = items.rows();
    const auto cols    = items.cols();
    auto resultMeta    = items.metadata();
    const float radius = radiusInMeter / static_cast<float>(resultMeta.cellSize);
    // use -9999 as nodata as it cannot clash with a cluster id
    int32_t nodata = -9999;
    if (resultMeta.nodata.has_value()) {
        resultMeta.nodata = nodata;
    }
    RasterType<int32_t> result(resultMeta, nodata);

    RasterType<uint8_t> mark(resultMeta, s_markTodo);
    FiLo<Cell> border(resultMeta.rows, resultMeta.cols);
    int clusterId = 1;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            Cell cell(r, c);
            if (items.is_nodata(cell)) {
                mark[cell] = s_markDone;
                result.mark_as_nodata(cell);
                continue;
            }

            if (items[cell] > 0 && (mark[cell] == s_markTodo)) {
                if (obstacles[cell]) {
                    mark[cell]   = s_markDone;
                    result[cell] = clusterId;
                    result.mark_as_data(cell);
                } else {
                    compute_fuzzy_cluster_id_with_obstacles_rc(cell, items, backgroundId, obstacles, rows, cols, radius, clusterId, mark, border, result);
                }
                ++clusterId;
            }
        }
    }

    return result;
}
}
