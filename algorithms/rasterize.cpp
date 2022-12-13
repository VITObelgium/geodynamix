#include "gdx/algo/rasterize.h"
#include <gdx/rasteriterator.h>

#include "infra/chrono.h"
#include "infra/geometry.h"
#include "infra/log.h"

#include <algorithm>
#include <execution>

#ifdef GDX_HAVE_GEOS
#include <geos/geom/Geometry.h>
#include <geos/geom/prep/PreparedGeometryFactory.h>
#endif

namespace gdx::internal {

using namespace inf;

#ifdef GDX_HAVE_GEOS

static GeoMetadata create_geometry_extent(const geos::geom::Geometry& geom, const GeoMetadata& gridExtent)
{
    GeoMetadata geometryExtent = gridExtent;

    const auto* env = geom.getEnvelopeInternal();

    Rect<double> geomRect;

    auto topLeft     = Point<double>(env->getMinX(), env->getMaxY());
    auto bottomRight = Point<double>(env->getMaxX(), env->getMinY());

    const auto topLeftCell     = gridExtent.convert_point_to_cell(topLeft);
    const auto bottomRightCell = gridExtent.convert_point_to_cell(bottomRight);

    const auto topLeftLL     = gridExtent.convert_cell_ll_to_xy(topLeftCell);
    const auto bottomRightLL = gridExtent.convert_cell_ll_to_xy(bottomRightCell);

    geomRect.topLeft     = Point<double>(topLeftLL.x, topLeftLL.y - gridExtent.cell_size_y());
    geomRect.bottomRight = Point<double>(bottomRightLL.x + gridExtent.cell_size_x(), bottomRightLL.y);

    geometryExtent.xll  = geomRect.topLeft.x;
    geometryExtent.yll  = geomRect.bottomRight.y;
    geometryExtent.cols = (bottomRightCell.c - topLeftCell.c) + 1;
    geometryExtent.rows = (bottomRightCell.r - topLeftCell.r) + 1;

    return geometryExtent;
}

static inf::GeoMetadata create_geometry_extent(const geos::geom::Geometry& geom, const inf::GeoMetadata& gridExtent, const gdal::SpatialReference& sourceProjection)
{
    gdal::SpatialReference destProj(gridExtent.projection);

    if (sourceProjection.epsg_cs() != destProj.epsg_cs()) {
        auto outputGeometry = geom.clone();
        geom::CoordinateWarpFilter warpFilter(sourceProjection.export_to_wkt().c_str(), gridExtent.projection.c_str());
        outputGeometry->apply_rw(warpFilter);
        return create_geometry_extent(*outputGeometry, gridExtent);
    } else {
        return create_geometry_extent(geom, gridExtent);
    }
}

static GeoMetadata create_geometry_intersection_extent(const geos::geom::Geometry& geom, const GeoMetadata& gridExtent)
{
    GeoMetadata geometryExtent = gridExtent;

    const auto* env = geom.getEnvelopeInternal();

    Rect<double> geomRect;
    geomRect.topLeft     = Point<double>(env->getMinX(), env->getMaxY());
    geomRect.bottomRight = Point<double>(env->getMaxX(), env->getMinY());

    auto intersect = rectangle_intersection(geomRect, gridExtent.bounding_box());
    if (!intersect.is_valid() || intersect.width() == 0 || intersect.height() == 0) {
        // no intersection
        return {};
    }

    auto topLeftCell     = gridExtent.convert_point_to_cell(intersect.topLeft);
    auto bottomRightCell = gridExtent.convert_point_to_cell(intersect.bottomRight);

    auto lowerLeft = gridExtent.convert_cell_ll_to_xy(Cell(bottomRightCell.r, topLeftCell.c));

    geometryExtent.xll  = lowerLeft.x;
    geometryExtent.yll  = lowerLeft.y;
    geometryExtent.cols = std::max(0, (bottomRightCell.c - topLeftCell.c) + 1);
    geometryExtent.rows = std::max(0, (bottomRightCell.r - topLeftCell.r) + 1);

    return geometryExtent;
}

static GeoMetadata create_geometry_intersection_extent(const geos::geom::Geometry& geom, const inf::GeoMetadata& gridExtent, const gdal::SpatialReference& sourceProjection)
{
    gdal::SpatialReference destProj(gridExtent.projection);

    if (sourceProjection.epsg_cs() != destProj.epsg_cs()) {
        auto outputGeometry = geom.clone();
        geom::CoordinateWarpFilter warpFilter(sourceProjection.export_to_wkt().c_str(), gridExtent.projection.c_str());
        outputGeometry->apply_rw(warpFilter);
        return create_geometry_intersection_extent(*outputGeometry, gridExtent);
    } else {
        return create_geometry_intersection_extent(geom, gridExtent);
    }
}

static std::vector<PolygonCellCoverage::CellInfo> create_cell_coverages(const GeoMetadata& extent, const GeoMetadata& polygonExtent, const geos::geom::Geometry& geom)
{
    std::vector<PolygonCellCoverage::CellInfo> result;

    auto preparedGeom = geos::geom::prep::PreparedGeometryFactory::prepare(&geom);

    const auto cellSize = extent.cellSize;
    const auto cellArea = std::abs(cellSize.x * cellSize.y);

    for (auto cell : gdx::RasterCells(polygonExtent.rows, polygonExtent.cols)) {
        const Rect<double> box = polygonExtent.bounding_box(cell);
        const auto cellGeom    = geom::create_polygon(box.topLeft, box.bottomRight);

        // Intersect it with the polygon
        auto xyCentre   = polygonExtent.convert_cell_centre_to_xy(cell);
        auto outputCell = extent.convert_point_to_cell(xyCentre);

        if (preparedGeom->within(cellGeom.get())) {
            result.emplace_back(outputCell, 1.0);
        } else if (preparedGeom->intersects(cellGeom.get())) {
            auto intersectGeometry   = preparedGeom->getGeometry().intersection(cellGeom.get());
            const auto intersectArea = intersectGeometry->getArea();
            if (intersectArea > 0) {
                result.emplace_back(outputCell, intersectArea / geom.getArea());
            }
        }
    }

    return result;
}

static PolygonCellCoverage create_polygon_coverage(uint64_t polygonId,
                                                   const geos::geom::Geometry& geom,
                                                   const gdal::SpatialReference& geometryProjection,
                                                   const GeoMetadata& outputExtent)
{
    PolygonCellCoverage cov;

    const geos::geom::Geometry* geometry = &geom;
    geos::geom::Geometry::Ptr warpedGeometry;

    if (geometryProjection.epsg_cs() != outputExtent.projected_epsg()) {
        // clone the country geometry and warp it to the output grid projection
        warpedGeometry = geom.clone();
        geom::CoordinateWarpFilter warpFilter(outputExtent.projection.c_str(), outputExtent.projection.c_str());
        warpedGeometry->apply_rw(warpFilter);
        geometry = warpedGeometry.get();
    }

    cov.id                  = polygonId;
    cov.outputSubgridExtent = create_geometry_extent(*geometry, outputExtent, geometryProjection);
    cov.cells               = create_cell_coverages(outputExtent, cov.outputSubgridExtent, *geometry);

    return cov;
}

#else
static PolygonCellCoverage create_polygon_coverage(uint64_t /*polygonId*/,
                                                   const geos::geom::Geometry& /*geom*/,
                                                   const gdal::SpatialReference& /*geometryProjection*/,
                                                   const GeoMetadata& /*outputExtent*/)
{
    throw RuntimeError("GeoDynamiX was not compiled with geometry support");
}
#endif

std::vector<PolygonCellCoverage> create_polygon_coverages(const inf::GeoMetadata& outputExtent,
                                                          gdal::VectorDataSet& vectorDs,
                                                          std::variant<std::string, double> burnValue,
                                                          const std::string& inputLayer,
                                                          const ProgressInfo::Callback& progressCb)
{
    std::vector<PolygonCellCoverage> result;
    std::vector<std::tuple<int64_t, double, geos::geom::Geometry::Ptr>> geometries;

    for (int32_t i = 0; i < vectorDs.layer_count(); ++i) {
        auto layer = vectorDs.layer(i);
        if (!inputLayer.empty() && layer.name() != inputLayer) {
            continue;
        }

        int32_t valueColumn = -1;
        if (std::holds_alternative<std::string>(burnValue)) {
            valueColumn = layer.layer_definition().required_field_index(std::get<std::string>(burnValue));
        }

        assert(!outputExtent.projection.empty());
        if (!layer.projection().has_value()) {
            throw RuntimeError("Invalid input vector: No projection information available");
        }

        if (outputExtent.geographic_epsg() != layer.projection()->epsg_geog_cs()) {
            throw RuntimeError("Projection mismatch between input vector and metadata grid EPSG:{} <-> EPSG:{}", outputExtent.geographic_epsg().value(), layer.projection()->epsg_geog_cs().value());
        }

        const auto bbox = outputExtent.bounding_box();
        layer.set_spatial_filter(bbox.topLeft, bbox.bottomRight);

        for (auto& feature : layer) {
            double value = valueColumn == -1 ? std::get<double>(burnValue) : feature.field_as<double>(valueColumn);
            geometries.emplace_back(feature.id(), value, geom::gdal_to_geos(feature.geometry()));
        }
    }

    {
        Log::debug("Create cell coverages");
        chrono::DurationRecorder rec;

        // sort on geometry complexity, so we always start processing the most complex geometries
        // this avoids processing the most complext geometry in the end on a single core
        std::sort(geometries.begin(), geometries.end(), [](const std::tuple<int64_t, double, geos::geom::Geometry::Ptr>& lhs, const std::tuple<int64_t, double, geos::geom::Geometry::Ptr>& rhs) {
            return std::get<2>(lhs)->getNumPoints() > std::get<2>(rhs)->getNumPoints();
        });

        // export to string and import in every loop instance, accessing the spatial reference
        // from multiple threads is not thread safe
        auto projection = gdal::SpatialReference(outputExtent.projection).export_to_wkt();

        std::mutex mut;
        ProgressInfo progress(geometries.size(), progressCb);
        std::for_each(std::execution::par, geometries.begin(), geometries.end(), [&](const std::tuple<int64_t, double, geos::geom::Geometry::Ptr>& idGeom) {
            auto cov  = create_polygon_coverage(std::get<0>(idGeom), *std::get<2>(idGeom), gdal::SpatialReference(projection), outputExtent);
            cov.value = std::get<1>(idGeom);
            progress.tick();

            std::scoped_lock lock(mut);
            result.push_back(std::move(cov));
        });

        Log::debug("Create cell coverages took: {}", rec.elapsed_time_string());
    }

    return result;
}
}
