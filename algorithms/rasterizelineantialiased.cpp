#include "gdx/algo/rasterizelineantialiased.h"
#include "gdx/log.h"
#include "gdx/rastermetadata.h"
#include "infra/gdal.h"

#include <cassert>
#include <cmath>
#include <utility>
#include <vector>

namespace gdx {

using namespace inf;

static int ipart(double value)
{
    return (int)floor(value);
}

static double fpart(double value)
{
    return value - ipart(value);
}

static void plot(int ry, int cx, float frac, bool reverse, int rows, int cols, std::vector<std::pair<Cell, float>>& output)
{
    assert(frac >= 0);
    if (reverse) {
        // cells outside the grid might be colored, so we need to check for this and
        // only add the cells inside the grid to the output.
        if (cx >= 0 && cx < rows && ry >= 0 && ry < cols) {
            output.emplace_back(Cell(cx, ry), frac);
        }
    } else {
        if (ry >= 0 && ry < rows && cx >= 0 && cx < cols) {
            output.emplace_back(Cell(ry, cx), frac);
        }
    }
}

template <typename TReal>
static TReal computeLength(const TReal x0, const TReal y0, const TReal x1, const TReal y1)
{
    const TReal dx = x0 - x1;
    const TReal dy = y0 - y1;
    return sqrt(dx * dx + dy * dy);
}

template <typename TReal>
static TReal computeLength(const std::vector<Point<TReal>>& endPoints)
{
    TReal result = 0;
    for (int i = 1; i < int(endPoints.size()); i++) {
        result += computeLength(endPoints[i - 1].x, endPoints[i - 1].y,
            endPoints[i].x, endPoints[i].y);
    }
    return result;
}

template <typename TReal>
static TReal computeLength(const std::vector<std::vector<Point<TReal>>>& multi_line)
{
    TReal result = 0;
    for (int i = 0; i < int(multi_line.size()); i++) {
        result += computeLength(multi_line[i]);
    }
    return result;
}

template <typename TReal>
void process_points_from_line(const inf::gdal::Line& line, std::vector<Point<TReal>>& endPoints)
{
    endPoints.clear();
    for (auto iter = begin(line); iter != end(line); ++iter) {
        TReal x = static_cast<TReal>(iter->x), y = static_cast<TReal>(iter->y);
        endPoints.emplace_back(x, y);
    }
}

template <typename TReal>
static std::vector<std::pair<std::vector<std::vector<Point<TReal>>>, float>> extractLinesFromVectorLayer(
    inf::gdal::Layer linesLayer,
    const std::string& fieldname)
{
    std::vector<std::pair<std::vector<std::vector<Point<TReal>>>, float>> result;
    std::vector<std::vector<Point<TReal>>> endPoints;
    int fieldIndex = (fieldname != "" ? linesLayer.field_index(fieldname) : -1);

    for (auto& feature : linesLayer) {
        float value = (fieldname != "" ? feature.field_as<float>(fieldIndex) : 1.0f);

        if (!feature.has_geometry()) {
            continue;
        }

        auto geometry = feature.geometry();
        switch (geometry.type()) {
        case inf::gdal::Geometry::Type::Line:
            endPoints.resize(1);
            process_points_from_line(geometry.as<inf::gdal::Line>(), endPoints[0]);
            result.push_back(std::pair(endPoints, value));
            break;
        case inf::gdal::Geometry::Type::MultiLine: {
            auto multiLine = geometry.as<inf::gdal::MultiLine>();
            endPoints.resize(multiLine.size());
            for (int i = 0; i < multiLine.size(); ++i) {
                process_points_from_line(multiLine.line_at(i), endPoints[i]);
            }
            result.push_back(std::pair(endPoints, value));
            break;
        }
        default:
            break;
        }
    }
    return result;
}

template <typename TReal>
static void rasterize_segment_anti_aliased_impl(
    TReal xStart, TReal yStart, TReal xEnd, TReal yEnd, // x&y as float to have it the same as in Weiss
    const gdx::RasterMetadata& meta,
    std::vector<std::pair<Cell, float /*brightness[0-1]*/>>& out)
{
    const TReal cellSize = static_cast<TReal>(meta.cellSize);
    const TReal llx      = static_cast<TReal>(meta.xll);
    const TReal lly      = static_cast<TReal>(meta.yll);

    if ((xStart == xEnd) && (yStart == yEnd)) {
        return;
    }
    if (((xStart < llx) && (xEnd < llx)) || ((yStart < lly) && (yEnd < lly)) ||
        ((xStart > llx + meta.cols * cellSize) && (xEnd > llx + meta.cols * cellSize)) ||
        ((yStart > lly + meta.rows * cellSize) && (yEnd > lly + meta.rows * cellSize))) {
        return;
    }

    if (xStart < llx) xStart = llx;
    if (yStart < lly) yStart = lly;
    if (xEnd < llx) xEnd = llx;
    if (yEnd < lly) yEnd = lly;
    if (xStart > llx + meta.cols * cellSize) xStart = llx + meta.cols * cellSize;
    if (yStart > lly + meta.rows * cellSize) yStart = lly + meta.rows * cellSize;
    if (xEnd > llx + meta.cols * cellSize) xEnd = llx + meta.cols * cellSize;
    if (yEnd > lly + meta.rows * cellSize) yEnd = lly + meta.rows * cellSize;

    double x1 = (xStart - llx) / cellSize - 0.5;
    double y1 = meta.rows - (yStart - lly) / cellSize - 0.5;
    double x2 = (xEnd - llx) / cellSize - 0.5;
    double y2 = meta.rows - (yEnd - lly) / cellSize - 0.5;

    double dx = x2 - x1;
    double dy = y2 - y1;
    if (fabs(dx) > fabs(dy)) { // horizontal(ish) lines
        if (x2 < x1) {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }
        double gradient = dy / dx;

        double xbegin = round(x1);
        double ybegin = y1 + gradient * (xbegin - x1);
        double xend   = round(x2);
        double yend   = y2 + gradient * (xend - x2);

        if (xbegin == xend) {
            // small line less than one pixel long
            double xgap = fpart(x2 + 0.5) - fpart(x1 + 0.5);

            auto xpxl1 = (int)xbegin;
            int ypxl1  = ipart(ybegin);

            plot(xpxl1, ypxl1, (float)((1.0 - fpart(ybegin)) * xgap), true, meta.rows, meta.cols, out);
            plot(xpxl1, ypxl1 + 1, (float)(fpart(ybegin) * xgap), true, meta.rows, meta.cols, out);
        } else {
            double xgap = 1.0 - fpart(x1 + 0.5);

            auto xpxl1 = (int)xbegin;
            int ypxl1  = ipart(ybegin);

            plot(xpxl1, ypxl1, (float)((1.0 - fpart(ybegin)) * xgap), true, meta.rows, meta.cols, out);
            plot(xpxl1, ypxl1 + 1, (float)(fpart(ybegin) * xgap), true, meta.rows, meta.cols, out);
            double intery = ybegin + gradient;

            xgap = fpart(x2 + 0.5);

            auto xpxl2 = (int)xend;
            int ypxl2  = ipart(yend);

            plot(xpxl2, ypxl2, (float)((1.0 - fpart(yend)) * xgap), true, meta.rows, meta.cols, out);
            plot(xpxl2, ypxl2 + 1, (float)(fpart(yend) * xgap), true, meta.rows, meta.cols, out);

            int x;
            for (x = xpxl1 + 1; x <= (xpxl2 - 1); x++) {
                plot(x, ipart(intery), (float)(1.0 - fpart(intery)), true, meta.rows, meta.cols, out);
                plot(x, ipart(intery) + 1, (float)fpart(intery), true, meta.rows, meta.cols, out);
                intery += gradient;
            }
        }
    } else { // vertical(ish) line
        if (y2 < y1) {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }
        double gradient = dy == 0.0 ? 0.0 : dx / dy;

        double ybegin = round(y1);
        double xbegin = x1 + gradient * (ybegin - y1);
        double yend   = round(y2);
        double xend   = x2 + gradient * (yend - y2);

        if (ybegin == yend) {
            // small line less than one pixel high
            double ygap = fpart(y2 + 0.5) - fpart(y1 + 0.5);

            auto ypxl1 = (int)ybegin;
            int xpxl1  = ipart(xbegin);

            plot(xpxl1, ypxl1, (float)((1.0 - fpart(xbegin)) * ygap), true, meta.rows, meta.cols, out);
            plot(xpxl1 + 1, ypxl1, (float)(fpart(xbegin) * ygap), true, meta.rows, meta.cols, out); // xpxl1+1, ypxl1 ?
        } else {
            double ygap = 1.0 - fpart(y1 + 0.5);

            auto ypxl1 = (int)ybegin;
            int xpxl1  = ipart(xbegin);

            plot(xpxl1, ypxl1, (float)((1.0 - fpart(xbegin)) * ygap), true, meta.rows, meta.cols, out);
            plot(xpxl1 + 1, ypxl1, (float)(fpart(xbegin) * ygap), true, meta.rows, meta.cols, out); // xpxl1+1, ypxl1 ?
            double interx = xbegin + gradient;

            ygap = fpart(y2 + 0.5);

            auto ypxl2 = (int)yend;
            int xpxl2  = ipart(xend);

            plot(xpxl2, ypxl2, (float)((1.0 - fpart(xend)) * ygap), true, meta.rows, meta.cols, out);
            plot(xpxl2 + 1, ypxl2, (float)(fpart(xend) * ygap), true, meta.rows, meta.cols, out); // xpxl1+1, ypxl1 ?

            int y;
            for (y = ypxl1 + 1; y <= (ypxl2 - 1); y++) {
                plot(ipart(interx), y, (float)(1.0 - fpart(interx)), true, meta.rows, meta.cols, out);
                plot(ipart(interx) + 1, y, (float)fpart(interx), true, meta.rows, meta.cols, out);
                interx += gradient;
            }
        }
    }
}

template <typename TReal>
void rasterize_segments_anti_aliased_impl(
    const std::vector<std::vector<Point<TReal>>>& endPoints, // [multi_line][segments]
    const float value,
    const bool normalise_brightness,
    const gdx::RasterMetadata& meta,
    std::vector<std::vector<float>>& targetGrid)
{
    std::vector<std::pair<Cell, float /*brightness[0-1]*/>> locations;
    for (int multi_line = 0; multi_line < int(endPoints.size()); ++multi_line) {
        const auto& p = endPoints[multi_line];
        for (int i = 1; i < int(p.size()); i++) {
            rasterize_segment_anti_aliased_impl<TReal>(p[i - 1].x, p[i - 1].y,
                p[i].x, p[i].y, meta, locations);
        }
    }
    if (normalise_brightness) {
        float sum_brightness = 0.0f;
        for (const auto& loc : locations) {
            sum_brightness += loc.second;
        }
        for (auto& loc : locations) {
            loc.second /= sum_brightness;
        }
    }
    for (const auto& loc : locations) {
        const int ry = loc.first.r, cx = loc.first.c;
        targetGrid[ry][cx] += value * loc.second;
    }
}

template <typename TReal>
static void rasterize_lines_anti_aliased(
    const std::vector<std::pair<std::vector<std::vector<Point<TReal>>>, float /*value_to_burn*/>>& lines,
    const bool normalise_brightness,
    const bool multiply_by_length,
    const gdx::RasterMetadata& meta,
    std::vector<std::vector<float>>& targetGrid)
{
    targetGrid.resize(meta.rows);
    for (int ry = 0; ry < meta.rows; ++ry) {
        targetGrid[ry].assign(meta.cols, 0.0f);
    }
    for (auto& line : lines) {
        float value = line.second;
        if (multiply_by_length) {
            value *= static_cast<float>(computeLength(line.first)) / 1000.0f;
        }

        rasterize_segments_anti_aliased_impl<TReal>(line.first, value, normalise_brightness, meta, targetGrid);
    }
}

void details::rasterize_lines_anti_aliased(
    inf::gdal::Layer linesLayer,
    const RasterMetadata& meta,
    const std::string& fieldName, // action = f"-a {fieldName}" if fieldName != "" else "burn 1"
    const bool normalise_brightness,
    const bool multiply_by_length,
    std::vector<std::vector<float>>& raster // anti-aliased line rasterization only makes sense for float rasters
)
{
    bool weissCompatibilityTest = true;
    // The Weiss compatibility tests were done in April 2019, to test if "weiss 2.0 with gdx-algo's" has the same outcome as Weiss 1.0.
    // It turned out that Weiss 1.0 anti-aliasing rasterisation lost some precision in its workflow (double --> float --> double),
    // while the actual work was done on double precision. The difference resulted in some cells having a non-zero anti-aliasing value
    // instead of 0, which resulted in turn to give an error in the comparison of Weiss 1.0 and "weiss 2.0 with gdx algo's"
    // because the relative difference between 0 and a non-zero value always exceed a relative error difference threshold of 0.0001.
    // To have the same results, the same loss of of precision was recreated by using the template<float> below.
    if (weissCompatibilityTest) {
        // TODO : remove the 'then' part code when Weiss tests are done.
        auto lines = extractLinesFromVectorLayer<float>(linesLayer, fieldName);
        gdx::rasterize_lines_anti_aliased(lines, normalise_brightness, multiply_by_length, meta, raster);
    } else {
        // TODO : always use 'else' part code when Weiss tests are done.
        auto lines = extractLinesFromVectorLayer<double>(linesLayer, fieldName);
        gdx::rasterize_lines_anti_aliased(lines, normalise_brightness, multiply_by_length, meta, raster);
    }
}

void details::rasterize_segment_anti_aliased(
    float xStart, float yStart, float xEnd, float yEnd, // x&y as float to have it the same as in Weiss
    const gdx::RasterMetadata& meta,
    std::vector<std::pair<Cell, float /*brightness[0-1]*/>>& out)
{
    return rasterize_segment_anti_aliased_impl<float>(xStart, yStart, xEnd, yEnd, meta, out);
}

void details::rasterize_segment_anti_aliased(
    double xStart, double yStart, double xEnd, double yEnd, // x&y as double because Flanders in Lambert requires already 6 digits precision
    const gdx::RasterMetadata& meta,
    std::vector<std::pair<Cell, float /*brightness[0-1]*/>>& out)
{
    return rasterize_segment_anti_aliased_impl<double>(xStart, yStart, xEnd, yEnd, meta, out);
}

void details::rasterize_segments_anti_aliased(
    const std::vector<std::vector<Point<float>>>& endPoints,
    const float value,
    const bool normalise_brightness,
    const gdx::RasterMetadata& meta,
    std::vector<std::vector<float>>& targetGrid)
{
    rasterize_segments_anti_aliased_impl<float>(endPoints, value, normalise_brightness, meta, targetGrid);
}

void details::rasterize_segments_anti_aliased(
    const std::vector<std::vector<Point<double>>>& endPoints,
    const float value,
    const bool normalise_brightness,
    const gdx::RasterMetadata& meta,
    std::vector<std::vector<float>>& targetGrid)
{
    rasterize_segments_anti_aliased_impl<double>(endPoints, value, normalise_brightness, meta, targetGrid);
}

}
