#pragma once

#include "gdx/algo/clusterutils.h"
#include "gdx/exception.h"
#include "gdx/rastermetadata.h"

#include <cassert>
#include <cmath>
#include <random>

namespace gdx {

namespace internal {

template <template <typename> typename RasterType, typename TTarget, typename TTravelTime>
void compute_node_value_distance_decay(
    RasterType<float>& result,
    int32_t targetRow, int32_t targetCol, TTarget target,
    const RasterType<TTravelTime>& travelTime, TTravelTime maxTravelTime,
    const float a, const float b)
{
}

}

template <template <typename> typename RasterType, typename TTarget, typename TTravelTime>
RasterType<float> node_value_distance_decay(const RasterType<TTarget>& target, const RasterType<TTravelTime>& travelTime, TTravelTime maxTravelTime, float a, float b)
{
    throw_on_size_mismatch(target, travelTime);

    const int32_t rows = target.rows();
    const int32_t cols = target.cols();

    constexpr auto nan = std::numeric_limits<float>::quiet_NaN();
    RasterType<float> result(copy_metadata_replace_nodata(target.metadata(), nan), 0);

    FiLo<Cell> border(rows, cols);
    const float sqrt2       = std::sqrt(2.f);
    const float unreachable = static_cast<float>(maxTravelTime) + 1.f;
    RasterType<float> distanceToTarget(copy_metadata_replace_nodata(result.metadata(), {}));
    RasterType<uint8_t> mark(copy_metadata_replace_nodata(result.metadata(), {}));

    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            if (target.is_nodata(r, c) || target(r, c) == 0) {
                continue;
            }

            distanceToTarget.fill(unreachable);
            mark.fill(s_markTodo);

            assert(border.empty());

            Cell cell(r, c);
            border.push_back(cell);
            distanceToTarget[cell] = 0;
            mark[cell]             = s_markBorder;

            while (!border.empty()) {
                cell = border.pop_head();
                assert(mark[cell] == s_markBorder);
                mark[cell] = s_markDone;
                if (distanceToTarget[cell] <= maxTravelTime) {
                    const float d = distanceToTarget[cell];
                    const float v = std::exp(a - b * d) / (1 + std::exp(a - b * d)) * static_cast<float>(target(r, c));
                    if (auto& resultVal = result[cell]; resultVal < v) {
                        resultVal = v;
                    }
                }

                visit_neighbour_cells(cell, rows, cols, [&](const Cell& neighbour) {
                    internal::handle_time_cell(1.f, cell, neighbour, distanceToTarget, mark, travelTime, border);
                });

                visit_neighbour_diag_cells(cell, rows, cols, [&](const Cell& neighbour) {
                    internal::handle_time_cell(sqrt2, cell, neighbour, distanceToTarget, mark, travelTime, border);
                });
            }
        }
    }

    return result;
}

}
