#pragma once

#include "gdx/exception.h"

#include <vector>

namespace gdx {

template <typename T>
void zero2D(const int dim1, const int dim2, std::vector<std::vector<T>>& x)
{
    x.resize(dim1);
    for (int i = 0; i < dim1; ++i) {
        x[i].assign(dim2, static_cast<T>(0));
    }
}

template <template <typename> typename RasterType, typename IntType, typename FloatType>
std::vector<std::vector<double>> computeGkz(const RasterType<IntType>& klasse_map,
    const std::vector<FloatType>& Ck, // weight per class.  Set to 1 to have zero effect
    const RasterType<IntType>& zone_map,
    const int zs) // number of zones, numbered 0..(zs-1)
{
    std::vector<std::vector<double>> gkz;

    const int ks = int(Ck.size());
    std::vector<std::vector<int>> Akz;
    zero2D<int>(ks, zs, Akz);
    std::vector<int> Ak;
    Ak.assign(ks, 0);
    std::vector<int> Az;
    Az.assign(zs, 0);
    size_t size = klasse_map.size();
    if (size != zone_map.size()) {
        throw InvalidArgument("class map and zone map should have the same size in dasymatrix mapping");
    }
    for (size_t i = 0; i < size; ++i) {
        if (!klasse_map.is_nodata(i) && !zone_map.is_nodata(i)) {
            const int k = int(klasse_map[i]);
            const int z = int(zone_map[i]);
            if (k < 0 || k >= ks) {
                throw InvalidArgument("class map value out of range of Ck parameter in dasymatrix mapping");
            }
            ++Ak[k];
            if (z < 0 || z >= zs) {
                throw InvalidArgument("zone map value out of range of Gz parameter in dasymatrix mapping");
            }
            ++Az[z];
            ++Akz[k][z];
        }
    }
    int A = 0;
    for (int z = 0; z < zs; ++z) {
        A += Az[z];
    }
    std::vector<std::vector<double>> ckz;
    zero2D<double>(ks, zs, ckz);
    for (int k = 0; k < ks; ++k) {
        for (int z = 0; z < zs; z++) {
            if (!std::isnan(Ck[k])) {
                ckz[k][z] = (Az[z] > 0 && A > 0 && Ak[k] > 0 ? Ck[k] * (Akz[k][z] / double(Az[z])) / (Ak[k] / double(A)) : 0);
            }
        }
    }
    std::vector<double> cz;
    cz.assign(zs, 0);
    for (int z = 0; z < zs; z++) {
        for (int k = 0; k < ks; ++k) {
            cz[z] += ckz[k][z];
        }
    }
    zero2D<double>(ks, zs, gkz);
    for (int z = 0; z < zs; z++) {
        if (cz[z] > 0) {
            for (int k = 0; k < ks; ++k) {
                gkz[k][z] = (Akz[k][z] > 0 ? (ckz[k][z] / cz[z]) / Akz[k][z] : 0);
            }
        } else {
            for (int k = 0; k < ks; ++k) {
                gkz[k][z] = (Az[z] > 0 ? 1.0 / double(Az[z]) : 0);
            }
        }
    }

    return gkz;
}

/*! Dasymetric mapping.
 *  Returns a gdx raster with the result.
 *  Only floating point result rasters make sense.
 */

template <typename ResultType, template <typename> typename RasterType, typename IntType, typename FloatType>
RasterType<ResultType> dasMap(
    const RasterType<IntType>& landuse_map, // landuses are identified in the code with variable 'k'
    const std::vector<FloatType>& Ck,       // [k] : landuse weights.  Set all to 1 to have zero effect.
    const RasterType<IntType>& zone_map,    // zones are identified in the code with variable 'z'
    const std::vector<FloatType>& amounts)  // [z] : the amount that has to be mapped per zone
{
    static_assert(std::is_floating_point_v<ResultType>, "dasMap: only makes sense with floating point result rasters");

    if (landuse_map.metadata().rows != zone_map.metadata().rows || landuse_map.metadata().cols != zone_map.metadata().cols) {
        throw InvalidArgument("dasMap: landuse map and zone map should have equal extent");
    }

    auto meta   = zone_map.metadata();
    meta.nodata = std::numeric_limits<double>::quiet_NaN();

    RasterType<ResultType> result(meta, ResultType(meta.nodata.value()));

    auto gkz          = computeGkz(landuse_map, Ck, zone_map, int(amounts.size()));
    const size_t size = result.size();
    for (size_t i = 0; i < size; ++i) {
        if (!landuse_map.is_nodata(i) && !zone_map.is_nodata(i)) {
            const auto z = zone_map[i];
            const auto k = landuse_map[i];
            if (!std::isnan(amounts[z])) {
                result.mark_as_data(i);
                result[i] = static_cast<ResultType>(amounts[z] * gkz[k][z]);
            }
        }
    }

    return result;
}
}
