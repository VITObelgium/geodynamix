#include "gdx/algo/accuflux.h"
#include "gdx/test/testbase.h"

#include <doctest/trompeloeil.hpp>

#include <random>

namespace gdx::test {

using namespace trompeloeil;

TEST_CASE("AccufluxTest.Accuflux")
{
    RasterMetadata meta(4, 4);

    MaskedRaster<float> freightMap(meta, std::vector<float>({1, 1, 1, 1,
                                             2, 3, 4, 5,
                                             1, 1, 1, 1,
                                             1, 1, 1, 1}));

    MaskedRaster<uint8_t> lddMap(meta, std::vector<uint8_t>({
                                           2, 2, 2, 2, // | | | |
                                           2, 2, 2, 2, // | | | |
                                           3, 2, 1, 4, // \ | / -
                                           6, 5, 4, 4  // - . - -
                                       }));

    auto result = accuflux(lddMap, freightMap);

    MaskedRaster<float> expected(meta, std::vector<float>({1, 1, 1, 1,
                                           3, 4, 5, 6,
                                           4, 5, 13, 7,
                                           1, 26, 2, 1}));

    CHECK_RASTER_EQ(expected, result);
}

TEST_CASE("AccufluxTest.AccufluxNANValues")
{
    RasterMetadata meta(4, 4, 0);

    MaskedRaster<float> freightMap(meta, std::vector<float>({1, 1, 1, 1,
                                             2, 3, 4, 5,
                                             1, 1, 1, 1,
                                             1, 1, 1, 1}));

    MaskedRaster<uint8_t> lddMap(meta, std::vector<uint8_t>({
                                           2, 2, 2, 2, // | | | |
                                           2, 2, 2, 2, // | | | |
                                           3, 2, 1, 4, // \ | / -
                                           6, 5, 4, 0  // - . - -
                                       }));

    auto result = accuflux(lddMap, freightMap);

    MaskedRaster<float> expected(meta, std::vector<float>({1, 1, 1, 1,
                                           3, 4, 5, 6,
                                           4, 5, 13, 7,
                                           1, 25, 1, 0}));

    CHECK_RASTER_NEAR(expected, result);
}

TEST_CASE("AccufluxTest.AccufluxFailsLddLoop")
{
    RasterMetadata meta(4, 4);

    MaskedRaster<uint8_t> lddMap(meta, std::vector<uint8_t>({6, 6, 6, 2,
                                           8, 5, 5, 2,
                                           8, 5, 5, 2,
                                           8, 4, 4, 4}));

    MaskedRaster<float> freightMap(meta, 1);
    CHECK_THROWS_AS(accuflux(lddMap, freightMap), RuntimeError);
}

TEST_CASE("AccufluxTest.Accufractionflux")
{
    RasterMetadata meta(5, 5);
    RasterMetadata floatMeta(5, 5);
    floatMeta.nodata = std::numeric_limits<double>::quiet_NaN();

    // reference data from
    // http://pcraster.geo.uu.nl/pcraster/4.1.0/doc/manual/op_accufraction.html

    MaskedRaster<float> freightMap(floatMeta, std::vector<float>({10, 10, 10, 10, 10,
                                                  10, 10, 10, 10, 10,
                                                  10, 10, 10, 10, 10,
                                                  10, 10, 10, 10, 50,
                                                  50, 50, 50, 50, 49}));

    MaskedRaster<float> fractionMap(floatMeta, std::vector<float>({0.9f, 1.0f, 0.1f, 0.1f, 0.1f,
                                                   0.9f, 1.0f, 0.1f, 0.1f, 0.1f,
                                                   0.9f, 1.0f, 0.1f, 0.1f, 0.1f,
                                                   0.9f, 1.0f, 0.1f, 0.1f, 0.1f,
                                                   0.9f, 1.0f, 0.1f, 0.1f, 0.1f}));

    MaskedRaster<uint8_t> lddMap(meta, std::vector<uint8_t>({2, 2, 2, 1, 1,
                                           2, 2, 1, 1, 1,
                                           3, 2, 1, 4, 1,
                                           3, 2, 1, 4, 4,
                                           6, 5, 4, 4, 4}));

    auto result = accufractionflux(lddMap, freightMap, fractionMap);

    MaskedRaster<float> expected(floatMeta, std::vector<float>({9.0f, 10.0f, 1.0f, 1.0f, 1.0f,
                                                17.1f, 20.0f, 1.2f, 1.1f, 1.0f,
                                                24.39f, 31.2f, 1.22f, 1.1f, 1.0f,
                                                9.0f, 66.81f, 1.16f, 1.6f, 5.0f,
                                                45.0f, 177.52f, 5.549f, 5.49f, 4.9f}));

    CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, result, 0.01);
}

