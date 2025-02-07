#include "gdx/config.h"

#include "gdx/maskedraster.h"
#include "gdx/rasterdiff.h"
#include "gdx/sparseraster.h"
#include "infra/string.h"

#ifdef GDX_ENABLE_SIMD
#include "gdx/denseraster.h"
#endif

#ifndef GDX_NO_DOCTEST
#include <doctest/doctest.h>
#include <infra/test/containerasserts.h>
#endif

#ifdef GDX_GTEST
#include "gdx/test/printsupport.h"
#include <gtest/gtest.h>
#endif

#include <algorithm>

template <typename T>
static std::string print_raster_data(const gdx::MaskedRaster<T>& lhs, const gdx::MaskedRaster<T>& rhs)
{
    std::stringstream ss;
    ss << "--- Expected data:\n"
       << lhs.eigen_data() << "\n--- Actual data:\n"
       << rhs.eigen_data()
       << "\n--- Expected mask:\n"
       << (lhs).mask_data() << "\n--- Actual mask:\n"
       << (rhs).mask_data();

    return ss.str();
}

template <typename Map>
static std::string print_map_data(const Map& lhs, const Map& rhs)
{
    std::stringstream ss;
    ss << "--- Expected data:\n"
       << print_map(lhs)
       << "\n--- Actual data:\n"
       << print_map(rhs);

    return ss.str();
}

#ifdef GDX_ENABLE_SIMD
template <typename T>
static std::string print_raster_data(const gdx::DenseRaster<T>& lhs, const gdx::DenseRaster<T>& rhs)
{
    std::stringstream ss;
    ss << "--- Expected raster:\n"
       << lhs.to_string() << "\n--- Actual raster:\n"
       << rhs.to_string();

    return ss.str();
}
#endif

template <typename T>
static std::string print_raster_data(const gdx::SparseRaster<T>& lhs, const gdx::SparseRaster<T>& rhs)
{
    std::stringstream ss;
    ss << "--- Expected raster:\n"
       << lhs.to_string() << "\n--- Actual raster:\n"
       << rhs.to_string();

    return ss.str();
}

template <typename Raster>
static std::string print_raster_diff(const Raster& lhs, const Raster& rhs, double tolerance)
{
    if (lhs.rows() <= 10 && lhs.cols() <= 10) {
        return print_raster_data(lhs, rhs);
    }

    auto diff = diff_rasters(lhs, rhs, tolerance);
    std::stringstream ss;
    ss << fmt::format("# matches:\t\t{}\n", diff.equal);

    if (diff.dataDifference) {
        ss << fmt::format("# mismatches:\t{}\n", diff.dataDifference);
    }

    if (diff.zeroToNonZero) {
        ss << fmt::format("# zero -> non zero:\t{}\n", diff.zeroToNonZero);
    }

    if (diff.nonZeroToZero) {
        ss << fmt::format("# non zero -> zero:\t{}\n", diff.nonZeroToZero);
    }

    if (diff.zeroToNodata) {
        ss << fmt::format("# zero -> nodata:\t{}\n", diff.zeroToNodata);
    }

    if (diff.nonZeroToNodata) {
        ss << fmt::format("# non zero -> nodata:\t{}\n", diff.nonZeroToNodata);
    }

    if (diff.nodataToZero) {
        ss << fmt::format("# nodata -> zero:\t{}\n", diff.nodataToZero);
    }

    if (diff.nodataToNonZero) {
        ss << fmt::format("# nodata -> non zero:\t{}\n", diff.nodataToNonZero);
    }

    return ss.str();
}

template <typename Raster>
static std::string print_raster_diff_meta_check(const Raster& lhs, const Raster& rhs, double tolerance)
{
    if (lhs.metadata() != rhs.metadata()) {
        std::stringstream ss;
        ss << "Metadata mismatch\n";
        ss << lhs.metadata() << "\n";
        ss << rhs.metadata() << "\n";
        return ss.str();
    } else {
        return print_raster_diff(lhs, rhs, tolerance);
    }
}

#ifdef GDX_GTEST
// gtest
#define EXPECT_RASTER_EQ(expected, actual) \
    EXPECT_EQ((expected), (actual)) << print_raster_diff(expected, actual, 0.0);

#define EXPECT_RASTER_NEAR(expected, actual) \
    EXPECT_TRUE(expected.tolerant_equal_to((actual))) << print_raster_diff_meta_check(expected, actual, std::numeric_limits<double>::epsilon());

#define EXPECT_RASTER_NEAR_WITH_TOLERANCE(expected, actual, tolerance) \
    EXPECT_TRUE(expected.tolerant_equal_to((actual), tolerance)) << print_raster_diff_meta_check(expected, actual, tolerance);

#define EXPECT_RASTER_NE(expected, actual) \
    EXPECT_NE((expected), (actual)) << print_raster_diff(expected, actual, 0.0);

#endif

#ifndef GDX_NO_DOCTEST
// doctest

template <typename T, template <typename> typename RasterType>
void check_rasters_near(const RasterType<T>& lhs, const RasterType<T>& rhs, double tolerance)
{
    CHECK_MESSAGE(lhs.tolerant_equal_to(rhs, inf::truncate<T>(tolerance)), print_raster_diff_meta_check(lhs, rhs, tolerance));
}

#define CHECK_RASTER_EQ(lhs, rhs) \
    CHECK_MESSAGE((lhs) == (rhs), print_raster_diff(lhs, rhs, 0.0));

#define CHECK_RASTER_NEAR(lhs, rhs) \
    check_rasters_near((lhs), (rhs), std::numeric_limits<double>::epsilon())

#define CHECK_RASTER_NEAR_WITH_TOLERANCE(lhs, rhs, tolerance) \
    check_rasters_near((lhs), (rhs), (tolerance));

#define CHECK_RASTER_NE(lhs, rhs) \
    CHECK_MESSAGE((lhs) != (rhs), print_raster_diff(lhs, rhs, 0.0));

template <typename T>
bool nan_equal_to(T lhs, T rhs)
{
    if (std::isnan(lhs)) { return std::isnan(rhs); }
    if (std::isnan(rhs)) { return false; }
    return lhs == rhs;
}

#endif
