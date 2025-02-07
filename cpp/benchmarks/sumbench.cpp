#include "gdx/algo/sum.h"
#include "gdx/denseraster.h"
#include "gdx/maskedraster.h"

#include <benchmark/benchmark.h>
#include <iostream>
#include <numeric>

using namespace gdx;

static void algoSumMaskedRaster(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    MaskedRaster<float> ras(RasterMetadata(dim, dim, -1.0), 1.f);
    double sum = 0.0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(sum = gdx::sum(ras));
    }
}

static void algoSumDenseRaster(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    DenseRaster<float> ras(RasterMetadata(dim, dim, -1.0), 1.f);
    double sum = 0.0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(sum = gdx::sum(ras));
    }
}

static void simdSumDenseRasterInt(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    DenseRaster<int> ras(RasterMetadata(dim, dim, -1.0), 1);
    int sum = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(sum = ras.sum());
    }
}

static void simdSumDenseRasterIntOnlyData(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    DenseRaster<int> ras(RasterMetadata(dim, dim), 1);
    int sum = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(sum = ras.sum());
    }
}

static void simdSumDenseRasterFloat(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    DenseRaster<float> ras(RasterMetadata(dim, dim, -1.0), 1.f);
    double sum = 0.0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(sum = ras.sum<double>());
    }
}

static void simdSumDenseRasterFloatOnlyData(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    DenseRaster<float> ras(RasterMetadata(dim, dim), 1.f);
    double sum = 0.0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(sum = ras.sum<double>());
    }
}

static void simdSumDenseRasterDouble(benchmark::State& state)
{
    auto dim = inf::truncate<int32_t>(state.range(0));
    DenseRaster<double> ras(RasterMetadata(dim, dim, -1.0), 1.f);
    double sum = 0.0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(sum = ras.sum());
    }
}

BENCHMARK(algoSumMaskedRaster)->Arg(10)->Arg(1000);
BENCHMARK(algoSumDenseRaster)->Arg(10)->Arg(1000);
BENCHMARK(simdSumDenseRasterInt)->Arg(10)->Arg(1000);
BENCHMARK(simdSumDenseRasterIntOnlyData)->Arg(10)->Arg(1000);
BENCHMARK(simdSumDenseRasterFloat)->Arg(10)->Arg(1000);
BENCHMARK(simdSumDenseRasterFloatOnlyData)->Arg(10)->Arg(1000);
BENCHMARK(simdSumDenseRasterDouble)->Arg(10)->Arg(1000);

BENCHMARK_MAIN();
