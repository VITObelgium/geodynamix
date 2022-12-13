#pragma once

#include "gdx/cell.h"
#include "gdx/exception.h"
#include "gdx/log.h"

#include "gdx/algo/clusterutils.h"
#include "gdx/algo/nodata.h"
#include "infra/chrono.h"

#include <cassert>
#include <chrono>
#include <cmath>
#include <limits>
#include <vector>

namespace gdx {

// on whole maps: targets and barriers outside modelling area do matter in case region map reduction is used instead zoning.
// for (int r = 0; r < gc->nRows; ++r) {
//     for (int c = 0; c < gc->nCols; ++c) {
//         if (ftarget(r, c) && !isnan(ftarget(r, c))) btarget(r, c) = 1;
//         if (fbarrier(r, c) || isnan(fbarrier(r, c))) bbarrier(r, c) = 1;
//     }
// }

enum class BarrierDiagonals
{
    Include, // Allow traveral through diagonal barriers
    Exclude,
};

namespace internal {

template <template <typename> typename RasterType, typename T>
void handle_cell_closest_target(float deltaD, const Cell& cell, const Cell& newCell,
                                RasterType<float>& distanceToTarget,
                                RasterType<T>& closesttarget,
                                RasterType<uint8_t>& mark,
                                FiLo<Cell>& border)
{
    if (distanceToTarget[newCell] > distanceToTarget[cell] + deltaD) {
        distanceToTarget[newCell] = distanceToTarget[cell] + deltaD;
        closesttarget[newCell]    = closesttarget[cell];
        if (mark[newCell] != s_markBorder) {
            mark[newCell] = s_markBorder;
            border.push_back(newCell);
        }
    }
}

template <template <typename> typename RasterType, typename T>
void handle_sum_le_time_distance_cell(float deltaD, const Cell& cell, const Cell& newCell,
                                      RasterType<float>& distanceToTarget,
                                      RasterType<uint8_t>& mark,
                                      const RasterType<T>& travelTime,
                                      FiLo<Cell>& border,
                                      std::vector<Cell>& cells)
{
    if (travelTime.is_nodata(newCell)) {
        return;
    }
    const float alternativeDist = static_cast<float>(distanceToTarget[cell] +
                                                     deltaD / 2.0f * (travelTime[cell] + travelTime[newCell]));
    float& d                    = distanceToTarget[newCell];
    if (d > alternativeDist) {
        d          = alternativeDist;
        uint8_t& m = mark[newCell];
        if (m != s_markBorder) {
            if (m == s_markTodo) {
                cells.push_back(newCell);
            }
            m = s_markBorder;
            border.push_back(newCell);
        }
    }
}

template <template <typename> typename RasterType, typename T>
void handle_cell_value_at_closest_target(float deltaD, const Cell& cell, const Cell& newCell,
                                         RasterType<float>& distanceToTarget,
                                         RasterType<uint8_t>& mark,
                                         RasterType<T>& valueatclosesttarget,
                                         FiLo<Cell>& border)
{
    if (distanceToTarget[newCell] > distanceToTarget[cell] + deltaD) {
        distanceToTarget[newCell]     = distanceToTarget[cell] + deltaD;
        valueatclosesttarget[newCell] = valueatclosesttarget[cell];
        if (mark[newCell] != s_markBorder) {
            mark[newCell] = s_markBorder;
            border.push_back(newCell);
        }
    }
}

template <template <typename> typename RasterType, typename TTravel, typename TValue>
void handle_cell_value_at_closest_travel_target(float deltaD, const Cell& cell, const Cell& newCell,
                                                RasterType<float>& distanceToTarget,
                                                RasterType<TValue>& valueatclosesttarget,
                                                const RasterType<TTravel>& travelTime,
                                                RasterType<uint8_t>& mark,
                                                FiLo<Cell>& border)
{
    auto alternativeDist = static_cast<float>(distanceToTarget[cell] + deltaD * travelTime[newCell]);
    if (distanceToTarget[newCell] > alternativeDist) {
        distanceToTarget[newCell]     = alternativeDist;
        valueatclosesttarget[newCell] = valueatclosesttarget[cell];
        if (mark[newCell] != s_markBorder) {
            mark[newCell] = s_markBorder;
            border.push_back(newCell);
        }
    }
}

template <template <typename> typename RasterType>
void handle_cell(float deltaD, const Cell& cell, const Cell& newCell,
                 RasterType<float>& distanceToTarget,
                 RasterType<uint8_t>& mark,
                 FiLo<Cell>& border)
{
    if (distanceToTarget[newCell] > distanceToTarget[cell] + deltaD) {
        distanceToTarget[newCell] = distanceToTarget[cell] + deltaD;
        if (mark[newCell] != s_markBorder) {
            mark[newCell] = s_markBorder;
            border.push_back(newCell);
        }
    }
}

template <template <typename> typename RasterType>
void handle_diagonal_cell(float deltaD, const Cell& cell, const Cell& newCell,
                          RasterType<float>& distanceToTarget,
                          RasterType<uint8_t>& mark,
                          FiLo<Cell>& border)
{
    if (distanceToTarget[newCell] > distanceToTarget[cell] + deltaD) {
        distanceToTarget[newCell] = distanceToTarget[cell] + deltaD;
        if (mark[newCell] != s_markBorder) {
            mark[newCell] = s_markBorder;
            border.push_back(newCell);
        }
    }
}

template <template <typename> typename RasterType>
void handle_cell_with_obstacles(float deltaD, const Cell& cell, const Cell& newCell,
                                const RasterType<uint8_t>& obstacles,
                                RasterType<float>& distanceToTarget,
                                RasterType<uint8_t>& mark,
                                FiLo<Cell>& border)
{
    if (((!obstacles.is_nodata(newCell)) && obstacles[newCell] == 0) && distanceToTarget[newCell] > distanceToTarget[cell] + deltaD) {
        distanceToTarget[newCell] = distanceToTarget[cell] + deltaD;
        if (mark[newCell] != s_markBorder) {
            mark[newCell] = s_markBorder;
            border.push_back(newCell);
        }
    }
}

template <template <typename> typename RasterType>
void handle_cell_with_obstacles_diag(float deltaD, const Cell& cell, const Cell& newCell,
                                     const RasterType<uint8_t>& obstacles,
                                     RasterType<float>& distanceToTarget,
                                     RasterType<uint8_t>& mark,
                                     FiLo<Cell>& border)
{
    if (obstacles.is_nodata(newCell) ||
        obstacles.is_nodata(cell.r, newCell.c) ||
        obstacles.is_nodata(newCell.r, cell.c)) {
        return;
    }

    if ((obstacles[newCell] == 0) &&
        !(obstacles(cell.r, newCell.c) != 0 && obstacles(newCell.r, cell.c) != 0) &&
        distanceToTarget[newCell] > distanceToTarget[cell] + deltaD) {
        distanceToTarget[newCell] = distanceToTarget[cell] + deltaD;
        if (mark[newCell] != s_markBorder) {
            mark[newCell] = s_markBorder;
            border.push_back(newCell);
        }
    }
}
}

template <template <typename> typename RasterType>
RasterType<float> distances_up_to(const RasterType<uint8_t>& target, const float unreachable)
{
    const auto rows = target.rows();
    const auto cols = target.cols();

    auto meta   = target.metadata();
    meta.nodata = RasterType<float>::NaN;
    RasterType<float> distanceToTarget(std::move(meta), unreachable);
    RasterType<uint8_t> mark(target.rows(), target.cols(), s_markTodo);

    FiLo<Cell> border(rows, cols);

    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            if (target.is_nodata(r, c)) {
                distanceToTarget.mark_as_nodata(r, c);
            } else if (target(r, c) != 0) {
                distanceToTarget(r, c) = 0;
                mark(r, c)             = s_markBorder;
                border.push_back(Cell(r, c));
            }
        }
    }

    const float sqrt2 = std::sqrt(2.f);
    while (!border.empty()) {
        auto cell = border.pop_head();
        assert(mark[cell] == s_markBorder);
        mark[cell] = s_markDone;

        visit_neighbour_cells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handle_cell(1.f, cell, neighbour, distanceToTarget, mark, border);
        });

        visit_neighbour_diag_cells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handle_cell(sqrt2, cell, neighbour, distanceToTarget, mark, border);
        });
    }

    distanceToTarget *= static_cast<float>(target.metadata().cellSize.x);
    return distanceToTarget;
}

