#include "gdx/algo/dasmap.h"
#include "gdx/test/testbase.h"

namespace gdx::test {

TEST_CASE("DasMap.dasMap")
{
    const DenseRaster<int> landuse(1, 5, std::vector<int>{1, 1, 2, 2, 3});
    const std::vector<double> weights = {1.0f, 1.0f, 1.0f, 1.0f};
    const DenseRaster<int> zones(1, 5, std::vector<int>{1, 1, 1, 2, 2});
    const std::vector<double> amounts = {0.0f, 1.0f, 1.0f};
    const DenseRaster<float> expected(1, 5, std::vector<float>{1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f, 2.0f / 3.0f});
    DenseRaster<float> actual = gdx::dasMap<float, DenseRaster, int, double>(landuse, weights, zones, amounts);
    CHECK_RASTER_EQ(expected, actual);
}

TEST_CASE("DasMap.dasMap2")
{
    const DenseRaster<int> landuse(1, 5, std::vector<int>{1, 0, 2, 2, 3});
    const std::vector<double> weights = {1.0f, 1.0f, 1.0f, 1.0f};
    const DenseRaster<int> zones(1, 5, std::vector<int>{0, 1, 1, 2, 2});
    const std::vector<double> amounts = {0.0f, 1.0f, 1.0f};
    const DenseRaster<float> expected(1, 5, std::vector<float>{0.0f, 2.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f, 2.0f / 3.0f});
    DenseRaster<float> actual = gdx::dasMap<float, DenseRaster, int, double>(landuse, weights, zones, amounts);
    CHECK_RASTER_EQ(expected, actual);
}

TEST_CASE("DasMap.dasMap3")
{
    const DenseRaster<int> landuse(1, 5, std::vector<int>{1, 1, 2, 2, 3});
    const std::vector<double> weights = {1.0f, 1.0f, 2.0f, 1.0f};
    const DenseRaster<int> zones(1, 5, std::vector<int>{1, 1, 1, 2, 2});
    const std::vector<double> amounts = {0.0f, 2.0f, 3.0f};
    const DenseRaster<float> expected(1, 5, std::vector<float>{0.5f, 0.5f, 1.0f, 1.5f, 1.5f});
    DenseRaster<float> actual = gdx::dasMap<float, DenseRaster, int, double>(landuse, weights, zones, amounts);
    CHECK_RASTER_EQ(expected, actual);
}

TEST_CASE("DasMap.dasMapWeiss")
{
    RasterMetadata meta(5, 5, 0.0, 0.0, 100.0, -9999.0);
    const DenseRaster<int> landuse(meta, std::vector<int>{
                                             0, 1, 2, 3, -9999,
                                             0, 1, 2, 3, -9999,
                                             0, 1, 2, 3, -9999,
                                             0, 1, 2, 3, -9999,
                                             0, 1, 2, 3, -9999});
    const std::vector<double> weights = {1.0f, 0.0f, 1.2f, 1.3f};
    const DenseRaster<int> zones(meta, std::vector<int>{
                                           0, 0, 0, 0, 0,
                                           1, 1, 1, 1, 1,
                                           2, 2, 2, 2, 2,
                                           3, 3, 3, 3, 3,
                                           -9999, -9999, -9999, -9999, -9999});
    const std::vector<double> amounts = {1.0f, 0.0f, 100.0f, 1000.0f};
    DenseRaster<float> actual         = gdx::dasMap<float, DenseRaster, int, double>(landuse, weights, zones, amounts);

    auto nan = std::numeric_limits<float>::quiet_NaN();
    DenseRaster<float> expected(meta, std::vector<float>{
                                          0.2857142984867096f, 0.0f, 0.34285715222358704f, 0.37142854928970337f, nan,
                                          0.0f, 0.0f, 0.0f, 0.0f, nan,
                                          28.571430206298828f, 0.0f, 34.28571701049805f, 37.14285659790039f, nan,
                                          285.71429443359375f, 0.0f, 342.8571472167969f, 371.4285583496094f, nan,
                                          nan, nan, nan, nan, nan});
    expected.set_nodata(nan);
    CHECK(actual.metadata() == expected.metadata());
    CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
}

TEST_CASE("DasMap.dasMapWeiss2")
{
    RasterMetadata meta(5, 5, 0.0, 0.0, 100.0, -9999.0);
    const DenseRaster<int> landuse(meta, std::vector<int>{
                                             0, 0, 0, 0, 2,
                                             2, 0, 0, 0, 0,
                                             0, 0, 1, 1, 0,
                                             0, 0, 0, 1, 0,
                                             0, 0, 0, 0, 2});
    std::vector<double> weights(7, std::numeric_limits<double>::quiet_NaN());
    weights[0] = 0.0;
    weights[1] = 0.0;
    weights[2] = 0.97591;
    weights[3] = 0.0;
    weights[4] = 0.0;
    weights[5] = 0.02407;
    weights[6] = 0.0;

    std::vector<double> amounts(7, std::numeric_limits<double>::quiet_NaN());
    amounts[1] = 13.948950;
    amounts[2] = 98.291438;
    amounts[3] = 14.460188;
    amounts[4] = 16.544103;
    amounts[5] = 89.133503;
    amounts[6] = 134.886442;

    const DenseRaster<int> zones(meta, std::vector<int>{
                                           1, 1, 2, 2, 2,
                                           1, 1, 2, 2, 2,
                                           3, 4, 4, 4, 2,
                                           3, 5, 5, 4, 6,
                                           5, 5, 5, 5, 6});

    DenseRaster<float> actual = gdx::dasMap<float>(landuse, weights, zones, amounts);

    auto nan    = std::numeric_limits<float>::quiet_NaN();
    meta.nodata = nan;
    DenseRaster<float> expected(meta, std::vector<float>{
                                          0.f, 0.0f, 0.f, 0.f, 98.291438f,
                                          13.948950f, 0.0f, 0.0f, 0.0f, 0.0f,
                                          7.23009f, 4.13603f, 4.13603f, 4.13603f, 0.0f,
                                          7.23009f, 14.8556f, 14.8556f, 4.13603f, 0.0f,
                                          14.8556f, 14.8556f, 14.8556f, 14.8556f, 134.886f});
    CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
}

TEST_CASE("DasMap.dasMapWeiss3")
{
    RasterMetadata meta(5, 5, -9999.0);
    const DenseRaster<int> landuse(meta, std::vector<int>{
                                             -9999, -9999, -9999, -9999, 2,
                                             2, -9999, -9999, -9999, -9999,
                                             -9999, -9999, 1, 1, -9999,
                                             -9999, -9999, -9999, 1, -9999,
                                             -9999, -9999, -9999, -9999, 2});
    std::vector<double> weights(7, std::numeric_limits<double>::quiet_NaN());
    weights[0] = 0.0;
    weights[1] = 0.0;
    weights[2] = 0.97591;
    weights[3] = 0.0;
    weights[4] = 0.0;
    weights[5] = 0.02407;
    weights[6] = 0.0;

    std::vector<double> amounts(7, std::numeric_limits<double>::quiet_NaN());
    amounts[1] = 13.948950;
    amounts[2] = 98.291438;
    amounts[3] = 14.460188;
    amounts[4] = 16.544103;
    amounts[5] = 89.133503;
    amounts[6] = 134.886442;

    const DenseRaster<int> zones(meta, std::vector<int>{
                                           1, 1, 2, 2, 2,
                                           1, 1, 2, 2, 2,
                                           3, 4, 4, 4, 2,
                                           3, 5, 5, 4, 6,
                                           5, 5, 5, 5, 6});

    DenseRaster<float> actual = gdx::dasMap<float>(landuse, weights, zones, amounts);

    auto nan    = std::numeric_limits<float>::quiet_NaN();
    meta.nodata = nan;
    DenseRaster<float> expected(meta, std::vector<float>{
                                          nan, nan, nan, nan, 98.291438f,
                                          13.948950f, nan, nan, nan, nan,
                                          nan, nan, 5.5147f, 5.5147f, nan,
                                          nan, nan, nan, 5.5147f, nan,
                                          nan, nan, nan, nan, 134.886f});
    CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, actual, 1e-5f);
}
}
