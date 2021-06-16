#include "gdx/test/testbase.h"

namespace gdx::test {

void compareMetaData(const RasterMetadata& expected, const RasterMetadata& actual)
{
    CHECK(expected.rows == actual.rows);
    CHECK(expected.cols == actual.cols);
    CHECK(expected.nodata == actual.nodata);
    CHECK(expected.cellSize == Approx(actual.cellSize));
    CHECK(expected.xll == Approx(actual.xll));
    CHECK(expected.yll == Approx(actual.yll));
}

}