template <template <typename> typename RasterType>
RasterType<float> distance(const RasterType<uint8_t>& target)
{
    const float unreachable = std::numeric_limits<float>::infinity();
    return distances_up_to(target, unreachable);
}

template <template <typename> typename RasterType, typename TTarget, typename TObstacles>
RasterType<float> distance(const RasterType<TTarget>& target, const RasterType<TObstacles>& obstacles, BarrierDiagonals diagonals = BarrierDiagonals::Exclude)
{
    throw_on_size_mismatch(target, obstacles);

    constexpr const float unreachable = std::numeric_limits<float>::infinity();

    auto meta   = target.metadata();
    meta.nodata = RasterType<float>::NaN;
    RasterType<float> distanceToTarget(meta, unreachable);
    RasterType<uint8_t> mark(target.rows(), target.cols(), s_markTodo);

    RasterType<uint8_t> byteTarget(meta.rows, meta.cols, 0);
    RasterType<uint8_t> byteObstacles(meta.rows, meta.cols, 0);

    const auto rows = meta.rows;
    const auto cols = meta.cols;
    FiLo<Cell> border(rows, cols);

    // on whole maps: targets and barriers outside modelling area do matter in case region map reduction is used instead zoning.
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (!target.is_nodata(r, c) && target(r, c) != 0) {
                byteTarget(r, c) = 1;
            }

            if (obstacles.is_nodata(r, c) || obstacles(r, c) != 0) {
                byteObstacles(r, c) = 1;
            }
        }
    }

    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            if (byteTarget.is_nodata(r, c)) {
                distanceToTarget(r, c) = 0;
            } else if (byteTarget(r, c) != 0) {
                distanceToTarget(r, c) = 0;
                mark(r, c)             = s_markBorder;
                border.push_back(Cell(r, c));
            }
        }
    }

    const float sqrt2 = std::sqrt(2.f);
    while (!border.empty()) {
        auto cell = border.pop_head();
        assert(mark[cell] == s_markBorder);
        mark[cell] = s_markDone;

        visit_neighbour_cells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handle_cell_with_obstacles(1.f, cell, neighbour, byteObstacles, distanceToTarget, mark, border);
        });

        if (diagonals == BarrierDiagonals::Include) {
            visit_neighbour_diag_cells(cell, rows, cols, [&](const Cell& neighbour) {
                internal::handle_cell_with_obstacles(sqrt2, cell, neighbour, byteObstacles, distanceToTarget, mark, border);
            });
        } else {
            assert(diagonals == BarrierDiagonals::Exclude);
            visit_neighbour_diag_cells(cell, rows, cols, [&](const Cell& neighbour) {
                internal::handle_cell_with_obstacles_diag(sqrt2, cell, neighbour, byteObstacles, distanceToTarget, mark, border);
            });
        }
    }

    distanceToTarget *= static_cast<float>(meta.cellSize.x);

    return distanceToTarget;
}

