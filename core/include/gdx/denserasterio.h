#pragma once

#include "gdx/denseraster.h"
#include "infra/cast.h"
#include "infra/filesystem.h"
#include "infra/gdalalgo.h"
#include "infra/gdalio.h"

#include <variant>

namespace gdx {

using DenseRasterVariant = std::variant<
    DenseRaster<uint8_t>,
    DenseRaster<uint16_t>,
    DenseRaster<uint32_t>,
    DenseRaster<int16_t>,
    DenseRaster<int32_t>,
    DenseRaster<float>,
    DenseRaster<double>>;

template <typename T>
DenseRaster<T> create_dense_raster_filled_with_nodata(const RasterMetadata& meta)
{
    return DenseRaster<T>(meta, static_cast<T>(meta.nodata.value_or(0.0)));
}

template <typename T>
DenseRaster<T> create_dense_raster_filled_with_zeros(const RasterMetadata& meta)
{
    return DenseRaster<T>(meta, 0);
}

template <typename T>
DenseRaster<T> read_dense_raster_from_dataset_band(inf::gdal::RasterDataSet& ds, int bandNr)
{
    assert(bandNr > 0);
    DenseRaster<T> raster(ds.y_size(), ds.x_size());
    raster.set_metadata(inf::gdal::io::read_raster_data<T>(ds, bandNr, raster));
    raster.init_nodata_values();
    return raster;
}

template <typename T>
DenseRaster<T> read_dense_raster(const fs::path& filename)
{
    auto dataSet = inf::gdal::RasterDataSet::open(filename);
    return read_dense_raster_from_dataset_band<T>(dataSet, 1);
}

template <typename T>
DenseRaster<T> read_dense_raster(const fs::path& filename, int bandNr)
{
    auto dataSet = inf::gdal::RasterDataSet::open(filename);
    return read_dense_raster_from_dataset_band<T>(dataSet, bandNr);
}

inline DenseRasterVariant read_dense_raster(const fs::path& filename)
{
    auto& type = inf::gdal::io::get_raster_type(filename);
    if (type == typeid(uint8_t)) return read_dense_raster<uint8_t>(filename);
    if (type == typeid(int16_t)) return read_dense_raster<int16_t>(filename);
    if (type == typeid(uint16_t)) return read_dense_raster<uint16_t>(filename);
    if (type == typeid(int32_t)) return read_dense_raster<int32_t>(filename);
    if (type == typeid(uint32_t)) return read_dense_raster<uint32_t>(filename);
    if (type == typeid(float)) return read_dense_raster<float>(filename);
    if (type == typeid(double)) return read_dense_raster<double>(filename);

    throw RuntimeError("Unsupported raster data type");
}

template <typename T>
DenseRaster<T> read_dense_raster(const fs::path& filename, const RasterMetadata& extent)
{
    auto dataSet = inf::gdal::RasterDataSet::open(filename);
    DenseRaster<T> raster(extent);
    raster.set_metadata(inf::gdal::io::read_raster_data<T>(dataSet, extent, raster));
    raster.init_nodata_values();
    return raster;
}

template <typename T>
const RasterMetadata& read_raster(const fs::path& filename, DenseRaster<T>& raster)
{
    raster = read_dense_raster<T>(filename);
    return raster.metadata();
}

template <typename T>
const RasterMetadata& read_raster(const fs::path& filename, const RasterMetadata& extent, DenseRaster<T>& raster)
{
    raster = read_dense_raster<T>(filename, extent);
    return raster.metadata();
}

template <typename T>
void write_raster(const DenseRaster<T>& raster, const fs::path& filename, std::span<const std::string> driverOptions = {})
{
    auto copy = raster.copy();
    copy.collapse_data();
    inf::gdal::io::write_raster(std::span<const T>(copy), copy.metadata(), filename, driverOptions);
}

template <typename T>
void write_raster_to_dataset_band(DenseRaster<T>&& raster, inf::gdal::RasterDataSet& ds, int bandNr)
{
    raster.collapse_data();
    inf::gdal::io::write_raster(std::span<const T>(raster), raster.metadata(), ds, bandNr);
}

template <typename T>
void write_raster_to_dataset_band(const DenseRaster<T>& raster, inf::gdal::RasterDataSet& ds, int bandNr)
{
    return write_raster_to_dataset_band(raster.copy(), ds, bandNr);
}

template <typename T>
void write_raster(DenseRaster<T>&& raster, const fs::path& filename, std::span<const std::string> driverOptions = {})
{
    raster.collapse_data();
    inf::gdal::io::write_raster(std::span<const T>(raster), raster.metadata(), filename, driverOptions);
}

template <typename T>
DenseRaster<T> warp_raster(const DenseRaster<T>& raster, int32_t destCrs, inf::gdal::ResampleAlgorithm algo = inf::gdal::ResampleAlgorithm::NearestNeighbour)
{
    auto srcMeta = raster.metadata();

    if (DenseRaster<T>::raster_type_has_nan) {
        // Set set source nodata to nan to avoid having to collapse the data
        // since the nodata values for floats are nan and not the actual nodata value
        srcMeta.nodata = DenseRaster<T>::NaN;
    }

    auto destMeta = inf::gdal::warp_metadata(srcMeta, destCrs);

    DenseRaster<T> result(destMeta, inf::truncate<T>(*destMeta.nodata));
    inf::gdal::io::warp_raster<T, T>(raster, srcMeta, result, result.metadata(), algo);
    return result;
}

template <typename T>
DenseRaster<T> resample_raster(const DenseRaster<T>& raster, const RasterMetadata& meta, inf::gdal::ResampleAlgorithm algo)
{
    const auto& srcMeta = raster.metadata();

    auto destMeta   = meta;
    destMeta.nodata = srcMeta.nodata;
    if (!destMeta.projected_epsg().has_value()) {
        if (auto epsg = srcMeta.projected_epsg(); epsg.has_value()) {
            destMeta.set_projection_from_epsg(*epsg);
        }
    }

    DenseRaster<T> result(destMeta, inf::truncate<T>(destMeta.nodata.value_or(0.0)));
    inf::gdal::io::warp_raster<T, T>(raster, raster.metadata(), result, result.metadata(), algo);
    result.init_nodata_values();
    return result;
}

}
