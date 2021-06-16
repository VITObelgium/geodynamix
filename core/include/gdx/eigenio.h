#pragma once

#include "gdx/eigeniterationsupport-private.h"
#include "gdx/rastermetadata.h"
#include "infra/filesystem.h"
#include "infra/gdalio.h"

#include <Eigen/Core>
#include <optional>

namespace gdx {

template <typename T>
RasterMetadata read_raster(const fs::path& filename, Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>& raster)
{
    auto dataSet = inf::gdal::RasterDataSet::open(filename);
    raster.resize(dataSet.y_size(), dataSet.x_size());
    return inf::gdal::io::read_raster_data(dataSet, raster, std::span<T>(raster.data(), raster.size()));
}

template <typename T>
RasterMetadata read_raster(const fs::path& filename, const RasterMetadata& extent, Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>& raster)
{
    auto dataSet = inf::gdal::RasterDataSet::open(filename);
    raster.resize(extent.rows, extent.cols);
    return inf::gdal::io::read_raster_data(dataSet, extent, std::span<T>(raster.data(), raster.size()));
}
}
