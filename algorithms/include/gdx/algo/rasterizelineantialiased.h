#pragma once

#include "gdx/cell.h"
#include "gdx/point.h"
#include "gdx/rastermetadata.h"
#include "infra/gdalalgo.h"

namespace gdx {

namespace details {
void rasterize_lines_anti_aliased(
    inf::gdal::Layer linesLayer,
    const RasterMetadata& meta,
    const std::string& fieldName,           // action = f"-a {fieldName}" if fieldName != "" else "burn 1"
    const bool normalise_brightness,        // if true, the sum of burn-strength of a line is normalised to 1
    const bool multiply_by_length,          // if true, the value to be burned is MULTIPLIED by its length in km
    std::vector<std::vector<float>>& raster // Anti-aliased line rasterization only makes sense for float rasters
                                            // Not double, that's to large, and anti-aliasing is not precisely defined.
);

void rasterize_segment_anti_aliased(
    double xStart, double yStart, double xEnd, double yEnd, // x&y as double because Flanders in Lambert requires already 6 digits precision
    const gdx::RasterMetadata& meta,
    std::vector<std::pair<Cell, float /*brightness[0-1]*/>>& out);

void rasterize_segment_anti_aliased(
    float xStart, float yStart, float xEnd, float yEnd, // x&y as float to have it the same as in Weiss
    const gdx::RasterMetadata& meta,
    std::vector<std::pair<Cell, float /*brightness[0-1]*/>>& out);

void rasterize_segments_anti_aliased(
    const std::vector<std::vector<Point<float>>>& endPoints,
    const float value,
    const bool normalise_brightness,
    const gdx::RasterMetadata& meta,
    std::vector<std::vector<float>>& targetGrid);

void rasterize_segments_anti_aliased(
    const std::vector<std::vector<Point<double>>>& endPoints,
    const float value,
    const bool normalise_brightness,
    const gdx::RasterMetadata& meta,
    std::vector<std::vector<float>>& targetGrid);

} // details

template <typename ResultType>
ResultType rasterize_lines_anti_aliased(
    inf::gdal::Layer linesLayer,
    const RasterMetadata& meta,
    const std::string& fieldName,    // action = f"-a {fieldName}" if fieldName != "" else "burn 1"
    const bool normalise_brightness, // if true, the sum of burn-strength of a line is normalised to 1
    const bool multiply_by_length    // if true, the value to be burned is MULTIPLIED by its length in km
)
{
    auto resultMeta   = meta;
    resultMeta.nodata = 0.0;

    ResultType result(resultMeta);
    using T = typename ResultType::value_type;

    std::vector<std::vector<float>> raster;
    details::rasterize_lines_anti_aliased(linesLayer, resultMeta, fieldName, normalise_brightness, multiply_by_length, raster);
    for (int r = 0; r < result.rows(); ++r) {
        for (int c = 0; c < result.cols(); ++c) {
            const auto v = static_cast<T>(raster[r][c]);
            if (v != 0) {
                result.mark_as_data(r, c);
                result(r, c) = static_cast<T>(raster[r][c]);
            } else {
                result(r, c) = 0;
                result.mark_as_nodata(r, c);
            }
        }
    }

    return result;
}

}
