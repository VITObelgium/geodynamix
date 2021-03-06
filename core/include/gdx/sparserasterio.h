#pragma once

#include "gdx/sparseraster.h"
#include "infra/filesystem.h"
#include "infra/gdalio.h"

namespace gdx {

template <typename T>
SparseRaster<T> read_sparse_raster(const fs::path& filename)
{
    auto dataSet = inf::gdal::RasterDataSet::open(filename);
    std::vector<T> denseData(dataSet.x_size() * dataSet.y_size() * sizeof(T));
    auto meta = inf::gdal::io::read_raster_data<T>(dataSet, denseData);
    return SparseRaster<T>(meta, denseData);
}

template <typename T>
SparseRaster<T> read_sparse_raster(const fs::path& filename, const RasterMetadata& extent)
{
    auto dataSet = inf::gdal::RasterDataSet::open(filename);
    std::vector<T> denseData(dataSet.x_size() * dataSet.y_size() * sizeof(T));
    auto meta = inf::gdal::io::read_raster_data<T>(dataSet, extent, denseData);
    return SparseRaster<T>(meta, denseData);
}

template <typename T>
void read_raster(const fs::path& filename, SparseRaster<T>& raster)
{
    auto dataSet = inf::gdal::RasterDataSet::open(filename);
    std::vector<T> denseData(dataSet.x_size() * dataSet.y_size() * sizeof(T));
    auto meta = inf::gdal::io::read_raster_data<T>(dataSet, denseData);
    raster    = SparseRaster<T>(meta, denseData);
}

template <typename RasterDataType>
void write_raster(const SparseRaster<RasterDataType>& raster, const fs::path& filename)
{
    inf::gdal::io::write_raster(raster, raster.metadata(), filename);
}

template <typename RasterDataType>
SparseRaster<RasterDataType> warp_raster(const SparseRaster<RasterDataType>& raster, int32_t destCrs)
{
    SparseRaster<RasterDataType> result;
    result.set_metadata(inf::gdal::io::warp_raster(raster, raster.metadata(), destCrs, result));
    return result;
}
}
