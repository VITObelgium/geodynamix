#pragma once

#include "gdx/algo/algorithm.h"
#include "gdx/exception.h"

#include <gsl/span>
#include <numeric>
#include <type_traits>
#include <unordered_map>

namespace gdx {

template <class T>
struct has_sum_member
{
    template <typename U, U u>
    struct reallyHas;

    template <typename C>
    static constexpr std::true_type test(reallyHas<double (C::*)(), &C::sum>* /*unused*/)
    {
        return {};
    }

    template <typename C>
    static constexpr std::true_type test(reallyHas<double (C::*)() const, &C::sum>* /*unused*/)
    {
        return {};
    }

    template <typename>
    static std::false_type test(...)
    {
        return {};
    }

    static constexpr bool value = decltype(test<T>(0))::value;
};

template <typename T>
inline constexpr bool has_sum_member_v = has_sum_member<T>::value;

template <typename RasterType>
typename std::enable_if_t<has_sum_member_v<RasterType>, double> sum(const RasterType& ras)
{
    return ras.template sum<double>();
}

template <typename RasterType>
typename std::enable_if_t<!has_sum_member_v<RasterType>, double> sum(const RasterType& ras)
{
    double sum      = 0.0;
    const auto size = ras.size();

    for (std::size_t i = 0; i < size; ++i) {
        if (!ras.is_nodata(i)) {
            sum += ras[i];
        }
    }

    return sum;
}

template <typename RasterType>
double ssum(const RasterType& ras)
{
    using T = typename RasterType::value_type;
    return static_cast<double>(std::accumulate(value_begin(ras), value_end(ras), T(0), std::plus<T>()));
}

}