template <template <typename> typename RasterType, typename T>
RasterType<float> travel_distances_up_to(const RasterType<uint8_t>& target, const RasterType<T>& travelTime, const float unreachable)
{
    if (target.size() != travelTime.size()) {
        throw InvalidArgument("Target raster dimensions should match travel time raster dimensions");
    }

    const auto rows = target.rows();
    const auto cols = target.cols();

    auto meta   = target.metadata();
    meta.nodata = RasterType<float>::NaN;
    RasterType<float> distanceToTarget(std::move(meta), unreachable);
    RasterType<uint8_t> mark(target.metadata(), s_markTodo);

    FiLo<Cell> border(rows, cols);

    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            if (target.is_nodata(r, c) || travelTime.is_nodata(r, c)) {
                distanceToTarget.mark_as_nodata(r, c);
            } else if (target(r, c) != 0) {
                distanceToTarget(r, c) = 0;
                mark(r, c)             = s_markBorder;
                border.push_back(Cell(r, c));
            }
        }
    }

    const float sqrt2 = std::sqrt(2.f);
    while (!border.empty()) {
        auto cell = border.pop_head();
        assert(mark[cell] == s_markBorder);
        mark[cell] = s_markDone;

        visit_neighbour_cells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handle_time_cell(1.f, cell, neighbour, distanceToTarget, mark, travelTime, border);
        });

        visit_neighbour_diag_cells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handle_time_cell(sqrt2, cell, neighbour, distanceToTarget, mark, travelTime, border);
        });
    }

    return distanceToTarget;
}