TEST_CASE("AccufluxTest.Accufractionflux2")
{
    auto nan = std::numeric_limits<float>::quiet_NaN();

    RasterMetadata floatMeta(5, 5, nan);
    RasterMetadata meta(5, 5, 0);

    // reference data from
    // http://pcraster.geo.uu.nl/pcraster/4.1.0/doc/manual/op_accufraction.html

    MaskedRaster<float> freightMap(floatMeta, std::vector<float>({1, 1, 1, 1, 1,
                                                  1, nan, 1, 1, 1,
                                                  1, 1, 1, 1, 1,
                                                  1, 1, 1, 1, 1,
                                                  1, 1, 1, 1, 1}));

    MaskedRaster<float> fractionMap(floatMeta, std::vector<float>({1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                                                   1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                                                   1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                                                   1.0f, 1.0f, 1.0f, nan, 1.0f,
                                                   1.0f, 1.0f, 1.0f, 1.0f, 1.0f}));

    MaskedRaster<uint8_t> lddMap(meta, std::vector<uint8_t>({5, 1, 1, 1, 0,
                                           5, 1, 1, 1, 1,
                                           5, 1, 1, 1, 1,
                                           5, 1, 1, 1, 1,
                                           5, 5, 5, 5, 5}));

    auto result = accufractionflux(lddMap, freightMap, fractionMap);

    MaskedRaster<float> expected(floatMeta, std::vector<float>({1.0f, 1.0f, 1.0f, 1.0f, nan,
                                                2.0f, nan, 2.0f, 1.0f, 1.0f,
                                                nan, 3.0f, 2.0f, 2.0f, 1.0f,
                                                4.0f, 3.0f, 3.0f, nan, 1.0f,
                                                4.0f, 4.0f, nan, 2.0f, 1.0f}));

    CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, result, 0.02);
}

TEST_CASE("AccufluxTest.LddClusterStationsInPits")
{
    auto nan = std::numeric_limits<float>::quiet_NaN();

    RasterMetadata floatMeta(4, 4);
    floatMeta.nodata = nan;

    RasterMetadata meta(4, 4);
    meta.nodata = 0;

    MaskedRaster<int32_t> idMap(RasterMetadata(4, 4), std::vector<int32_t>({0, 0, 0, 0,
                                                          0, 0, 0, 0,
                                                          0, 0, 0, 0,
                                                          5, 0, 0, 8}));

    MaskedRaster<uint8_t> lddMap(meta, std::vector<uint8_t>({2, 1, 2, 2,
                                           2, 1, 2, 2,
                                           2, 1, 3, 2,
                                           5, 4, 6, 5}));

    auto result = ldd_cluster(lddMap, idMap);

    MaskedRaster<int32_t> expected(floatMeta, std::vector<int32_t>({5, 5, 8, 8,
                                                  5, 5, 8, 8,
                                                  5, 5, 8, 8,
                                                  5, 5, 8, 8}));

    CHECK_RASTER_EQ(expected, result);
}

