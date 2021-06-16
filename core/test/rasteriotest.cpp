#include "gdx/test/testbase.h"
#include "testconfig.h"

#include "gdx/rasterspanio.h"

#ifdef GDX_ENABLE_SIMD
#include "gdx/denserasterio.h"
#endif

#include "gdx/algo/cast.h"
#include "gdx/maskedrasterio.h"
#include "gdx/sparserasterio.h"
#include "infra/crs.h"
#include "infra/string.h"
#include "infra/typeinfo.h"

#include <doctest/doctest.h>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <iterator>

#ifdef GDX_ENABLE_SIMD
TYPE_TO_STRING(gdx::DenseRaster<float>);
#endif

TYPE_TO_STRING(gdx::MaskedRaster<float>);
TYPE_TO_STRING(std::vector<float>);

namespace gdx::test {

using namespace inf;
using namespace doctest;
using namespace std::string_literals;

#ifdef GDX_ENABLE_SIMD
#define RasterIOTypes DenseRaster<float>,  \
                      MaskedRaster<float>, \
                      std::vector<float>

#define DenseRasterIntIOTypes DenseRaster<int32_t>, \
                              MaskedRaster<int32_t>

#define DenseRasterFloatIOTypes DenseRaster<float>, \
                                MaskedRaster<float>

#define ContiguousRasterTypes MaskedRaster<float>, \
                              DenseRaster<float>
#else
#define RasterIOTypes MaskedRaster<float>, \
                      std::vector<float>

#define DenseRasterIntIOTypes MaskedRaster<int32_t>

#define DenseRasterFloatIOTypes MaskedRaster<float>

#define ContiguousRasterTypes MaskedRaster<float>
#endif

TEST_CASE("read metadata")
{
    auto meta = inf::gdal::io::read_metadata(TEST_DATA_DIR "/testraster-nodata.asc");

    CHECK(meta.rows == 3);
    CHECK(meta.cols == 5);
    CHECK(meta.nodata.has_value());
    CHECK(*meta.nodata == Approx(-1.0));
    CHECK(meta.cellSize == Approx(4.0));
    CHECK(meta.xll == Approx(1.0));
    CHECK(meta.yll == Approx(-10.0));
}

TEST_CASE("read metadata with projection")
{
    auto meta = inf::gdal::io::read_metadata(fs::u8path(TEST_DATA_DIR) / "../../../test/mapdata/landusebyte.tif");
    CHECK(meta.projection_frienly_name() == "EPSG:31370");
}

// Testraster
// llx 1
// lly -10
// 0, 1, 2, 3, 4
// 5, 6, 7, 8, 9
// 4, 3, 2, 1, 0

TEST_CASE("read raster as int with extent which is contained")
{
    RasterMetadata extent(2, 2);
    extent.xll      = 9.0;
    extent.yll      = -10.0;
    extent.cellSize = 4.0;

    auto ras = read_masked_raster<int32_t>(TEST_DATA_DIR "/testraster.asc", extent);

    CHECK(ras.metadata().rows == 2);
    CHECK(ras.metadata().cols == 2);
    CHECK_FALSE(ras.metadata().nodata.has_value());
    CHECK(ras.metadata().cellSize == Approx(4.0));
    CHECK(ras.metadata().xll == Approx(9.0));
    CHECK(ras.metadata().yll == Approx(-10.0));

    std::vector<int32_t> expectedData{{7, 8, 2, 1}};

    CHECK_CONTAINER_EQ(ras, expectedData);
}

TEST_CASE("read raster as int with extent which is not contained bottom left")
{
    RasterMetadata extent(3, 4);
    extent.xll      = -3.0;
    extent.yll      = -14.0;
    extent.cellSize = 4.0;

    MaskedRaster<int32_t> ras;
    auto meta = read_raster(TEST_DATA_DIR "/testraster.asc", extent, ras);
    auto nod  = std::numeric_limits<int32_t>::max();

    CHECK(meta.rows == 3);
    CHECK(meta.cols == 4);
    CHECK(meta.nodata.has_value());
    CHECK(meta.cellSize == Approx(4.0));
    CHECK(meta.xll == Approx(-3.0));
    CHECK(meta.yll == Approx(-14.0));

    MaskedRaster<int32_t> expected(RasterMetadata(3, 4, double(nod)), std::vector<int32_t>{
                                                                          nod, 5, 6, 7,
                                                                          nod, 4, 3, 2,
                                                                          nod, nod, nod, nod});

    CHECK_RASTER_EQ(expected, ras);
}

TEST_CASE("read raster as int with extent which is not contained top left")
{
    RasterMetadata extent(3, 4);
    extent.xll      = -3.0;
    extent.yll      = -6.0;
    extent.cellSize = 4.0;

    MaskedRaster<int32_t> ras;
    auto meta = read_raster(TEST_DATA_DIR "/testraster.asc", extent, ras);

    CHECK(meta.rows == 3);
    CHECK(meta.cols == 4);
    CHECK(meta.nodata.has_value());
    CHECK(meta.cellSize == Approx(4.0));
    CHECK(meta.xll == Approx(-3.0));
    CHECK(meta.yll == Approx(-6.0));

    MaskedRaster<int32_t> expected(RasterMetadata(3, 4, -1), std::vector<int32_t>{
                                                                 -1, -1, -1, -1,
                                                                 -1, 0, 1, 2,
                                                                 -1, 5, 6, 7});

    CHECK(expected.tolerant_data_equal_to(ras));
}

TEST_CASE("read raster as int with extent which is not contained top right")
{
    RasterMetadata extent(3, 4);
    extent.xll      = 13.0;
    extent.yll      = -6.0;
    extent.cellSize = 4.0;

    MaskedRaster<int32_t> ras;
    auto meta = read_raster(TEST_DATA_DIR "/testraster.asc", extent, ras);

    CHECK(meta.rows == 3);
    CHECK(meta.cols == 4);
    CHECK(meta.nodata.has_value());
    CHECK(meta.cellSize == Approx(4.0));
    CHECK(meta.xll == Approx(13.0));
    CHECK(meta.yll == Approx(-6.0));

    MaskedRaster<int32_t> expected(RasterMetadata(3, 4, -1), std::vector<int32_t>{
                                                                 -1, -1, -1, -1,
                                                                 3, 4, -1, -1,
                                                                 8, 9, -1, -1});

    CHECK(expected.tolerant_data_equal_to(ras));
}

TEST_CASE("read_raster as int with extent which is not contained bottom right")
{
    RasterMetadata extent(3, 4);
    extent.xll      = 9.0;
    extent.yll      = -18.0;
    extent.cellSize = 4.0;

    MaskedRaster<int32_t> ras;
    auto meta = read_raster(TEST_DATA_DIR "/testraster.asc", extent, ras);

    CHECK(meta.rows == 3);
    CHECK(meta.cols == 4);
    CHECK(meta.nodata.has_value());
    CHECK(meta.cellSize == Approx(4.0));
    CHECK(meta.xll == Approx(9.0));
    CHECK(meta.yll == Approx(-18.0));

    MaskedRaster<int32_t> expected(RasterMetadata(3, 4, -1), std::vector<int32_t>{
                                                                 2, 1, 0, -1,
                                                                 -1, -1, -1, -1,
                                                                 -1, -1, -1, -1});

    CHECK(expected.tolerant_data_equal_to(ras));
}

TEST_CASE("read float raster as byte")
{
    MaskedRaster<uint8_t> ras;
    auto meta = read_raster(TEST_DATA_DIR "/floatraster-negative-nodata.tif", ras);

    CHECK(meta.rows == 40);
    CHECK(meta.cols == 49);
    CHECK(meta.nodata == 255);
    CHECK(meta.cellSize == 100);
    CHECK(meta.xll == 209400);
    CHECK(meta.yll == 217000);

    // The upper right corner contains nodata
    CHECK(ras.is_nodata(0, 48));
}

TEST_CASE("same cast and read behavior")
{
    // Casting while reading should have te same behavior as casting after reading

    MaskedRaster<uint8_t> ras1;
    MaskedRaster<float> ras2;

    read_raster(TEST_DATA_DIR "/floatraster-negative-nodata.tif", ras1);
    read_raster(TEST_DATA_DIR "/floatraster-negative-nodata.tif", ras2);
    auto ras2Uint = raster_cast<uint8_t>(ras2);

    CHECK(ras1 == ras2Uint);
    CHECK(ras1.metadata() == ras2Uint.metadata());
}

TEST_CASE("cast to float and back")
{
    std::vector<uint8_t> data = {
        255, 1, 2,
        3, 4, 5,
        0, 0, 255};

    RasterMetadata meta(3, 3);
    meta.nodata = 255;
    MaskedRaster<uint8_t> ras(meta, data);

    auto floatRaster = raster_cast<float>(ras);
    auto result      = raster_cast<uint8_t>(floatRaster);
    CHECK(ras == result);
    CHECK(ras.metadata() == result.metadata());
}

TEST_CASE("save raster as png")
{
    if (!gdal::RasterDriver::is_supported(gdal::RasterType::Png)) {
        return;
    }

    MaskedRaster<uint8_t> ras;
    read_raster(TEST_DATA_DIR "/testraster.asc", ras);
    CHECK_NOTHROW(write_raster<uint8_t>(ras, "test.png"));
}

TEST_CASE("save raster as gif")
{
    if (!gdal::RasterDriver::is_supported(gdal::RasterType::Gif)) {
        return;
    }

    MaskedRaster<uint8_t> ras;
    read_raster(TEST_DATA_DIR "/testraster.asc", ras);
    CHECK_NOTHROW(write_raster<uint8_t>(ras, "test.gif"));
}

TEST_CASE("save raster as tif")
{
    MaskedRaster<uint8_t> ras;
    read_raster(TEST_DATA_DIR "/testraster.asc", ras);
    CHECK_NOTHROW(write_raster<uint8_t>(ras, "test.tif"));
}

template <typename T>
void testRaster(std::string_view filename)
{
    MaskedRaster<int32_t> referenceRaster;
    referenceRaster.set_metadata(read_raster(fs::u8path(TEST_DATA_DIR) / std::string(filename), referenceRaster));
    write_raster(referenceRaster, "raster.asc");

    MaskedRaster<int32_t> writtenRaster;
    auto meta = read_raster<>("raster.asc", writtenRaster);

    compareMetaData(referenceRaster.metadata(), meta);
    CHECK_CONTAINER_EQ(referenceRaster, writtenRaster);
}

TEST_CASE("write raster as int")
{
    testRaster<int32_t>("testraster.asc");
}

TEST_CASE("write raster as float")
{
    testRaster<float>("testraster.asc");
}

TEST_CASE("read raster with nodata")
{
    MaskedRaster<float> ras;
    auto meta = read_raster(TEST_DATA_DIR "/testraster-nodata.asc", ras);

    CHECK(meta.rows == 3);
    CHECK(meta.cols == 5);
    CHECK(meta.nodata.value() == Approx(-1.0));
    CHECK(meta.cellSize == Approx(4.0));
    CHECK(meta.xll == Approx(1.0));
    CHECK(meta.yll == Approx(-10.0));

    MaskedRaster<float> expected(ras.metadata(), std::vector<float>{
                                                     0.f, 1.f, 2.f, 3.f, 4.f,
                                                     5.f, 6.f, 7.f, 8.f, 9.f,
                                                     4.f, 3.f, 2.f, -1.f, 0.f});

    CHECK(ras.is_nodata(2, 3));
    CHECK(expected.tolerant_equal_to(ras));
}

TEST_CASE("read raster with nodata as int")
{
    MaskedRaster<int32_t> ras;
    auto meta = read_raster(TEST_DATA_DIR "/testraster-nodata.asc", ras);

    CHECK(meta.rows == 3);
    CHECK(meta.cols == 5);
    CHECK(meta.nodata.value() == Approx(-1.0));
    CHECK(meta.cellSize == Approx(4.0));
    CHECK(meta.xll == Approx(1.0));
    CHECK(meta.yll == Approx(-10.0));

    std::vector<int32_t> expectedData = {
        0, 1, 2, 3, 4,
        5, 6, 7, 8, 9,
        4, 3, 2, -1, 0};

    CHECK(ras.is_nodata(2, 3));
    CHECK_CONTAINER_EQ(ras, expectedData)
}

TEST_CASE("read raster from memory")
{
    static const std::string raster =
        "ncols        5\n"
        "nrows        3\n"
        "xllcorner    1.000000000000\n"
        "yllcorner    -10.000000000000\n"
        "cellsize     4.000000000000\n"
        "NODATA_value  -1\n"
        "0 1 2 3 4\n"
        "5 6 7 8 9\n"
        "4 3 2 -1 0\n";

    inf::gdal::MemoryFile memFile("/vsimem/ras.asc", std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(raster.data()), raster.size()));
    MaskedRaster<int16_t> ras;
    auto meta = read_raster(memFile.path(), ras);
    CHECK(meta.rows == 3);
    CHECK(meta.cols == 5);
}

