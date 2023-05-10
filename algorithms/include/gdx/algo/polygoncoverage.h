#pragma once

#include "infra/cast.h"
#include "infra/filesystem.h"
#include "infra/gdalalgo.h"
#include "infra/geometadata.h"
#include "infra/math.h"
#include "infra/progressinfo.h"

#include <variant>

namespace gdx {

namespace gdal = inf::gdal;

struct PolygonCellCoverage
{
    struct CellInfo
    {
        CellInfo() noexcept = default;
        CellInfo(inf::Cell compute, inf::Cell polygon, double cov, double cellCov)
        : computeGridCell(compute)
        , polygonGridCell(polygon)
        , coverage(cov)
        , cellCoverage(cellCov)
        {
        }

        inf::Cell computeGridCell; // row column index of this cell in the full output grid
        inf::Cell polygonGridCell; // row column index of this cell in the polygon sub grid of the spatial pattern grid

        double coverage     = 0.0; // The percentage of the polygon surface in this cell
        double cellCoverage = 0.0; // The percentage of this cell covered by the polygon

        constexpr bool operator==(const CellInfo& other) const noexcept
        {
            // Don't compare the polygon cell, we want to compare cells from different polygons in the compute grid
            return computeGridCell == other.computeGridCell && coverage == other.coverage;
        }
    };

    int64_t id = 0;
    std::string name;
    double value = 0.0;
    inf::GeoMetadata outputSubgridExtent; // This polygon subgrid within the output grid
    std::vector<CellInfo> cells;
};

std::vector<PolygonCellCoverage> create_polygon_coverages(const inf::GeoMetadata& outputExtent,
                                                          gdal::VectorDataSet& vectorDs,
                                                          std::variant<std::string, double> burnValue,
                                                          const std::string& attributeFilter,
                                                          const std::string& inputLayer,
                                                          const std::string& nameField,
                                                          const inf::ProgressInfo::Callback& progressCb);

/* Cut the polygon out of the grid, using the cellcoverage info, the extent of ras has to be the country subextent */
template <typename TResult, template <typename> typename RasterType, typename T>
RasterType<TResult> cutout_polygon(const RasterType<T>& ras, const PolygonCellCoverage& polygonCoverage)
{
    static_assert(std::is_floating_point_v<TResult>);

    constexpr auto nan = inf::math::nan<TResult>();
    RasterType<TResult> result(copy_metadata_replace_nodata(ras.metadata(), nan), nan);

    for (const auto& cellInfo : polygonCoverage.cells) {
        const auto& polygonCell = cellInfo.polygonGridCell;
        assert(ras.metadata().is_on_map(polygonCell));
        if (!ras.metadata().is_on_map(polygonCell)) {
            continue;
        }

        if (ras.is_nodata(polygonCell)) {
            continue;
        }

        result[polygonCell] = static_cast<TResult>(ras[polygonCell] * cellInfo.cellCoverage);
    }

    return result;
}

}
