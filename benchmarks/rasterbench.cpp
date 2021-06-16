#include "denserasternonsimd.h"
#include "gdx/algo/sum.h"
#include "gdx/denseraster.h"
#include "gdx/maskedraster.h"
#include "gdx/rasteriterator.h"

#include <Eigen/Core>

#include <benchmark/benchmark.h>
#include <iostream>
#include <numeric>
#include <stdexcept>

using namespace gdx;

template <typename T>
using EigenArray = Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

template <typename T>
EigenArray<T> make_eigen_array(int32_t rows, int32_t cols, T fillValue)
{
    EigenArray<T> arr(rows, cols);
    arr.fill(fillValue);
    return arr;
}

template <typename Raster>
static void add_2_rasters(benchmark::State& state)
{
    using T  = typename Raster::value_type;
    auto dim = inf::truncate<int32_t>(state.range(0));
    Raster ras1(RasterMetadata(dim, dim, -1.0), T(1));
    Raster ras2(RasterMetadata(dim, dim, -1.0), T(2));
    for (auto _ : state) {
        benchmark::DoNotOptimize(ras1 + ras2);
    }
}

static void add_2_rasters_eigen(benchmark::State& state)
{
    auto dim  = inf::truncate<int32_t>(state.range(0));
    auto ras1 = make_eigen_array<int32_t>(dim, dim, 1);
    auto ras2 = make_eigen_array<int32_t>(dim, dim, 2);
    for (auto _ : state) {
        // Assign to an eigen array to evaluate the expression template
        EigenArray<int> res = ras1 + ras2;
        (void)res;
    }
}

template <typename Raster>
static void add_2_rasters_inplace(benchmark::State& state)
{
    using T  = typename Raster::value_type;
    auto dim = inf::truncate<int32_t>(state.range(0));
    Raster ras1(RasterMetadata(dim, dim, -1.0), T(1));
    Raster ras2(RasterMetadata(dim, dim, -1.0), T(2));
    for (auto _ : state) {
        benchmark::DoNotOptimize(ras1 += ras2);
    }
}

template <typename T>
static void add_2_rasters_inplace_eigen(benchmark::State& state)
{
    auto dim  = inf::truncate<int32_t>(state.range(0));
    auto ras1 = make_eigen_array<T>(dim, dim, T(1));
    auto ras2 = make_eigen_array<T>(dim, dim, T(2));
    for (auto _ : state) {
        ras1 += ras2;
    }
}

static void rasterAddScalarInt(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    DenseRaster<int> ras(RasterMetadata(dim, dim, -1.0), 1);
    for (auto _ : state) {
        auto res = ras + 5;
        if (res[0] != 6) {
            throw std::runtime_error("");
        }
    }
}

static void rasterAddScalarFloat(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    DenseRaster<float> ras(RasterMetadata(dim, dim, -1.0), 1.f);
    for (auto _ : state) {
        auto res = ras + 5.f;
        if (res[0] != 6.f) {
            throw std::runtime_error("");
        }
    }
}

static void rasterAddScalarFloatEigen(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    auto ras = make_eigen_array<float>(dim, dim, 1.f);
    for (auto _ : state) {
        EigenArray<float> res = ras + 5.f;
        (void)res;
    }
}

static void rasterMultiplyInPlaceInt(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    DenseRaster<int> ras(RasterMetadata(dim, dim, -1.0), 1);
    for (auto _ : state) {
        ras *= 5;
    }
}

static void rasterMultiplyInPlaceFloat(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    DenseRaster<float> ras(RasterMetadata(dim, dim, -1.0), 1);
    for (auto _ : state) {
        ras *= 5.f;
    }
}

static void rasterAddInPlaceInt(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    DenseRaster<int> ras(RasterMetadata(dim, dim, -1.0), 1);
    for (auto _ : state) {
        ras += 5;
    }
}

static void rasterAddInPlaceFloat(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    DenseRaster<float> ras(RasterMetadata(dim, dim, -1.0), 1);
    for (auto _ : state) {
        ras += 5.f;
    }
}

static void rasterAddInPlaceFloatEigen(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    auto ras = make_eigen_array<float>(dim, dim, 1.f);
    for (auto _ : state) {
        ras += 5.f;
    }
}

static void rasterSubtractInPlaceInt(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    DenseRaster<int> ras(RasterMetadata(dim, dim, -1.0), 1);
    for (auto _ : state) {
        ras -= 5;
    }
}

