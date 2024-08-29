#include "gdx/algo/shape.h"
#include "gdx/exception.h"
#include "gdx/log.h"
#include "infra/gdal.h"

#include <cassert>
#include <stdexcept>

namespace gdx {

using namespace inf;
namespace gdal = inf::gdal;

static void process_points_from_line(gdal::LineCRef line, std::vector<Line<double>>& lines)
{
    auto iter          = begin(line);
    Point<double> prev = *iter;
    for (auto endIter = end(line); iter != endIter; ++iter) {
        lines.emplace_back(prev, *iter);
        prev = *iter;
    }
}

void read_shape_file(const fs::path& fileName,
                     std::vector<Point<double>>& points,
                     std::vector<Line<double>>& lines)
{
    auto dataSet = gdal::VectorDataSet::open(fileName, gdal::VectorType::ShapeFile);
    if (dataSet.layer_count() == 0) {
        throw RuntimeError("No data found in shape file");
    }

    for (const auto& feature : dataSet.layer(0)) {
        if (!feature.has_geometry()) {
            continue;
        }

        auto geometry = feature.geometry();
        switch (geometry.type()) {
        case gdal::Geometry::Type::Point:
            points.emplace_back(geometry.as<gdal::PointCRef>().point());
            break;
        case gdal::Geometry::Type::Line:
            process_points_from_line(geometry.as<gdal::LineCRef>(), lines);
            break;
        case gdal::Geometry::Type::MultiLine: {
            auto multiLine = geometry.as<gdal::MultiLineCRef>();
            for (int i = 0; i < multiLine.size(); ++i) {
                process_points_from_line(multiLine.line_at(i), lines);
            }
            break;
        }
        default:
            Log::warn("Unknown Geometry type");
        }
    }
}

std::vector<LineWithValue<float>> read_lines_with_value_from_dataset(inf::gdal::Layer layer, const std::string& fieldName)
{
    std::vector<LineWithValue<float>> result;
    result.reserve(layer.feature_count());

    const int fieldIndex = layer.layer_definition().required_field_index(fieldName);

    for (const auto& feature : layer) {
        if (!feature.has_geometry()) {
            continue;
        }

        auto geometry = feature.geometry();
        switch (geometry.type()) {
        case inf::gdal::Geometry::Type::Line: {
            LineWithValue<float> valuedLine;
            valuedLine.value = feature.field_as<float>(fieldIndex);
            details::process_points_from_line(geometry.as<inf::gdal::LineCRef>(), valuedLine.points);
            result.emplace_back(std::move(valuedLine));
            break;
        }
        case inf::gdal::Geometry::Type::MultiLine: {
            auto multiLine = geometry.as<inf::gdal::MultiLineCRef>();
            for (int i = 0; i < multiLine.size(); ++i) {
                LineWithValue<float> valuedLine;
                valuedLine.value = feature.field_as<float>(fieldIndex);
                details::process_points_from_line(multiLine.line_at(i), valuedLine.points);
                result.emplace_back(std::move(valuedLine));
            }
            break;
        }
        default:
            break;
        }
    }
    return result;
}

}
