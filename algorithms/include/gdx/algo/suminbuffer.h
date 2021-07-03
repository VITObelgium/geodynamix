#pragma once

#include "gdx/algo/bufferstyle.h"
#include "gdx/algo/cast.h"
#include "gdx/algo/conversion.h"
#include "gdx/algo/nodata.h"

#include "gdx/cell.h"
#include "gdx/exception.h"
#include "infra/math.h"
#include "infra/span.h"

#include <vector>

namespace gdx {

// "summed area table" technique, see wikipedia
template <template <typename> typename RasterType, typename T>
RasterType<double> compute_integral_image(const RasterType<T>& image)
{
    const auto nRows = image.metadata().rows;
    const auto nCols = image.metadata().cols;

    RasterType<double> summedArea(image.metadata());

    summedArea(0, 0) = image(0, 0);

    for (int c = 1; c < nCols; ++c) {
        summedArea(0, c) = summedArea(0, c - 1) + image(0, c);
    }

    for (int r = 1; r < nRows; ++r) {
        summedArea(r, 0) = summedArea(r - 1, 0) + image(r, 0);
    }

    for (int r = 1; r < nRows; ++r) {
        double rowSum            = image(r, 0);
        auto* pImage             = &image(r, 1);
        auto* pPrevSummedAreaRow = &summedArea(r - 1, 1);
        auto* pSummedAreaRow     = &summedArea(r, 1);
        for (int c = 1; c < nCols; ++c) {
            rowSum += *pImage;                              // rowSum += image(r, c);
            *pSummedAreaRow = rowSum + *pPrevSummedAreaRow; // summedArea(r,c) = rowSum + summedArea(r-1,c);
            ++pImage;
            ++pPrevSummedAreaRow;
            ++pSummedAreaRow;
        }
    }

    return summedArea;
}

// summed circular area, using sliding window technique.  Much slower than integral image technique.
void compute_circle_border_offsets(const int radius /*cells*/,
    std::vector<Cell>& plusRight,
    std::vector<Cell>& minLeft,
    std::vector<Cell>& plusDown,
    std::vector<Cell>& minTop);

template <typename T>
long double compute_sum_in_cells(const int32_t row, const int32_t col, const int32_t rows, int32_t cols,
    std::span<const T> image, std::span<const Cell> cells)
{
    long double result = 0.0;

    for (const auto& cell : cells) {
        const int r = row + cell.r;
        const int c = col + cell.c;
        if (0 <= r && r < rows && 0 <= c && c < cols) {
            result += image[cols * r + c];
        }
    }

    return result;
}

// this routine also works for rc outside the integral image
template <typename TValue, template <typename> typename RasterType, typename TIntegralImageElem>
TValue compute_sum_within_rectangle(int r0, int c0, int r1, int c1, const RasterType<TIntegralImageElem>& I /*integral image*/, const int nRows, const int nCols)
{
    if (r0 > nRows - 1) r0 = nRows - 1;
    if (c0 > nCols - 1) c0 = nCols - 1;
    if (r1 > nRows - 1) r1 = nRows - 1;
    if (c1 > nCols - 1) c1 = nCols - 1;
    const TIntegralImageElem A = (r0 >= 0 && c0 >= 0 ? I(r0, c0) : 0);
    const TIntegralImageElem B = (r0 >= 0 && c1 >= 0 ? I(r0, c1) : 0);
    const TIntegralImageElem C = (r1 >= 0 && c1 >= 0 ? I(r1, c1) : 0);
    const TIntegralImageElem D = (r1 >= 0 && c0 >= 0 ? I(r1, c0) : 0);
    return TValue(C + A - B - D);
}

// this routine also works for rc outside the integral image
template <typename TValue, template <typename> typename RasterType, typename TIntegralImageElem>
TValue compute_sum_within_rectangle_around(const int r, const int c, const int radius, const RasterType<TIntegralImageElem>& I /*integral image*/, const int nRows, const int nCols)
{
    int r0 = r - radius - 1;
    int c0 = c - radius - 1;
    int r1 = r + radius;
    int c1 = c + radius;

    return compute_sum_within_rectangle<TValue>(r0, c0, r1, c1, I, nRows, nCols);
}

template <typename T, template <typename> typename RasterType, typename TIntegralImageElem>
T compute_sum_within_circle(const int32_t row, const int32_t col, const int32_t radius /*cells*/,
    int32_t rows, int32_t cols, std::span<const T> image,
    long double& prevSum, int& prevR, int& prevC,
    std::vector<Cell>& plusRight,
    std::vector<Cell>& minLeft,
    std::vector<Cell>& plusDown,
    std::vector<Cell>& minTop,
    const RasterType<TIntegralImageElem>& I /*integral image, used to quickly detect circles with no values */)
{
    long double thisSum = 0;

    if (compute_sum_within_rectangle_around<float>(row, col, radius, I, rows, cols) == 0) {
        // we are done
    } else if (prevC + 1 == col && prevR == row) {
        thisSum = prevSum;
        thisSum += compute_sum_in_cells(row, col, rows, cols, image, plusRight);
        thisSum -= compute_sum_in_cells(row, col, rows, cols, image, minLeft);
    } else if (prevR + 1 == row && prevC == col) {
        thisSum = prevSum;
        thisSum += compute_sum_in_cells(row, col, rows, cols, image, plusDown);
        thisSum -= compute_sum_in_cells(row, col, rows, cols, image, minTop);
    } else {
        const int rad2 = radius * radius;
        for (int dR = -radius; dR <= +radius; ++dR) {
            const int r = row + dR;
            if (0 <= r && r < rows) {
                const int dR2 = dR * dR;
                for (int dC = -radius; dC <= +radius; ++dC) {
                    const int c = col + dC;
                    if (0 <= c && c < cols) {
                        if (dR2 + dC * dC <= rad2) {
                            thisSum += image[cols * r + c];
                        }
                    }
                }
            }
        }
    }

    prevR   = row;
    prevC   = col;
    prevSum = thisSum;

    return static_cast<T>(thisSum);
}

template <template <typename> typename RasterType, typename T>
RasterType<T> sum_in_buffer(const RasterType<T>& ras, float radiusInMeter, BufferStyle bufferStyle)
{
    static_assert(!std::is_same_v<T, uint8_t>, "Bad overload chosen for uint8_t");

    auto src = ras.copy();
    replace_nodata_in_place(src, 0);

    float radiusInCells = radiusInMeter / static_cast<float>(ras.metadata().cellSize.x);
    int32_t radius      = int32_t(radiusInCells);
    if (bufferStyle == BufferStyle::Square) {
        // pi r^2 = (R+1+R)^2,  The R+1+R construct is because we want R=0 mean the cell itself, R=1 with one cell around it, etc.
        // then R = r.sqrt(pi)/2 - 1/2, To round x we use int(x + 0.5)
        // thus round(R) = int(r.sqrt(pi)/2 - 1/2 + 0.5) = int(r.sqrt(pi)/2).  On average this gives a 0% error.
        radius = int32_t(radius * std::sqrt(inf::math::pi) / 2.0);
    }

    RasterType<T> result(ras.metadata());

    const auto rows = ras.metadata().rows;
    const auto cols = ras.metadata().cols;

    long double prevSum = 0;
    int prevR           = -radius + 1;
    int prevC           = -radius + 1;

    // datastructure for RoundBuffer style
    std::vector<Cell> plusRight;
    std::vector<Cell> minLeft;
    std::vector<Cell> plusDown;
    std::vector<Cell> minTop;

    auto summedArea = compute_integral_image(src);
    if (bufferStyle == BufferStyle::Circular) {
        compute_circle_border_offsets(radius, plusRight, minLeft, plusDown, minTop);
    }

    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            if (bufferStyle == BufferStyle::Circular) {
                result(r, c) = compute_sum_within_circle<T>(r, c, radius, rows, cols, src, prevSum, prevR, prevC, plusRight, minLeft, plusDown, minTop, summedArea);
            } else {
                result(r, c) = compute_sum_within_rectangle_around<T>(r, c, radius, summedArea, rows, cols);
            }
        }
    }

    return result;
}

