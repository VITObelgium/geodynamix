#include "gdx/eigeniterationsupport-private.h"
#include "infra/span.h"

#include <doctest/doctest.h>

namespace gdx::test {

using namespace doctest;

TEST_CASE("EigenIteration.convertToSpan")
{
    Eigen::Array<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> eigenArray;
    eigenArray.resize(10, 20);

    std::span<const float> span(eigenArray.data(), eigenArray.size());
    CHECK(eigenArray.size() == span.size());
    CHECK(eigenArray.data() == span.data());
}

TEST_CASE("EigenIteration.iterate")
{
    Eigen::Array<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> eigenArray;
    eigenArray.resize(10, 20);
    eigenArray.fill(5.f);

    for (auto& val : eigenArray) {
        CHECK(val == Approx(5.f));
    }

    CHECK(eigenArray.size() == std::distance(begin(eigenArray), end(eigenArray)));
}
}