static void rasterSubtractInPlaceFloat(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    DenseRaster<float> ras(RasterMetadata(dim, dim, -1.0), 1);
    for (auto _ : state) {
        ras -= 5.f;
    }
}

static void rasterDivideInPlaceInt(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    DenseRaster<int> ras(RasterMetadata(dim, dim, -1.0), 1);
    for (auto _ : state) {
        ras /= 5;
    }
}

static void rasterDivideInPlaceFloat(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    DenseRaster<float> ras(RasterMetadata(dim, dim, -1.0), 1);
    for (auto _ : state) {
        ras /= 5.f;
    }
}

template <typename Raster>
static void fill_raster(benchmark::State& state)
{
    using T = typename Raster::value_type;

    auto dim = inf::truncate<int32_t>(state.range(0));
    Raster ras(RasterMetadata(dim, dim, -1.0), T(1));
    for (auto _ : state) {
        ras.fill(T(5));
    }
}

template <typename Raster>
static void fill_raster_values(benchmark::State& state)
{
    using T = typename Raster::value_type;

    auto dim = inf::truncate<int32_t>(state.range(0));
    Raster ras(RasterMetadata(dim, dim, -1.0), T(1));
    for (auto _ : state) {
        ras.fill_values(T(5));
    }
}

static void initFloatRasterWithData(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    std::vector<float> v(size_t(dim * dim));
    std::iota(v.begin(), v.end(), -10.f);

    for (auto _ : state) {
        DenseRaster<float> ras(RasterMetadata(dim, dim, -1.0), v);
    }
}

#ifndef _MSC_VER
BENCHMARK_TEMPLATE(add_2_rasters, DenseRaster<uint8_t>)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(add_2_rasters, nosimd::DenseRaster<int32_t>)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(add_2_rasters, DenseRaster<int32_t>)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(add_2_rasters, nosimd::DenseRaster<int32_t>)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(add_2_rasters, DenseRaster<int32_t>)->Arg(10)->Arg(100);
#endif
BENCHMARK(add_2_rasters_eigen)->Arg(10)->Arg(100);

BENCHMARK_TEMPLATE(add_2_rasters_inplace, nosimd::DenseRaster<int32_t>)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(add_2_rasters_inplace, DenseRaster<int32_t>)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(add_2_rasters_inplace, nosimd::DenseRaster<float>)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(add_2_rasters_inplace, DenseRaster<float>)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(add_2_rasters_inplace_eigen, int32_t)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(add_2_rasters_inplace_eigen, float)->Arg(10)->Arg(100);

BENCHMARK(rasterAddScalarInt)->Arg(10)->Arg(100);
BENCHMARK(rasterAddScalarFloatEigen)->Arg(10)->Arg(100);
BENCHMARK(rasterAddScalarFloat)->Arg(10)->Arg(100);
BENCHMARK(rasterMultiplyInPlaceInt)->Arg(10)->Arg(100);
BENCHMARK(rasterMultiplyInPlaceFloat)->Arg(10)->Arg(100);
BENCHMARK(rasterAddInPlaceInt)->Arg(10)->Arg(100);
BENCHMARK(rasterAddInPlaceFloat)->Arg(10)->Arg(100);
BENCHMARK(rasterAddInPlaceFloatEigen)->Arg(10)->Arg(100);
BENCHMARK(rasterSubtractInPlaceInt)->Arg(10)->Arg(100);
BENCHMARK(rasterSubtractInPlaceFloat)->Arg(10)->Arg(100);
BENCHMARK(rasterDivideInPlaceInt)->Arg(10)->Arg(100);
BENCHMARK(rasterDivideInPlaceFloat)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(fill_raster, DenseRaster<int32_t>)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(fill_raster, nosimd::DenseRaster<int32_t>)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(fill_raster, DenseRaster<float>)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(fill_raster, nosimd::DenseRaster<float>)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(fill_raster_values, DenseRaster<int32_t>)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(fill_raster_values, nosimd::DenseRaster<int32_t>)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(fill_raster_values, DenseRaster<float>)->Arg(10)->Arg(100);
BENCHMARK_TEMPLATE(fill_raster_values, nosimd::DenseRaster<float>)->Arg(10)->Arg(100);
BENCHMARK(initFloatRasterWithData)->Arg(10)->Arg(100);

BENCHMARK_MAIN();
