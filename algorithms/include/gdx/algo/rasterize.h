#pragma once

#include "gdx/rastermetadata.h"
#include "infra/filesystem.h"
#include "infra/gdalalgo.h"

namespace gdx {

namespace gdal = inf::gdal;

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
        gdalOpts.emplace_back(std::to_string(std::get<T>(options.burnValue)));
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
    }

    for (std::size_t i = 0; i < options.values.size(); ++i) {
        gdalOpts.push_back(options.values[i]);
    }

    auto datasetDataPair = gdal::rasterize<T>(shapeDataSet, options.meta, gdalOpts);
    return RasterType(datasetDataPair.first, datasetDataPair.second);
}

template <typename RasterType>
RasterType rasterize(const fs::path& shapePath, const RasterizeOptions<typename RasterType::value_type>& options)
{
    return rasterize<RasterType>(gdal::VectorDataSet::open(shapePath), options);
}

template <template <typename> typename RasterType, typename T>
RasterType<T> translate(const RasterType<T>& ras, const TranslateOptions<T>& options)
{
    auto& meta = ras.metadata();

    auto memDriver = gdal::RasterDriver::create(gdal::RasterType::Memory);
    gdal::RasterDataSet memDataSet(memDriver.create_dataset<T>(meta.rows, meta.cols, 0));
    memDataSet.add_band(ras.data());
    memDataSet.set_geotransform(inf::metadata_to_geo_transform(meta));
    memDataSet.set_nodata_value(1, meta.nodata);
    memDataSet.set_projection(meta.projection);

    auto dataPair = gdal::translate<T>(memDataSet, options.meta);
    return RasterType<T>(dataPair.first, dataPair.second);
}
}
