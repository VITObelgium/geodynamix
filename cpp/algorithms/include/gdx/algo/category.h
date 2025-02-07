#pragma once

#include "gdx/algo/cast.h"
#include "gdx/algo/categoryio.h"
#include "gdx/algo/clusterutils.h"
#include "gdx/cell.h"
#include "gdx/exception.h"

#include <cassert>
#include <limits>
#include <map>
#include <set>
#include <unordered_map>
#include <utility> // pair

namespace gdx {

namespace details {
template <template <typename> typename RasterType, typename T, typename BinaryPredicate>
std::unordered_map<int32_t, T> category_map_func(const RasterType<int32_t>& clusters, const RasterType<T>& values, T initValue, BinaryPredicate&& pred)
{
    const auto size = clusters.size();

    if (values.size() != size) {
        throw InvalidArgument("Cluster raster dimensions should match values raster dimensions");
    }

    std::unordered_map<int32_t, T> m;

    for (std::size_t i = 0; i < size; ++i) {
        if (!clusters.is_nodata(i) && !values.is_nodata(i)) {
            const int cat = clusters[i];
            const T value = values[i];
            if (cat != 0 && value != 0) {
                auto iter = m.find(cat);
                if (iter == m.end()) {
                    m[cat] = pred(initValue, value);
                } else {
                    m[cat] = pred(m[cat], value);
                }
            }
        }
    }

    return m;
}

template <template <typename> typename RasterType, typename T, typename BinaryPredicate>
RasterType<T> category_func(const RasterType<int32_t>& clusters, const RasterType<T>& values, T initValue, BinaryPredicate&& pred)
{
    const auto size = clusters.size();

    RasterType<T> result(values.metadata(), 0);
    auto m = category_map_func(clusters, values, initValue, pred);

    for (std::size_t i = 0; i < size; ++i) {
        if (clusters.is_nodata(i)) {
            result.mark_as_nodata(i);
        } else {
            const auto cat = clusters[i];
            if (cat != 0) {
                result[i] = m[cat];
                result.mark_as_data(i);
            }
        }
    }

    return result;
}
}

template <template <typename> typename RasterType, typename T>
RasterType<T> category_sum(const RasterType<int32_t>& clusters, const RasterType<T>& values)
{
    return details::category_func(clusters, values, T(0), std::plus<T>());
}

template <template <typename> typename RasterType, typename T>
std::unordered_map<int32_t, T> category_sum_map(const RasterType<int32_t>& clusters, const RasterType<T>& values)
{
    return details::category_map_func(clusters, values, T(0), std::plus<T>());
}

template <template <typename> typename RasterType>
RasterType<int32_t> category_sum(const RasterType<int32_t>& clusters, const RasterType<uint8_t>& values)
{
    return details::category_func(clusters, raster_cast<int32_t>(values), int32_t(0), std::plus<int32_t>());
}

template <template <typename> typename RasterType, typename T>
RasterType<T> category_mode(const RasterType<int32_t>& clusters, const RasterType<T>& values)
{
    const auto size = clusters.size();

    if (values.size() != size) {
        throw InvalidArgument("Cluster raster dimensions should match values raster dimensions");
    }

    RasterType<T> result(values.metadata(), 0);

    // count
    std::map<std::pair<int32_t, T>, int> counts;
    for (std::size_t i = 0; i < size; ++i) {
        if (!clusters.is_nodata(i) && !values.is_nodata(i)) {
            auto cat = clusters[i];
            if (cat != 0) {
                const auto key = std::pair(cat, values[i]);
                if (counts.find(key) == counts.end()) {
                    counts[key] = 1;
                } else {
                    counts[key] += 1;
                }
            }
        }
    }

    // get max_count
    std::unordered_map<int32_t, T> top_count;
    for (auto& cat_value_count : counts) {
        auto cat   = cat_value_count.first.first;
        auto value = cat_value_count.first.second;
        auto count = cat_value_count.second;
        if (top_count.find(cat) == top_count.end()) {
            top_count[cat] = value;
        } else {
            if (count > counts[std::pair(cat, top_count[cat])]) {
                top_count[cat] = value;
            }
        }
    }

    // assign value with max_count
    for (std::size_t i = 0; i < size; ++i) {
        const auto cat = clusters[i];
        if (cat != 0) {
            result[i] = top_count[cat];
        }
    }

    return result;
}

template <template <typename> typename RasterType, typename T>
RasterType<T> category_max(const RasterType<int32_t>& clusters, const RasterType<T>& values)
{
    return details::category_func(clusters, values, std::numeric_limits<T>::lowest(), [](auto lhs, auto rhs) {
        return std::max(lhs, rhs);
    });
}

template <template <typename> typename RasterType, typename T>
RasterType<T> category_min(const RasterType<int32_t>& clusters, const RasterType<T>& values)
{
    return details::category_func(clusters, values, std::numeric_limits<T>::max(), [](auto lhs, auto rhs) {
        return std::min(lhs, rhs);
    });
}

template <template <typename> typename RasterType, typename T, typename FilterPredicate>
RasterType<int32_t> category_filter(const RasterType<int32_t>& clusters, const RasterType<T>& filter, FilterPredicate&& pred)
{
    const auto size = clusters.size();

    if (filter.size() != size) {
        throw InvalidArgument("Cluster raster dimensions should match filter raster dimensions");
    }

    struct ClusterCount
    {
        uint32_t trueCount = 0;
        uint32_t count     = 0;
    };

    RasterType<int32_t> result = clusters.copy();
    std::unordered_map<int32_t, ClusterCount> m;

    for (std::size_t i = 0; i < size; ++i) {
        if (!clusters.is_nodata(i) && !filter.is_nodata(i)) {
            const int cat = clusters[i];
            auto& count   = m[cat];
            ++count.count;
            if (filter[i]) {
                ++count.trueCount;
            }
        }
    }

    for (std::size_t i = 0; i < size; ++i) {
        const auto cat = clusters[i];
        if (cat != 0) {
            result[i] = pred(i, m[cat]);
        }
    }

    return result;
}

template <template <typename> typename RasterType, typename T>
RasterType<int32_t> category_filter_or(const RasterType<int32_t>& clusters, const RasterType<T>& filter)
{
    return category_filter(clusters, filter, [&clusters](std::size_t index, auto&& clusterCount) {
        return clusterCount.trueCount > 0 ? clusters[index] : 0;
    });
}

template <template <typename> typename RasterType, typename T>
RasterType<int32_t> category_filter_and(const RasterType<int32_t>& clusters, const RasterType<T>& filter)
{
    return category_filter(clusters, filter, [&clusters](std::size_t index, auto&& clusterCount) {
        return clusterCount.count == clusterCount.trueCount ? clusters[index] : 0;
    });
}

template <template <typename> typename RasterType, typename T>
RasterType<int32_t> category_filter_not(const RasterType<int32_t>& clusters, const RasterType<T>& filter)
{
    return category_filter(clusters, filter, [&clusters](std::size_t index, auto&& clusterCount) {
        return clusterCount.trueCount == 0 ? clusters[index] : 0;
    });
}

namespace internal {
template <template <typename> typename RasterType>
std::set<int32_t> get_clusters_around(const int32_t row, const int32_t col, const RasterType<int32_t>& clusters, const float radiusInCells)
{
    std::set<int32_t> idAround;
    int32_t radius = static_cast<int32_t>(radiusInCells);

    const auto rows = clusters.rows();
    const auto cols = clusters.cols();

    int r0 = row - radius;
    if (r0 < 0) r0 = 0;
    int r1 = row + radius;
    if (r1 > rows - 1) r1 = rows - 1;
    int c0 = col - radius;
    if (c0 < 0) c0 = 0;
    int c1 = col + radius;
    if (c1 > cols - 1) c1 = cols - 1;
    const int radiusSqr = int(radiusInCells * radiusInCells);
    for (int r = r0; r <= r1; ++r) {
        for (int c = c0; c <= c1; ++c) {
            if (clusters.is_nodata(r, c)) {
                continue;
            }

            int dr = r - row;
            int dc = c - col;
            if (dr * dr + dc * dc <= radiusSqr) {
                const int id = clusters(r, c);
                if (id) {
                    idAround.insert(id);
                }
            }
        }
    }

    return idAround;
}
}

template <template <typename> typename RasterType, typename T>
RasterType<T> category_sum_in_buffer(const RasterType<int32_t>& clusters, const RasterType<T>& mapToSum, const float radiusInMeter)
{
    if (mapToSum.size() != clusters.size()) {
        throw InvalidArgument("Cluster raster dimensions should match raster to sum dimensions");
    }

    const auto rows           = mapToSum.rows();
    const auto cols           = mapToSum.cols();
    const float radiusInCells = static_cast<float>(radiusInMeter / mapToSum.metadata().cellSize.x);
    auto resultMeta           = mapToSum.metadata();

    RasterType<T> result(resultMeta);
    std::unordered_map<int, double> sum;

    // loop over all cells in mapToSum
    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            if (mapToSum.is_nodata(r, c)) {
                result(r, c) = mapToSum(r, c);
                result.mark_as_nodata(r, c);
                continue;
            }

            const T valueToSum = mapToSum(r, c);
            if (valueToSum != 0) {
                // bepaal rond iedere cel welke clusters er binnen een straat van radius cellen liggen
                auto idAround = internal::get_clusters_around(r, c, clusters, radiusInCells);

                // tel de waarde uit mapToSum op bij ieder van die clusters
                for (auto& id : idAround) {
                    sum[id] += valueToSum;
                }
            }
        }
    }

    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            if (clusters.is_nodata(r, c)) {
                continue;
            }

            const int id = clusters(r, c);
            if (id != 0) {
                auto iterSum = sum.find(id);
                if (iterSum != sum.end()) {
                    result(r, c) = static_cast<T>(iterSum->second);
                } else {
                    result(r, c) = T(0);
                }
            } else {
                result(r, c) = T(0);
            }
        }
    }

    return result;
}

}