template <template <typename> typename RasterType, typename T>
RasterType<float> travel_distance(const RasterType<uint8_t>& target, const RasterType<T>& travelTime)
{
    const float unreachable = std::numeric_limits<float>::max();
    return travel_distances_up_to(target, travelTime, unreachable);
}

template <template <typename> typename RasterType, typename T>
RasterType<T> closest_target(const RasterType<T>& target)
{
    const auto rows         = target.rows();
    const auto cols         = target.cols();
    const float unreachable = std::numeric_limits<float>::max();

    auto meta = target.metadata();
    meta.nodata.reset();
    RasterType<float> distanceToTarget(meta, unreachable);
    RasterType<T> closestTarget(meta, 0);
    RasterType<uint8_t> mark(meta, s_markTodo);

    FiLo<Cell> border(rows, cols);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (target.is_nodata(r, c)) {
                continue;
            } else if (target(r, c)) {
                distanceToTarget(r, c) = 0;
                closestTarget(r, c)    = target(r, c);
                mark(r, c)             = s_markBorder;
                border.push_back(Cell(r, c));
            }
        }
    }

    const float sqrt2 = std::sqrt(2.f);
    while (!border.empty()) {
        auto cell = border.pop_head();
        assert(mark[cell] == s_markBorder);
        mark[cell] = s_markDone;

        visit_neighbour_cells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handle_cell_closest_target(1.f, cell, neighbour, distanceToTarget, closestTarget, mark, border);
        });

        visit_neighbour_diag_cells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handle_cell_closest_target(sqrt2, cell, neighbour, distanceToTarget, closestTarget, mark, border);
        });
    }

    return closestTarget;
}

template <template <typename> typename RasterType, typename TValue, typename TTarget>
RasterType<TValue> value_at_closest_target(const RasterType<TTarget>& target, const RasterType<TValue>& value)
{
    if (target.size() != value.size()) {
        throw InvalidArgument("Target raster dimensions should match value raster dimensions");
    }

    const auto rows         = target.rows();
    const auto cols         = target.cols();
    const float unreachable = static_cast<float>(rows * cols + 1);

    RasterType<TValue> valueAtClosestTarget(value.metadata(), 0);
    RasterType<float> distanceToTarget(value.metadata(), unreachable);

    RasterType<uint8_t> mark(target.metadata(), s_markTodo);
    FiLo<Cell> border(rows, cols);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (target.is_nodata(r, c)) {
                continue;
            }

            if (target(r, c)) {
                distanceToTarget(r, c) = 0;

                if (value.is_nodata(r, c)) {
                    valueAtClosestTarget.mark_as_nodata(r, c);
                } else {
                    valueAtClosestTarget(r, c) = value(r, c);
                }

                mark(r, c) = s_markBorder;
                border.push_back(Cell(r, c));
            }
        }
    }

    const float sqrt2 = std::sqrt(2.f);
    while (!border.empty()) {
        auto cell = border.pop_head();
        assert(mark[cell] == s_markBorder);
        mark[cell] = s_markDone;

        visit_neighbour_cells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handle_cell_value_at_closest_target(1.f, cell, neighbour, distanceToTarget, mark, valueAtClosestTarget, border);
        });

        visit_neighbour_diag_cells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handle_cell_value_at_closest_target(sqrt2, cell, neighbour, distanceToTarget, mark, valueAtClosestTarget, border);
        });
    }

    return valueAtClosestTarget;
}