TEST_CASE("write raster with nodata float")
{
    MaskedRaster<float> referenceRaster;
    referenceRaster.set_metadata(read_raster(TEST_DATA_DIR "/testraster-nodata.asc", referenceRaster));
    write_raster(referenceRaster, "raster.asc");

    std::vector<std::string> expected({
        "ncols        5"s,
        "nrows        3"s,
        "xllcorner    1.000000000000"s,
        "yllcorner    -10.000000000000"s,
        "cellsize     4.000000000000"s,
        "NODATA_value  -1"s,
        "0.0 1 2 3 4"s,
        "5 6 7 8 9"s,
        "4 3 2 -1 0"s,
    });

    std::ifstream str("raster.asc");
    REQUIRE(str.is_open());
    std::vector<std::string> actual;
    std::string line;
    while (std::getline(str, line)) {
        actual.push_back(str::trim(line));
    }

    REQUIRE(expected.size() == actual.size());
    CHECK(std::equal(expected.begin(), expected.end(), actual.begin()));
}

TEST_CASE_TEMPLATE("raster io", RasterType, RasterIOTypes)
{
    SUBCASE("read raster as float")
    {
        RasterType raster;
        auto meta = read_raster(TEST_DATA_DIR "/testraster.asc", raster);

        RasterMetadata expectedMeta(3, 5);
        expectedMeta.cellSize = 4.0;
        expectedMeta.xll      = 1.0;
        expectedMeta.yll      = -10.0;
        compareMetaData(expectedMeta, meta);

        std::vector<float> expectedData = {
            0.f, 1.f, 2.f, 3.f, 4.f,
            5.f, 6.f, 7.f, 8.f, 9.f,
            4.f, 3.f, 2.f, 1.f, 0.f};

        CHECK_CONTAINER_EQ(expectedData, raster);
    }

    SUBCASE("read raster as int")
    {
        RasterType ras;
        auto meta = read_raster(TEST_DATA_DIR "/testraster.asc", ras);

        CHECK(meta.rows == 3);
        CHECK(meta.cols == 5);
        CHECK_FALSE(meta.nodata.has_value());
        CHECK(meta.cellSize == Approx(4.0));
        CHECK(meta.xll == Approx(1.0));
        CHECK(meta.yll == Approx(-10.0));

        std::vector<int32_t> expectedData = {
            0, 1, 2, 3, 4,
            5, 6, 7, 8, 9,
            4, 3, 2, 1, 0};

        CHECK_CONTAINER_EQ(expectedData, ras);
    }
}

