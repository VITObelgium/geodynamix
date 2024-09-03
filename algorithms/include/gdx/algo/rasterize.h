#pragma once

#include "gdx/algo/polygoncoverage.h"
#include "gdx/rastermetadata.h"
#include "infra/cast.h"
#include "infra/filesystem.h"
#include "infra/gdalalgo.h"
#include "infra/progressinfo.h"

#include <variant>

namespace gdx {

template <typename T>
struct RasterizeOptions
{
    // the attribute field used to burn the values, or a fixed value
    std::variant<std::string, T> burnValue;

    bool add        = false;
    bool allTouched = false;
    inf::GeoMetadata meta;
    std::string inputLayer; // if empty, the first layer will be used

    std::vector<std::string> values;
};

template <typename T>
struct TranslateOptions
{
    inf::GeoMetadata meta;
    std::vector<std::string> values;
};

template <template <typename> typename RasterType, typename T>
RasterType<T> translate(const RasterType<T>& ras, const TranslateOptions<T>& options)
{
    auto& meta = ras.metadata();

    auto memDriver = inf::gdal::RasterDriver::create(inf::gdal::RasterType::Memory);
    inf::gdal::RasterDataSet memDataSet(memDriver.create_dataset<T>(meta.rows, meta.cols, 0));
    memDataSet.add_band(ras.data());
    memDataSet.set_geotransform(inf::metadata_to_geo_transform(meta));
    memDataSet.set_nodata_value(1, meta.nodata);
    memDataSet.set_projection(meta.projection);

    auto dataPair = inf::gdal::translate<T>(memDataSet, options.meta);
    return RasterType<T>(dataPair.first, dataPair.second);
}

template <typename RasterType>
RasterType rasterize(const inf::gdal::VectorDataSet& shapeDataSet, const RasterizeOptions<typename RasterType::value_type>& options)
{
    using T = typename RasterType::value_type;

    std::vector<std::string> gdalOpts;

    if (std::holds_alternative<std::string>(options.burnValue)) {
        gdalOpts.emplace_back("-a");
        gdalOpts.emplace_back(std::get<std::string>(options.burnValue));

    } else if (std::holds_alternative<T>(options.burnValue)) {
        gdalOpts.emplace_back("-burn");
        if constexpr (std::is_floating_point_v<T>) {
            gdalOpts.emplace_back(fmt::format("{:f}", std::get<T>(options.burnValue)));
        } else {
            gdalOpts.emplace_back(std::to_string(std::get<T>(options.burnValue)));
        }
    }

    if (!options.inputLayer.empty()) {
        gdalOpts.emplace_back("-l");
        gdalOpts.emplace_back(options.inputLayer);
    }

    if (options.allTouched) {
        gdalOpts.emplace_back("-at");
    }

    if (options.add) {
        gdalOpts.emplace_back("-add");
        if (options.meta.nodata.has_value() && std::isnan(*options.meta.nodata)) {
            throw inf::RuntimeError("Raster output nodata is nan, this is not compatible with the add algorithm");
        }
    }

    for (std::size_t i = 0; i < options.values.size(); ++i) {
        gdalOpts.push_back(options.values[i]);
    }

    auto datasetDataPair = inf::gdal::rasterize<T>(shapeDataSet, options.meta, gdalOpts);
    return RasterType(datasetDataPair.first, datasetDataPair.second);
}

template <typename RasterType>
RasterType rasterize(const fs::path& shapePath, const RasterizeOptions<typename RasterType::value_type>& options)
{
    return rasterize<RasterType>(inf::gdal::VectorDataSet::open(shapePath), options);
}

struct RasterizePolygonOptions
{
    // the attribute field used to burn the values, or a fixed value
    std::variant<std::string, double> burnValue;
    std::optional<double> initNodata;

    inf::GeoMetadata outputMeta;
    std::string inputLayer;      // if empty, the first layer will be used
    std::string attributeFilter; // if empty, no filtering is applied
    BorderHandling borderHandling = BorderHandling::None;
    inf::ProgressInfo::Callback progressCb;
};

template <typename RasterType>
RasterType rasterize_polygons(inf::gdal::VectorDataSet& vectorDs, const RasterizePolygonOptions& options)
{
    using T              = typename RasterType::value_type;
    const auto coverages = create_polygon_coverages(options.outputMeta, vectorDs, options.borderHandling, options.burnValue, options.attributeFilter, options.inputLayer, "", options.progressCb);

    RasterType result(options.outputMeta, inf::truncate<T>(options.outputMeta.nodata.value_or(0.0)));
    for (const auto& polygonCov : coverages) {
        for (const auto& polygonCell : polygonCov.cells) {
            if (auto cell = polygonCell.computeGridCell; options.outputMeta.is_on_map(cell)) {
                if (result.is_nodata(cell)) {
                    result[cell] = inf::truncate<T>(options.initNodata.value_or(0.0));
                    result.mark_as_data(cell);
                }

                result[cell] += inf::truncate<T>(polygonCell.coverage * polygonCov.value);
            }
        }
    }

    return result;
}

template <typename RasterType>
RasterType rasterize_polygons(const fs::path& vector, const RasterizePolygonOptions& options)
{
    auto ds = inf::gdal::VectorDataSet::open(vector);
    return rasterize_polygons<RasterType>(ds, options);
}

}