template <template <typename> typename RasterType, typename TValue, typename TTravel, typename TTarget>
RasterType<TValue> value_at_closest_travel_target(const RasterType<TTarget>& target, const RasterType<TTravel>& travelTimes, const RasterType<TValue>& value)
{
    if (target.size() != value.size() || target.size() != value.size()) {
        throw InvalidArgument("Target, traveltimes and value map dimensions should be the same");
    }

    const auto rows         = target.rows();
    const auto cols         = target.cols();
    const float unreachable = std::numeric_limits<float>::max();

    RasterType<TValue> valueAtClosestTarget(value.metadata(), 0);
    RasterType<float> distanceToTarget(value.metadata(), unreachable);

    RasterType<uint8_t> mark(target.metadata(), s_markTodo);
    FiLo<Cell> border(rows, cols);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (value.is_nodata(r, c)) {
                valueAtClosestTarget(r, c) = value(r, c);
                distanceToTarget.mark_as_nodata(r, c);
            } else if (target(r, c)) {
                distanceToTarget(r, c)     = 0;
                valueAtClosestTarget(r, c) = value(r, c);
                mark(r, c)                 = s_markBorder;
                border.push_back(Cell(r, c));
            }
        }
    }

    const float sqrt2 = std::sqrt(2.f);
    while (!border.empty()) {
        auto cell = border.pop_head();
        assert(mark[cell] == s_markBorder);
        mark[cell] = s_markDone;

        visit_neighbour_cells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handle_cell_value_at_closest_travel_target(1.f, cell, neighbour, distanceToTarget, valueAtClosestTarget, travelTimes, mark, border);
        });

        visit_neighbour_diag_cells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handle_cell_value_at_closest_travel_target(sqrt2, cell, neighbour, distanceToTarget, valueAtClosestTarget, travelTimes, mark, border);
        });
    }

    return valueAtClosestTarget;
}

template <template <typename> typename RasterType, typename TValue, typename TTravel, typename TTarget>
RasterType<TValue> value_at_closest_less_then_travel_target(const RasterType<TTarget>& target, const RasterType<TTravel>& travelTimes, const float maxTravelTime, const RasterType<TValue>& value)
{
    if (target.size() != value.size() || target.size() != value.size()) {
        throw InvalidArgument("Target, traveltimes and value map dimensions should be the same");
    }

    const auto rows         = target.rows();
    const auto cols         = target.cols();
    const float unreachable = maxTravelTime;

    RasterType<TValue> valueAtClosestTarget(value.metadata(), 0);
    RasterType<float> distanceToTarget(value.metadata(), unreachable);

    RasterType<uint8_t> mark(target.metadata(), s_markTodo);
    FiLo<Cell> border(rows, cols);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (value.is_nodata(r, c)) {
                valueAtClosestTarget(r, c) = value(r, c);
                distanceToTarget.mark_as_nodata(r, c);
            } else if (target(r, c)) {
                distanceToTarget(r, c)     = 0;
                valueAtClosestTarget(r, c) = value(r, c);
                mark(r, c)                 = s_markBorder;
                border.push_back(Cell(r, c));
            }
        }
    }

    const float sqrt2 = std::sqrt(2.f);
    while (!border.empty()) {
        auto cell = border.pop_head();
        assert(mark[cell] == s_markBorder);
        mark[cell] = s_markDone;

        visit_neighbour_cells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handle_cell_value_at_closest_travel_target(1.f, cell, neighbour, distanceToTarget, valueAtClosestTarget, travelTimes, mark, border);
        });

        visit_neighbour_diag_cells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handle_cell_value_at_closest_travel_target(sqrt2, cell, neighbour, distanceToTarget, valueAtClosestTarget, travelTimes, mark, border);
        });
    }

    return valueAtClosestTarget;
}

