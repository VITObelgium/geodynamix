#pragma once

#include "gdx/cell.h"
#include "gdx/exception.h"
#include "gdx/log.h"

#include <cassert>
#include <limits>
#include <vector>

namespace gdx {

static constexpr uint8_t s_markTodo(0);
static constexpr uint8_t s_markBorder(1);
static constexpr uint8_t s_markDone(2);

enum class ClusterDiagonals
{
    Include,
    Exclude
};

template <typename T>
class FiLo
{
public:
    FiLo(int nRows, int nCols)
    : head(0)
    , tail(0)
    , filo((static_cast<size_t>(nRows) * static_cast<size_t>(nCols)) + 1)
    {
    }

    void reserve_space(int nRows, int nCols)
    {
        filo.resize((static_cast<size_t>(nRows) * static_cast<size_t>(nCols)) + 1);
    }

    void clear()
    {
        head = tail = 0;
    }

    int size() const noexcept
    {
        return (tail + filo.size() - head) % int(filo.size());
    }

    bool empty() const noexcept
    {
        return tail == head;
    }

    void push_back(T value)
    {
        filo[tail] = value;
        tail       = (tail + 1) % filo.size();

        if (tail == head) {
            throw RuntimeError("FiLo overflow");
        }
    }

    T pop_head()
    {
        assert(!empty());
        auto ret = filo[head];
        head     = (head + 1) % filo.size();
        return ret;
    }

private:
    int head, tail;
    std::vector<T> filo;
};

template <template <typename> typename RasterType>
inline void insert_cell(const Cell& cell, std::vector<Cell>& clusterCells, RasterType<uint8_t>& mark, FiLo<Cell>& border)
{
    mark(cell.r, cell.c) = s_markBorder;
    border.push_back(cell);
    clusterCells.push_back(cell);
}

template <template <typename> typename RasterType>
inline void insert_cell(const Cell& cell, RasterType<uint8_t>& mark, FiLo<Cell>& border)
{
    mark(cell.r, cell.c) = s_markBorder;
    border.push_back(cell);
}

template <template <typename> typename RasterType, typename T>
void handle_cell(const Cell cell,
                 const T clusterValue, std::vector<Cell>& clusterCells,
                 RasterType<uint8_t>& mark,
                 FiLo<Cell>& border, const RasterType<T>& raster)
{
    if (raster.is_nodata(cell)) {
        return;
    }

    if (raster[cell] == clusterValue && mark[cell] == s_markTodo) {
        insert_cell(cell, clusterCells, mark, border);
    }
}

namespace internal {

template <template <typename> typename RasterType, typename T>
void handle_time_cell(float deltaD, const Cell& cell, const Cell& newCell,
                      RasterType<float>& distanceToTarget,
                      RasterType<uint8_t>& mark,
                      const RasterType<T>& travelTime,
                      FiLo<Cell>& border)
{
    if (distanceToTarget.is_nodata(cell) || distanceToTarget.is_nodata(newCell)) {
        return;
    }

    const float alternativeDist = static_cast<float>(distanceToTarget[cell] + deltaD * travelTime[newCell]);
    float& d                    = distanceToTarget[newCell];
    if (d > alternativeDist) {
        d       = alternativeDist;
        auto& m = mark[newCell];
        if (m != s_markBorder) {
            m = s_markBorder;
            border.push_back(newCell);
        }
    }
}

}

template <typename Callable>
void visit_neighbour_cells(Cell cell, int32_t rows, int32_t cols, Callable&& callable)
{
    bool isLeftBorder  = cell.c == 0;
    bool isRightBorder = cell.c == cols - 1;

    bool isTopBorder    = cell.r == 0;
    bool isBottomBorder = cell.r == rows - 1;

    if (!isRightBorder) {
        callable(right_cell(cell));
    }

    if (!isLeftBorder) {
        callable(left_cell(cell));
    }

    if (!isBottomBorder) {
        callable(bottom_cell(cell));
    }

    if (!isTopBorder) {
        callable(top_cell(cell));
    }
}

template <typename Callable>
void visit_neighbour_diag_cells(Cell cell, int32_t rows, int32_t cols, Callable&& callable)
{
    bool isLeftBorder  = cell.c == 0;
    bool isRightBorder = cell.c == cols - 1;

    bool isTopBorder    = cell.r == 0;
    bool isBottomBorder = cell.r == rows - 1;

    if (!(isBottomBorder || isRightBorder)) {
        callable(bottom_right_cell(cell));
    }

    if (!(isTopBorder || isRightBorder)) {
        callable(top_right_cell(cell));
    }

    if (!(isBottomBorder || isLeftBorder)) {
        callable(bottom_left_cell(cell));
    }

    if (!(isTopBorder || isLeftBorder)) {
        callable(top_left_cell(cell));
    }
}

template <typename RasterType>
void show_warning_if_clustering_on_floats(const RasterType&)
{
    if constexpr (std::is_floating_point_v<typename RasterType::value_type>) {
        Log::warn("Performing cluster operation on floating point raster");
    }
}
}
