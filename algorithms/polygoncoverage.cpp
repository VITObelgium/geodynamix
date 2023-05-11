#include "gdx/algo/polygoncoverage.h"
#include "gdx/rasteriterator.h"

#include "infra/chrono.h"
#include "infra/geometry.h"
#include "infra/log.h"

#include <algorithm>

#ifdef GDX_HAVE_GEOS
#include <geos/geom/Geometry.h>
#include <geos/geom/prep/PreparedGeometryFactory.h>
#endif

#ifdef GDX_HAVE_PAR_EXECUTION
#include <execution>
#define PAR_POLICY std::execution::par,
#else
#define PAR_POLICY
#endif

namespace gdx {

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

static std::vector<PolygonCellCoverage::CellInfo> create_cell_coverages(const GeoMetadata& extent, const GeoMetadata& polygonExtent, const geos::geom::Geometry& geom)
{
    std::vector<PolygonCellCoverage::CellInfo> result;

    auto preparedGeom   = geos::geom::prep::PreparedGeometryFactory::prepare(&geom);
    const auto cellSize = extent.cellSize;
    const auto cellArea = std::abs(cellSize.x * cellSize.y);

    auto polygonArea = preparedGeom->getGeometry().getArea();

    for (auto polygonCell : gdx::RasterCells(polygonExtent.rows, polygonExtent.cols)) {
        const Rect<double> box = polygonExtent.bounding_box(polygonCell);
        const auto cellGeom    = geom::create_polygon(box.topLeft, box.bottomRight);

        // Intersect it with the polygon
        auto xyCentre   = polygonExtent.convert_cell_centre_to_xy(polygonCell);
        auto outputCell = extent.convert_point_to_cell(xyCentre);

        if (preparedGeom->within(cellGeom.get())) {
            result.emplace_back(outputCell, polygonCell, 1.0, polygonArea / cellArea);
        } else if (preparedGeom->containsProperly(cellGeom.get())) {
            result.emplace_back(outputCell, polygonCell, cellArea / polygonArea, 1.0);
        } else if (preparedGeom->intersects(cellGeom.get())) {
            auto intersectGeometry   = preparedGeom->getGeometry().intersection(cellGeom.get());
            const auto intersectArea = intersectGeometry->getArea();
            if (intersectArea > 0) {
                result.emplace_back(outputCell, polygonCell, intersectArea / polygonArea, intersectArea / cellArea);
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

std::vector<gdx::PolygonCellCoverage> process_region_borders(const std::vector<gdx::PolygonCellCoverage>& cellCoverages)
{
    std::vector<gdx::PolygonCellCoverage> result;
    result.reserve(cellCoverages.size());

    for (auto& cov : cellCoverages) {
        auto& region       = cov.name;
        auto& outputExtent = cov.outputSubgridExtent;
        auto& cells        = cov.cells;

        std::vector<gdx::PolygonCellCoverage::CellInfo> modifiedCoverages;
        modifiedCoverages.reserve(cells.size());

        for (auto& cell : cells) {
            auto modifiedCoverage = cell;

            if (cell.coverage < 1.0 && cell.coverage > 0.0) {
                // polygon border, check if there are other polygons in this cell
                double otherPolygonCoverages = 0;

                for (const auto& testCov : cellCoverages) {
                    if (testCov.name == region) {
                        continue;
                    }

                    // Locate the current cell in the coverage of the other polygon
                    auto cellIter = std::lower_bound(testCov.cells.begin(), testCov.cells.end(), cell.computeGridCell, [](const gdx::PolygonCellCoverage::CellInfo& cov, Cell c) {
                        return cov.computeGridCell < c;
                    });

                    if (cellIter != testCov.cells.end() && cellIter->computeGridCell == cell.computeGridCell) {
                        // the other polygon covers the cell
                        otherPolygonCoverages += cellIter->coverage;
                    }
                }

                if (otherPolygonCoverages == 0.0) {
                    // This is the only polygon in the cell, so we get all the coverage
                    modifiedCoverage.coverage = 1.0;
                } else {
                    modifiedCoverage.coverage = cell.coverage / (cell.coverage + otherPolygonCoverages);
                }
            }

            modifiedCoverages.push_back(modifiedCoverage);
        }

        gdx::PolygonCellCoverage cov;
        cov.name                = region;
        cov.cells               = std::move(modifiedCoverages);
        cov.outputSubgridExtent = outputExtent;
        result.push_back(std::move(cov));
    }

    return result;
}

std::vector<PolygonCellCoverage> create_polygon_coverages(const inf::GeoMetadata& outputExtent,
                                                          gdal::VectorDataSet& vectorDs,
                                                          BorderHandling borderHandling,
                                                          std::variant<std::string, double> burnValue,
                                                          const std::string& attributeFilter,
                                                          const std::string& inputLayer,
                                                          const std::string& nameField,
                                                          const ProgressInfo::Callback& progressCb)
{
    std::vector<PolygonCellCoverage> result;
    std::vector<std::tuple<int64_t, double, std::string, geos::geom::Geometry::Ptr>> geometries;

    for (int32_t i = 0; i < vectorDs.layer_count(); ++i) {
        auto layer = vectorDs.layer(i);
        if (!inputLayer.empty() && layer.name() != inputLayer) {
            continue;
        }

        int32_t valueColumn = -1;
        if (std::holds_alternative<std::string>(burnValue)) {
            valueColumn = layer.layer_definition().required_field_index(std::get<std::string>(burnValue));
        }

        int32_t nameColumn = -1;
        if (!nameField.empty()) {
            nameColumn = layer.layer_definition().required_field_index(nameField);
        }

        assert(!outputExtent.projection.empty());
        if (!layer.projection().has_value()) {
            throw RuntimeError("Invalid input vector: No projection information available");
        }

        if (outputExtent.projected_epsg() != layer.projection()->epsg_cs()) {
            throw RuntimeError("Projection mismatch between input vector and metadata grid EPSG:{} <-> EPSG:{}", outputExtent.projected_epsg().value(), layer.projection()->epsg_cs().value());
        }

        const auto bbox = outputExtent.bounding_box();
        layer.set_spatial_filter(bbox.topLeft, bbox.bottomRight);
        if (!attributeFilter.empty()) {
            layer.set_attribute_filter(attributeFilter);
        }

        for (auto& feature : layer) {
            double value     = valueColumn == -1 ? std::get<double>(burnValue) : feature.field_as<double>(valueColumn);
            std::string name = nameColumn == -1 ? std::string() : feature.field_as<std::string>(nameColumn);
            geometries.emplace_back(feature.id(), value, name, geom::gdal_to_geos(feature.geometry()));
        }
    }

    {
        Log::debug("Create cell coverages");
        chrono::DurationRecorder rec;

        // sort on geometry complexity, so we always start processing the most complex geometries
        // this avoids processing the most complext geometry in the end on a single core
        std::sort(geometries.begin(), geometries.end(), [](const std::tuple<int64_t, double, std::string, geos::geom::Geometry::Ptr>& lhs, const std::tuple<int64_t, double, std::string, geos::geom::Geometry::Ptr>& rhs) {
            return std::get<3>(lhs)->getNumPoints() > std::get<3>(rhs)->getNumPoints();
        });

        // export to string and import in every loop instance, accessing the spatial reference
        // from multiple threads is not thread safe
        auto projection = gdal::SpatialReference(outputExtent.projection).export_to_wkt();

        std::mutex mut;
        ProgressInfo progress(geometries.size(), progressCb);
        std::for_each(PAR_POLICY geometries.begin(), geometries.end(), [&](const std::tuple<int64_t, double, std::string, geos::geom::Geometry::Ptr>& idGeom) {
            auto cov  = create_polygon_coverage(std::get<0>(idGeom), *std::get<3>(idGeom), gdal::SpatialReference(projection), outputExtent);
            cov.value = std::get<1>(idGeom);
            cov.name  = std::get<2>(idGeom);
            progress.tick();

            std::scoped_lock lock(mut);
            result.push_back(std::move(cov));
        });

        Log::debug("Create cell coverages took: {}", rec.elapsed_time_string());

        if (borderHandling == BorderHandling::AdjustCoverage) {
            // Update the coverages on the polygon borders to get appropriate coverage values at the edges
            // E.g a cell on the border that is only covered by 1 polygon for 50% should be modified to 100%
            // Because the data is only for inside the region
            result = process_region_borders(result);
        }
    }

    return result;
}

}