// computes the sum of the valueToSum that is within the distance via lowest travelTime
template <template <typename> typename RasterType, typename TTravel, typename TValue>
TValue compute_sum_le_time_distance(Cell targetCell,
                                    const RasterType<TTravel>& travelTime,
                                    const float maxTravelTime,
                                    const float unreachable,
                                    const RasterType<TValue>& valueToSum,
                                    bool inclAdjacent,
                                    // temporary variables internal to compute_sum_le_time_distance.  Pass them again and again instead of having to recreate them again and again.
                                    RasterType<float>& distanceToTarget, // expected to be all unreachable.  This function restores any changes upon return.
                                    RasterType<uint8_t>& mark,           // expected to be all s_markTodo.  This function restores any changes upon return.
                                    FiLo<Cell>& border,                  // expected to be empty.  Restored to empty upon return.
                                    std::vector<Cell>& cells,            // idem
                                    std::vector<Cell>& adjacentCells     // idem
)
{
    // preconditions on parameters :
    // distanceToTarget all unreachable
    // mark all s_markTodo
    // border empty
    // cells empty

    TValue sum = 0;

    const auto rows = mark.rows();
    const auto cols = mark.cols();

    Cell cell              = targetCell;
    distanceToTarget[cell] = 0;
    if (!travelTime.is_nodata(cell)) {
        border.push_back(cell);
        mark[cell] = s_markBorder;
    } else {
        mark[cell] = s_markDone;
    }
    cells.push_back(cell);

    const float sqrt2 = std::sqrt(2.f);
    while (!border.empty()) {
        auto curCell = border.pop_head();
        assert(mark[curCell] == s_markBorder);
        mark[curCell] = s_markDone;

        visit_neighbour_cells(curCell, rows, cols, [&](const Cell& neighbour) {
            internal::handle_sum_le_time_distance_cell(1.f, curCell, neighbour, distanceToTarget, mark, travelTime, border, cells);
        });

        visit_neighbour_diag_cells(curCell, rows, cols, [&](const Cell& neighbour) {
            internal::handle_sum_le_time_distance_cell(sqrt2, curCell, neighbour, distanceToTarget, mark, travelTime, border, cells);
        });
    }

    // cells contain all the locations at <= radius now.
    // All modified locations can be restored to their original state efficiently now.
    for (int i = 0; i < int(cells.size()); ++i) {
        const int r = cells[i].r;
        const int c = cells[i].c;
        assert(distanceToTarget(r, c) <= maxTravelTime);
        if (!valueToSum.is_nodata(r, c)) {
            sum += valueToSum(r, c);
        }
        assert(mark(r, c) == s_markDone);
        mark(r, c) = s_markTodo;
    }

    if (inclAdjacent) {
        assert(adjacentCells.size() == 0);
        auto handleAdjacent = [&](int rr, int cc) {
            if (0 <= rr && rr < rows && 0 <= cc && cc < cols) {
                if (distanceToTarget(rr, cc) > maxTravelTime && mark(rr, cc) == s_markTodo) {
                    if (!valueToSum.is_nodata(rr, cc)) {
                        sum += valueToSum(rr, cc);
                    }
                    mark(rr, cc) = s_markDone;
                    adjacentCells.push_back(Cell(rr, cc));
                }
            }
        };
        for (int i = 0; i < int(cells.size()); ++i) {
            const int r = cells[i].r;
            const int c = cells[i].c;
            handleAdjacent(r - 1, c);
            handleAdjacent(r + 1, c);
            handleAdjacent(r, c - 1);
            handleAdjacent(r, c + 1);
        }
    }
    for (int i = 0; i < int(cells.size()); ++i) {
        distanceToTarget[cells[i]] = unreachable;
    }
    for (int i = 0; i < int(adjacentCells.size()); ++i) {
        mark[adjacentCells[i]] = s_markTodo;
    }
    cells.clear();
    adjacentCells.clear();
    return sum;
}

template <typename TResult, template <typename> typename RasterType, typename TMask, typename TResistence, typename TValue>
RasterType<TResult> sum_within_travel_distance(const RasterType<TMask>& mask,
                                               const RasterType<TResistence>& resistenceMap,
                                               const RasterType<TValue>& valueMap,
                                               float maxResistance,
                                               bool includeAdjacent)
{
    if (mask.size() != resistenceMap.size() || mask.size() != valueMap.size()) {
        throw InvalidArgument("Mask, resistence and value map dimensions should be the same");
    }
    if (maxResistance <= 0) {
        throw InvalidArgument("maxResistance should be postive");
    }
    for (int i = 0; i < int(resistenceMap.size()); ++i) {
        if (!resistenceMap.is_nodata(i) && resistenceMap[i] < 0) {
            throw InvalidArgument("resistance may not be negative");
        }
    }

    const auto rows = mask.rows();
    const auto cols = mask.cols();

    auto resultMeta = valueMap.metadata();

    RasterType<TResult> result(resultMeta, 0);
    if constexpr (result.has_nan()) {
        result.set_nodata(result.NaN);
    }

    const float unreachable = std::nextafter(maxResistance, std::numeric_limits<float>::max());

    // temporary variables internal to compute_sum_le_time_distance.  Pass them again and again instead of having to recreate them again and again.
    RasterType<float> distanceToTarget(mask.metadata(), unreachable);
    RasterType<uint8_t> mark(mask.metadata(), s_markTodo);
    FiLo<Cell> border(rows, cols);
    std::vector<Cell> cells;
    std::vector<Cell> adjacentCells;

    using namespace std::chrono_literals;
    auto t00     = std::chrono::steady_clock::now();
    auto lastMsg = t00;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (!mask.is_nodata(r, c) && mask(r, c) != 0) {
                result(r, c) = static_cast<TResult>(compute_sum_le_time_distance(Cell(r, c), resistenceMap, maxResistance,
                                                                                 unreachable, valueMap, includeAdjacent, distanceToTarget, mark, border, cells, adjacentCells));
            }
        }

        auto now = std::chrono::steady_clock::now();
        if ((now - lastMsg) > 3s) {
            auto elapsed = now - t00;
            auto total   = elapsed * static_cast<double>(rows) / (r + 1);

            lastMsg = now;
            Log::warn("sum_within_travel_distance processed {:.2f}%, elapsed {:02}:{:02}:{:02}, expected total runtime {:02}:{:02}:{:02}",
                      100.0 * (r + 1) / rows,
                      std::chrono::duration_cast<std::chrono::hours>(elapsed).count(),
                      std::chrono::duration_cast<std::chrono::minutes>(elapsed).count() % 60,
                      std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() % 60,
                      std::chrono::duration_cast<std::chrono::hours>(total).count(),
                      std::chrono::duration_cast<std::chrono::minutes>(total).count() % 60,
                      std::chrono::duration_cast<std::chrono::seconds>(total).count() % 60);
        }
    }

    return result;
}