TEST_CASE("AccufluxTest.LddClusterStationsNotInPits")
{
    auto nan = std::numeric_limits<float>::quiet_NaN();

    RasterMetadata floatMeta(4, 4);
    floatMeta.nodata = nan;

    RasterMetadata meta(4, 4);
    meta.nodata = 0;

    MaskedRaster<int32_t> idMap(RasterMetadata(4, 4), std::vector<int32_t>({0, 0, 0, 0,
                                                          0, 0, 0, 8,
                                                          5, 0, 0, 0,
                                                          0, 0, 0, 4}));

    MaskedRaster<uint8_t> lddMap(meta, std::vector<uint8_t>({2, 1, 2, 2,
                                           2, 1, 2, 2,
                                           2, 1, 3, 2,
                                           5, 4, 6, 5}));

    auto result = ldd_cluster(lddMap, idMap);

    MaskedRaster<int32_t> expected(floatMeta, std::vector<int32_t>({5, 5, 4, 8,
                                                  5, 5, 4, 8,
                                                  5, 0, 4, 4,
                                                  0, 0, 4, 4}));

    CHECK_RASTER_EQ(expected, result);
}

TEST_CASE("AccufluxTest.LddDist")
{
    constexpr auto nan = std::numeric_limits<float>::quiet_NaN();

    RasterMetadata floatMeta(5, 5);
    floatMeta.nodata = nan;
    floatMeta.set_cell_size(2.0);

    RasterMetadata meta(5, 5);
    meta.nodata = 0;
    meta.set_cell_size(2.0);

    // reference data from
    // http://pcraster.geo.uu.nl/pcraster/4.1.0/doc/manual/op_ldddist.html

    MaskedRaster<float> pointsMap(floatMeta, std::vector<float>({0, 0, 0, 1, 0,
                                                 0, 0, 1, 0, 0,
                                                 0, 0, 0, 0, 0,
                                                 0, 1, 0, 0, 1,
                                                 0, 0, 1, 0, 0}));

    MaskedRaster<uint8_t> lddMap(meta, std::vector<uint8_t>({
                                           2, 2, 2, 1, 1, // | | | / /
                                           2, 2, 1, 1, 1, // | | / / /
                                           3, 2, 1, 4, 1, // | | / - /
                                           3, 2, 1, 4, 4, // \ | / - -
                                           6, 5, 4, 4, 4  // - . - - -
                                       }));

    MaskedRaster<float> frictionMap(floatMeta, std::vector<float>({1, 5, 5, 5, 40,
                                                   1, nan, 5, 5, 0.1f,
                                                   1, 5, 5, 5, 0.1f,
                                                   1, 5, 0.1f, 0.1f, 0.1f,
                                                   1, 0.1f, 0.1f, 6.8f, 0.1f}));

    auto result = ldd_dist(lddMap, pointsMap, frictionMap);

    MaskedRaster<float> expected(floatMeta, std::vector<float>({12.48f, nan, 10, 0, 91.92f,
                                                10.48f, nan, 0, 28.28f, 31.35f,
                                                8.48f, 10, 14.14f, 24.14f, nan,
                                                nan, 0, nan, nan, 0,
                                                nan, nan, 0, 6.9f, 13.8f}));

    CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, result, 0.02);
}

TEST_CASE("AccufluxTest.LddDistConstantFriction")
{
    constexpr auto nan = std::numeric_limits<float>::quiet_NaN();

    RasterMetadata floatMeta(5, 5);
    floatMeta.nodata = nan;
    floatMeta.set_cell_size(2.0);

    RasterMetadata meta(5, 5);
    meta.nodata = 0;
    meta.set_cell_size(2.0);

    // reference data from
    // http://pcraster.geo.uu.nl/pcraster/4.1.0/doc/manual/op_ldddist.html

    MaskedRaster<float> pointsMap(floatMeta, std::vector<float>({0, 0, 0, 0, 0,
                                                 nan, 0, 0, 0, 0,
                                                 0, 0, 0, 0, 0,
                                                 0, 1, 0, 0, 0,
                                                 0, 0, 0, 0, 0}));

    MaskedRaster<uint8_t> lddMap(meta, std::vector<uint8_t>({
                                           2, 2, 2, 1, 1, // | | | / /
                                           2, 2, 1, 1, 1, // | | / / /
                                           3, 2, 1, 4, 1, // | | / - /
                                           3, 2, 1, 4, 4, // \ | / - -
                                           6, 5, 4, 4, 4  // - . - - -
                                       }));

    MaskedRaster<float> frictionMap(floatMeta, 1.f);

    auto result = ldd_dist(lddMap, pointsMap, frictionMap);

    MaskedRaster<float> expected(floatMeta, std::vector<float>({nan, 6, 6.83f, 7.65f, 8.48f,
                                                nan, 4, 4.82f, 5.65f, 7.65f,
                                                2.82f, 2, 2.83f, 4.83f, nan,
                                                nan, 0, nan, nan, nan,
                                                nan, nan, nan, nan, nan}));

    CHECK_RASTER_NEAR_WITH_TOLERANCE(expected, result, 0.02);
}