template <template <typename> typename RasterType>
inline RasterType<int32_t> sum_in_buffer(const RasterType<uint8_t>& ras, float radiusInMeter, BufferStyle bufferStyle)
{
    // cast uint8_t raster to int32_t, otherwise chances to overflow are too high
    auto intRaster = raster_cast<int32_t>(ras);
    intRaster.set_nodata(-1);
    return sum_in_buffer(intRaster, radiusInMeter, bufferStyle);
}

inline void compute_rect_on_map_around(const int row, const int col, const int radius, int& r0, int& c0, int& r1, int& c1, int nRows, int nCols)
{
    r0 = row - radius;
    if (r0 < 0) r0 = 0;
    r1 = row + radius;
    if (r1 > nRows - 1) r1 = nRows - 1;
    c0 = col - radius;
    if (c0 < 0) c0 = 0;
    c1 = col + radius;
    if (c1 > nCols - 1) c1 = nCols - 1;
}

template <typename T>
inline auto compute_d2(const T arow, const T acol, const T brow, const T bcol) -> T
{
    const T drow = arow - brow;
    const T dcol = acol - bcol;
    return drow * drow + dcol * dcol;
}

template <template <typename> typename RasterType, typename T>
RasterType<T> max_in_buffer(const RasterType<T>& ras, float radiusInMeter)
{
    float radiusInCells            = radiusInMeter / static_cast<float>(ras.metadata().cellSize.x);
    const long long radius2InCells = (long long)(ceil(radiusInCells * radiusInCells));
    const auto rows                = ras.metadata().rows;
    const auto cols                = ras.metadata().cols;

    RasterType<T> result(ras.metadata());
    result.fill(0);

    for (int32_t row = 0; row < rows; ++row) {
        for (int32_t col = 0; col < cols; ++col) {
            int r0, c0, r1, c1;
            compute_rect_on_map_around(row, col, int(ceil(radiusInCells)), r0, c0, r1, c1, rows, cols);
            T value = std::numeric_limits<T>::lowest();
            for (int r = r0; r <= r1; ++r) {
                for (int c = c0; c <= c1; ++c) {
                    if (compute_d2<long long>(row, col, r, c) <= radius2InCells) {
                        if (!ras.is_nodata(r, c) && value < ras(r, c)) {
                            value = ras(r, c);
                        }
                    }
                }
            }
            result(row, col) = value;
        }
    }

    return result;
}

}