TEST_CASE_TEMPLATE("float dense raster io", RasterType, DenseRasterFloatIOTypes)
{
    SUBCASE("warp raster")
    {
        RasterType referenceRaster;
        read_raster(TEST_DATA_DIR "/../../../test/mapdata/landusebyte.tif", referenceRaster);
        auto result = warp_raster(referenceRaster, crs::epsg::WGS84WebMercator);

        CHECK(referenceRaster.is_nodata(0, 0));
        CHECK(result.is_nodata(0, 0));

        CHECK_FALSE(referenceRaster.is_nodata(500, 1685));
        CHECK_FALSE(result.is_nodata(500, 1685));

        CHECK(crs::epsg::WGS84WebMercator == result.metadata().projected_epsg().value());
        CHECK(crs::epsg::WGS84 == result.metadata().geographic_epsg().value());
    }
}

TEST_CASE_TEMPLATE("contiguous raster io", RasterType, ContiguousRasterTypes)
{
    using T = typename RasterType::value_type;

    SUBCASE("resample to higher cellsize")
    {
        // clang-format off
        RasterMetadata meta(3, 2, 0);
        meta.cellSize = 100;
        meta.set_projection_from_epsg(crs::epsg::BelgianLambert72);

        RasterType ras(meta, std::vector<T>({
            1, 2,
            3, 4,
            5, 0,
            }));

        meta.rows = 6;
        meta.cols = 4;
        meta.cellSize = 50.0;
        const RasterType expected(meta, std::vector<T>({
            1, 1, 2, 2,
            1, 1, 2, 2,
            3, 3, 4, 4,
            3, 3, 4, 4,
            5, 5, 0, 0,
            5, 5, 0, 0,
            }));
        // clang-format on

        CHECK_RASTER_EQ(expected, resample_raster(ras, meta, gdal::ResampleAlgorithm::NearestNeighbour));
    }

    SUBCASE("resample to lower cellsize")
    {
        // clang-format off
        RasterMetadata meta(6, 4);
        meta.cellSize = 50.0;
        meta.set_projection_from_epsg(crs::epsg::BelgianLambert72);

        RasterType ras(meta, std::vector<T>({
            1, 2, 2, 3,
            3, 6, 4, 3,
            4, 5, 5, 6,
            6, 9, 7, 10,
            0, 0, 1, 1,
            0, 0, 1, 1,
        }));

        meta.rows = 3;
        meta.cols = 2;
        meta.cellSize = 100.0;
        const RasterType expectedNearest(meta, std::vector<T>({
            6, 3,
            9, 10,
            0, 1,
        }));
        const RasterType expectedAverage(meta, std::vector<T>({
            3, 3,
            6, 7,
            0, 1,
        }));
        const RasterType expectedMin(meta, std::vector<T>({
            1, 2,
            4, 5,
            0, 1,
        }));
        // clang-format on

        SUBCASE("neirest neighbour")
        {
            CHECK_RASTER_EQ(expectedNearest, resample_raster(ras, meta, gdal::ResampleAlgorithm::NearestNeighbour));
        }

        SUBCASE("average")
        {
            CHECK_RASTER_EQ(expectedAverage, resample_raster(ras, meta, gdal::ResampleAlgorithm::Average));
        }

        SUBCASE("minimum")
        {
            CHECK_RASTER_EQ(expectedMin, resample_raster(ras, meta, gdal::ResampleAlgorithm::Minimum));
        }
    }
}