TEST_CASE("AccufluxTest.FluxOrigin")
{
    constexpr auto nan = std::numeric_limits<float>::quiet_NaN();

    RasterMetadata floatMeta(4, 4);
    floatMeta.nodata = nan;

    RasterMetadata meta(4, 4);
    meta.nodata = 0;

    MaskedRaster<uint8_t> lddMap(meta, std::vector<uint8_t>({2, 1, 2, 2,
                                           2, 1, 2, 2,
                                           2, 1, 3, 2,
                                           5, 4, 6, 5}));

    MaskedRaster<float> freightMap(floatMeta, std::vector<float>({1, 1, 1, 1,
                                                  2, 3, 4, 5,
                                                  1, 1, 1, 1,
                                                  1, 1, 1, 1}));

    MaskedRaster<float> fractionMap(floatMeta, std::vector<float>({0.25f, 0.25f, 0.25f, 0.25f,
                                                   0.25f, 0.25f, 0.25f, 0.25f,
                                                   0.25f, 0.25f, 0.25f, 0.25f,
                                                   0.25f, 0.25f, 0.25f, 0.25f}));

    MaskedRaster<int32_t> idMap(meta, std::vector<int32_t>({0, 0, 0, 0,
                                          0, 0, 0, 1,
                                          1, 0, 0, 0,
                                          0, 0, 0, 1}));

    auto result = flux_origin(lddMap, freightMap, fractionMap, idMap);

    MaskedRaster<float> expected(floatMeta, std::vector<float>({0.0625f, 0.0625f, 0.015625f, 0.25f,
                                                0.5f, 0.75f, 0.25f, 1.25f,
                                                0.25f, 0.f, 0.25f, 0.25f,
                                                0.f, 0.f, 0.25f, 0.25f}));

    CHECK_RASTER_EQ(expected, result);
}

TEST_CASE("AccufluxTest.LddValidate")
{
    constexpr auto nan = std::numeric_limits<float>::quiet_NaN();

    RasterMetadata floatMeta(4, 4);
    floatMeta.nodata = nan;

    RasterMetadata meta(4, 4);
    meta.nodata = 0;

    MaskedRaster<uint8_t> lddMap(meta, std::vector<uint8_t>({2, 0, 0, 0,
                                           2, 0, 0, 0,
                                           3, 0, 0, 0,
                                           0, 5, 0, 0}));

    CHECK(validate_ldd(lddMap, nullptr, nullptr, nullptr, nullptr));
}

class LddCbMock
{
public:
    MAKE_MOCK2(OnCb, void(int32_t, int32_t));

    std::function<void(int32_t, int32_t)> getCb()
    {
        return [this](int32_t r, int32_t c) {
            OnCb(r, c);
        };
    }
};

TEST_CASE("AccufluxTest.LddValidateOk")
{
    constexpr auto nan = std::numeric_limits<float>::quiet_NaN();

    RasterMetadata floatMeta(4, 4);
    floatMeta.nodata = nan;

    RasterMetadata meta(4, 4);
    meta.nodata = 0;

    LddCbMock loopCbMock, invalidValueCbMock, endsInNodataCbMock, outsideOfMapCbMock;

    MaskedRaster<uint8_t> lddMap(meta, std::vector<uint8_t>({2, 0, 0, 0,
                                           2, 0, 0, 0,
                                           3, 0, 0, 0,
                                           0, 5, 0, 0}));

    FORBID_CALL(loopCbMock, OnCb(ANY(int32_t), ANY(int32_t)));
    FORBID_CALL(invalidValueCbMock, OnCb(ANY(int32_t), ANY(int32_t)));
    FORBID_CALL(endsInNodataCbMock, OnCb(ANY(int32_t), ANY(int32_t)));
    FORBID_CALL(outsideOfMapCbMock, OnCb(ANY(int32_t), ANY(int32_t)));
    CHECK(validate_ldd(lddMap, loopCbMock.getCb(), invalidValueCbMock.getCb(), endsInNodataCbMock.getCb(), outsideOfMapCbMock.getCb()));
}