template <typename TResult, template <typename> typename RasterType, typename TTarget, typename TResistance>
RasterType<TResult> sum_targets_within_travel_distance(const RasterType<TTarget>& targets,
                                                       const RasterType<TResistance>& resistanceMap,
                                                       float maxResistance)
{
    if (targets.size() != resistanceMap.size()) {
        throw inf::InvalidArgument("Targets and resistence map dimensions should be the same");
    }

    if (maxResistance <= 0) {
        throw inf::InvalidArgument("maxResistance should be postive");
    }

    for (size_t i = 0; i < resistanceMap.size(); ++i) {
        if (!resistanceMap.is_nodata(i) && resistanceMap[i] < 0) {
            throw inf::InvalidArgument("resistance may not be negative");
        }
    }

    auto resultMeta = targets.metadata();
    RasterType<TResult> result(resultMeta, 0);
    if constexpr (result.has_nan()) {
        result.set_nodata(result.NaN);
    }

    const auto rows         = targets.rows();
    const auto cols         = targets.cols();
    const float unreachable = maxResistance + 1;
    const float sqrt2       = std::sqrt(2.f);

    // FLT_MAX/4 allows to add 2 x sqrt(2) of them and still be less than FLT_MAX
    const auto resistance = gdx::replace_nodata<RasterType, TResistance>(resistanceMap, std::numeric_limits<TResistance>::max() / 4);

    resultMeta.nodata.reset();
    RasterType<float> distanceToTarget(resultMeta, unreachable);
    RasterType<uint8_t> mark(resultMeta, s_markTodo);
    RasterType<uint8_t> added(resultMeta, 0);
    FiLo<Cell> border(rows, cols);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (targets.is_nodata(r, c) || targets(r, c) == 0) {
                continue;
            }

            distanceToTarget.fill(unreachable);
            mark.fill(s_markTodo);
            added.fill(0);
            assert(border.empty());

            Cell cell(r, c);
            distanceToTarget[cell] = 0;
            if (!resistanceMap.is_nodata(cell)) {
                border.push_back(cell);
                mark[cell] = s_markBorder;
            } else {
                mark[cell] = s_markDone;
            }

            while (!border.empty()) {
                auto curCell = border.pop_head();
                assert(mark[curCell] == s_markBorder);
                mark[curCell] = s_markDone;
                if (distanceToTarget[curCell] <= maxResistance) {
                    // we can get here via different routes within the maxTravelTime.  But count only once.
                    if (added[curCell] == 0) {
                        result[curCell] += static_cast<TResult>(targets[cell]);
                        added[curCell] = 1;
                    }
                }

                visit_neighbour_cells(curCell, rows, cols, [&](const Cell& neighbour) {
                    internal::handle_time_cell(1.f, curCell, neighbour, distanceToTarget, mark, resistance, border);
                });

                visit_neighbour_diag_cells(curCell, rows, cols, [&](const Cell& neighbour) {
                    internal::handle_time_cell(sqrt2, curCell, neighbour, distanceToTarget, mark, resistance, border);
                });
            }
        }
    }

    return result;
}
}