TEST_CASE("store double dense raster as float, nodata needs adjustment")
{
    // This nodata value does not fit in a float
    RasterMetadata meta(3, 3, -1.7976931348623157e+308);
    meta.cellSize = 50;

    gdx::DenseRaster<double> ras(meta, std::vector<double>({-1.7976931348623157e+308, 1.0, -1.7976931348623157e+308,
                                           1.0, 1.0, 1.0,
                                           -1.7976931348623157e+308, 1.0, -1.7976931348623157e+308}));

    gdx::write_raster(ras, "/vsimem/infraster_double.tif");
    auto floatRas = gdx::read_dense_raster<float>("/vsimem/infraster_double.tif");
    CHECK(floatRas.is_nodata(0, 0));

    gdx::write_raster(floatRas, "/vsimem/infraster_float.tif");
    {
        auto resultRas = gdx::read_dense_raster<float>("/vsimem/infraster_float.tif");
        CHECK(resultRas.is_nodata(0, 0));
    }

    {
        auto resultRas = gdx::read_dense_raster<double>("/vsimem/infraster_float.tif");
        CHECK(resultRas.is_nodata(0, 0));
    }
}

TEST_CASE("store double masked raster as float, nodata needs adjustment")
{
    // This nodata value does not fit in a float
    RasterMetadata meta(3, 3, -1.7976931348623157e+308);
    meta.cellSize = 50;

    gdx::MaskedRaster<double> ras(meta, std::vector<double>({-1.7976931348623157e+308, 1.0, -1.7976931348623157e+308,
                                            1.0, 1.0, 1.0,
                                            -1.7976931348623157e+308, 1.0, -1.7976931348623157e+308}));

    gdx::write_raster(ras, "/vsimem/infraster_double.tif");
    auto floatRas = gdx::read_masked_raster<float>("/vsimem/infraster_double.tif");
    CHECK(floatRas.is_nodata(0, 0));

    gdx::write_raster(floatRas, "/vsimem/infraster_float.tif");
    {
        auto resultRas = gdx::read_masked_raster<float>("/vsimem/infraster_float.tif");
        CHECK(resultRas.is_nodata(0, 0));
    }

    {
        auto resultRas = gdx::read_masked_raster<double>("/vsimem/infraster_float.tif");
        CHECK(resultRas.is_nodata(0, 0));
    }
}

TEST_CASE("Write raster with different data type")
{
    RasterMetadata extent(2, 2);
    extent.xll      = 9.0;
    extent.yll      = -10.0;
    extent.cellSize = 4.0;

    auto path = fs::temp_directory_path() / "rasterio" / "int32.tif";

    auto ras = read_masked_raster<uint32_t>(fs::u8path(TEST_DATA_DIR) / "testraster.asc", extent);
    gdx::write_raster(ras, path, typeid(int32_t));

    CHECK(inf::type_name(gdal::io::get_raster_type(path)) == inf::type_name(typeid(int32_t)));
}

}