TEST_CASE("AccufluxTest.LddValidateContainsLoop")
{
    constexpr auto nan = std::numeric_limits<float>::quiet_NaN();

    RasterMetadata floatMeta(4, 4);
    floatMeta.nodata = nan;

    RasterMetadata meta(4, 4);
    meta.nodata = 0;

    LddCbMock loopCbMock, invalidValueCbMock, endsInNodataCbMock, outsideOfMapCbMock;

    MaskedRaster<uint8_t> lddMap(meta, std::vector<uint8_t>({2, 0, 3, 0,
                                           2, 0, 0, 2,
                                           3, 4, 0, 1,
                                           0, 8, 5, 0}));

    REQUIRE_CALL(loopCbMock, OnCb(2, 0));
    FORBID_CALL(invalidValueCbMock, OnCb(_, _));
    FORBID_CALL(endsInNodataCbMock, OnCb(_, _));
    FORBID_CALL(outsideOfMapCbMock, OnCb(_, _));
    CHECK_FALSE(validate_ldd(lddMap, loopCbMock.getCb(), invalidValueCbMock.getCb(), endsInNodataCbMock.getCb(), outsideOfMapCbMock.getCb()));
}

TEST_CASE("AccufluxTest.Catchment")
{
    RasterMetadata idMeta(5, 5);
    idMeta.nodata = -1;

    RasterMetadata meta(5, 5);
    meta.nodata = 0;

    MaskedRaster<int32_t> idMap(idMeta, std::vector<int32_t>({0, 0, 0, 0, 0,
                                            0, 0, 0, -1, 0,
                                            0, 4, 0, 0, 0,
                                            0, 3, 2, 0, 0,
                                            0, 0, 0, 0, 1}));

    MaskedRaster<uint8_t> lddMap(meta, std::vector<uint8_t>({2, 2, 2, 1, 1,
                                           2, 2, 1, 1, 1,
                                           3, 2, 1, 4, 1,
                                           3, 2, 1, 4, 4,
                                           6, 5, 4, 4, 4}));

    auto result = catchment(lddMap, idMap);

    MaskedRaster<int32_t> expected(idMeta, std::vector<int32_t>({3, 3, 3, 3, 3,
                                               3, 3, 3, 3, 3,
                                               3, 3, 3, 3, 2,
                                               0, 3, 2, 2, 2,
                                               0, 0, 0, 0, 1}));

    CHECK_RASTER_EQ(expected, result);
}

TEST_CASE("AccufluxTest.MaxUpstreamDist")
{
    RasterMetadata meta(5, 5);
    meta.nodata = 0;
    meta.set_cell_size(1.0);

    MaskedRaster<uint8_t> lddMap(meta, std::vector<uint8_t>({2, 2, 2, 1, 1,
                                           2, 2, 1, 1, 1,
                                           3, 2, 1, 4, 1,
                                           3, 2, 1, 4, 4,
                                           6, 5, 4, 4, 4}));

    auto result = max_upstream_dist(lddMap);

    auto diag = std::sqrt(1.f + 1.f);

    meta.nodata = std::numeric_limits<float>::quiet_NaN();
    MaskedRaster<float> expected(meta, std::vector<float>({0, 0, 0, 0, 0,
                                           1, 1, diag, diag, 0,
                                           2, 2 * diag, 2 * diag, diag, 0,
                                           0, 3 * diag, 1 + diag, diag, 0,
                                           0, 1 + 3 * diag, 2, 1, 0}));

    CHECK_RASTER_NEAR(expected, result);
}
}
