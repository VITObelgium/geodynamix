#include "gdx/algo/algorithm.h"
#include "gdx/rasteriterator.h"
#include "gdx/rasterspan.h"
#include "gdx/test/testbase.h"

#include <numeric>
#include <random>

namespace gdx::test {

TEST_CASE("AlgorithmTest.transformConstInput")
{
    const std::vector<int> v{{-1, 1, 2, 3, -1, 4, 5, 6, -1}};
    std::vector<int> result(v.size());

    RasterMetadata inputMeta(3, 3, -1);
    RasterMetadata outputMeta(3, 3, -9999);

    auto inputSpan  = make_raster_span(v, inputMeta);
    auto outputSpan = make_raster_span(result, outputMeta);

    gdx::transform(inputSpan, outputSpan, [](int value) {
        return value * 2;
    });

    const std::vector<int> expected{{-9999, 2, 4, 6, -9999, 8, 10, 12, -9999}};

    CHECK_CONTAINER_EQ(result, expected);
}

TEST_CASE("AlgorithmTest.transformNonConstInput")
{
    std::vector<int> v{{-1, 1, 2, 3, -1, 4, 5, 6, -1}};
    std::vector<int> result(v.size());

    RasterMetadata inputMeta(3, 3, -1);
    RasterMetadata outputMeta(3, 3, -9999);

    auto inputSpan  = make_raster_span(v, inputMeta);
    auto outputSpan = make_raster_span(result, outputMeta);

    gdx::transform(inputSpan, outputSpan, [](int value) {
        return value * 2;
    });

    const std::vector<int> expected{{-9999, 2, 4, 6, -9999, 8, 10, 12, -9999}};

    CHECK_CONTAINER_EQ(result, expected);
}

template <typename T, typename Input1, typename Input2>
static std::vector<T> transform2Inputs(Input1&& i1, Input2&& i2)
{
    CHECK(i1.size() == i2.size());
    std::vector<T> result(i1.size());

    RasterMetadata inputMeta1(3, 3, -1);
    RasterMetadata inputMeta2(3, 3, -2);
    RasterMetadata outputMeta(3, 3, -9999);

    auto inputSpan1 = make_raster_span(i1, inputMeta1);
    auto inputSpan2 = make_raster_span(i2, inputMeta2);
    auto outputSpan = make_raster_span(result, outputMeta);

    gdx::transform(inputSpan1, inputSpan2, outputSpan, [](int val1, int val2) {
        return val1 + val2;
    });

    return result;
}

TEST_CASE("AlgorithmTest.transform2ConstInputs")
{
    const std::vector<int> v1{{-1, 1, 2, 3, -1, 4, 5, 6, 0}};
    const std::vector<int> v2{{0, 10, 20, 30, -2, 40, 50, 60, -2}};
    const std::vector<int> expected{{-9999, 11, 22, 33, -9999, 44, 55, 66, -9999}};

    CHECK_CONTAINER_EQ(transform2Inputs<int>(v1, v2), expected);
}

TEST_CASE("AlgorithmTest.transform2NonConstInputs")
{
    std::vector<int> v1{{-1, 1, 2, 3, -1, 4, 5, 6, 0}};
    std::vector<int> v2{{0, 10, 20, 30, -2, 40, 50, 60, -2}};
    const std::vector<int> expected{{-9999, 11, 22, 33, -9999, 44, 55, 66, -9999}};

    CHECK_CONTAINER_EQ(transform2Inputs<int>(v1, v2), expected);
}

TEST_CASE("AlgorithmTest.for_each_value_const_inputs")
{
    const std::vector<int> v{{-1, 1, 2, 3, -1, 4, 5, 6, 0}};
    const std::vector<int> expected{{1, 2, 3, 4, 5, 6, 0}};
    std::vector<int> actual;

    RasterMetadata inputMeta(3, 3, -1);
    auto inputSpan = make_raster_span(v, inputMeta);

    gdx::for_each_data_value(inputSpan, [&actual](auto& value) {
        actual.push_back(value);
    });

    CHECK_CONTAINER_EQ(expected, actual);
}

TEST_CASE("AlgorithmTest.for_each_value_non_const_inputs")
{
    std::vector<int> v{{-1, 1, 2, 3, -1, 4, 5, 6, 0}};
    const std::vector<int> expected{{-1, 2, 3, 4, -1, 5, 6, 7, 1}};

    RasterMetadata inputMeta(3, 3, -1);
    auto inputSpan = make_raster_span(v, inputMeta);

    gdx::for_each_data_value(inputSpan, [](auto& value) {
        value = value + 1;
    });

    CHECK_CONTAINER_EQ(expected, v);
}

}
