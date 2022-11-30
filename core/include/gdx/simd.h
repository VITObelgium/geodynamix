#pragma once

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4244 4127 4005)
#endif
#include <Vc/common/simdize.h>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include <algorithm>
#include <cassert>
#include <type_traits>
#include <vector>

namespace gdx::simd {

template <class InputIt, class UnaryFunction, class ValueType = typename std::iterator_traits<InputIt>::value_type>
std::enable_if_t<Vc::Traits::is_functor_argument_immutable<UnaryFunction, Vc::simdize<ValueType>>::value, UnaryFunction>
for_each(InputIt first, InputIt last, UnaryFunction f)
{
    static_assert(std::is_same_v<std::random_access_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>, "simd::for_each requires random access iterator");

    using V  = Vc::simdize<ValueType>;
    using V1 = Vc::simdize<ValueType, 1>;

    if (V::size() <= static_cast<uint64_t>(std::distance(first, last))) {
        const auto lastV = last - V::Size + 1;
        for (; first < lastV; first += V::Size) {
            V tmp;
            load_interleaved(tmp, std::addressof(*first));
            f(tmp);
        }
    }

    for (; first != last; ++first) {
        V1 tmp;
        load_interleaved(tmp, std::addressof(*first));
        f(tmp);
    }

    return f;
}

template <typename InputIt, typename UnaryFunction, class ValueType = typename std::iterator_traits<InputIt>::value_type>
std::enable_if_t<!Vc::Traits::is_functor_argument_immutable<UnaryFunction, Vc::simdize<ValueType>>::value, UnaryFunction>
for_each(InputIt first, InputIt last, UnaryFunction f)
{
    static_assert(std::is_same_v<std::random_access_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>, "simd::for_each requires random access iterator");

    using V  = Vc::simdize<ValueType>;
    using V1 = Vc::simdize<ValueType, 1>;

    if (V::size() <= static_cast<uint64_t>(std::distance(first, last))) {
        const auto lastV = last - V::size() + 1;
        for (; first < lastV; first += V::size()) {
            V tmp;
            load_interleaved(tmp, std::addressof(*first));
            f(tmp);
            store_interleaved(tmp, std::addressof(*first));
        }
    }
    for (; first != last; ++first) {
        V1 tmp;
        load_interleaved(tmp, std::addressof(*first));
        f(tmp);
        store_interleaved(tmp, std::addressof(*first));
    }
    return f;
}

template <typename InputIt, typename OutputIt, typename UnaryFunction>
OutputIt transform(InputIt first, InputIt last, OutputIt out, UnaryFunction f)
{
    using ValueTypeIn  = typename std::iterator_traits<InputIt>::value_type;
    using ValueTypeOut = typename std::iterator_traits<OutputIt>::value_type;

    using V        = Vc::simdize<ValueTypeIn>;
    using VLast    = Vc::simdize<ValueTypeIn, 1>;
    using VOut     = Vc::simdize<ValueTypeOut>;
    using VOutLast = Vc::simdize<ValueTypeOut, 1>;

    static_assert(sizeof(typename V::value_type) == sizeof(typename VOut::value_type), "simd::transform: data types must have equal size");

    if (V::size() <= static_cast<uint64_t>(std::distance(first, last))) {
        const auto lastV = last - V::size() + 1;
        for (; first < lastV; first += V::size(), out += VOut::size()) {
            V tmp;
            load_interleaved(tmp, std::addressof(*first));
            VOut res = f(tmp);
            store_interleaved(res, std::addressof(*out));
        }
    }
    for (; first != last; ++first, ++out) {
        VLast tmp;
        load_interleaved(tmp, std::addressof(*first));
        VOutLast res = f(tmp);
        store_interleaved(res, std::addressof(*out));
    }

    return out;
}

template <typename InputIt1, typename InputIt2, typename OutputIt, typename BinaryFunction>
OutputIt transform(InputIt1 first, InputIt1 last, InputIt2 second, OutputIt out, BinaryFunction f)
{
    using ValueTypeIn1 = typename std::iterator_traits<InputIt1>::value_type;
    using ValueTypeIn2 = typename std::iterator_traits<InputIt2>::value_type;
    using ValueTypeOut = typename std::iterator_traits<OutputIt>::value_type;

    using V1     = Vc::simdize<ValueTypeIn1>;
    using V1Last = Vc::simdize<ValueTypeIn1, 1>;

    using V2       = Vc::simdize<ValueTypeIn2>;
    using V2Last   = Vc::simdize<ValueTypeIn2, 1>;
    using VOut     = Vc::simdize<ValueTypeOut>;
    using VOutLast = Vc::simdize<ValueTypeOut, 1>;

    static_assert(std::is_same_v<typename V1::abi, typename V2::abi>, "simd abi mismatch");
    static_assert(std::is_same_v<typename V1::abi, typename VOut::abi>, "simd abi mismatch");
    static_assert(sizeof(typename V1::value_type) == sizeof(typename V2::value_type), "simd::transform: input data types must have equal size");

    //&& sizeof(typename V1::value_type) == sizeof(typename VOut::value_type)

    if (V1::size() <= static_cast<uint64_t>(std::distance(first, last))) {
        const auto lastV = last - V1::size() + 1;
        for (; first < lastV; first += V1::size(), second += V2::size(), out += VOut::size()) {
            V1 tmp1;
            V2 tmp2;
            load_interleaved(tmp1, std::addressof(*first));
            load_interleaved(tmp2, std::addressof(*second));
            VOut res = f(tmp1, tmp2);
            store_interleaved(res, std::addressof(*out));
        }
    }

    for (; first != last; ++first, ++second, ++out) {
        V1Last tmp1;
        V2Last tmp2;
        load_interleaved(tmp1, std::addressof(*first));
        load_interleaved(tmp2, std::addressof(*second));
        VOutLast res = f(tmp1, tmp2);
        store_interleaved(res, std::addressof(*out));
    }

    return out;
}

}
