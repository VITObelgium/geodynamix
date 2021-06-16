#pragma once

#include "gdx/exception.h"
#include "gdx/log.h"

#include <algorithm>
#include <fstream>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace gdx {

enum class Operation
{
    Sum,
    Count,
    CountNonZero,
    Min,
    MinNonZero,
    Max,
    Average,
    AverageInCat1
};

template <template <typename> typename RasterType, typename TVal, typename TCat>
std::pair<std::vector<std::string> /*header*/, std::vector<std::string> /*line*/> // returning int's and doubles is difficult, because how to specify 'no-value' then?
table_row_impl(
    const RasterType<TVal>& ras,            // this is typically a float raster
    const RasterType<TCat>& categoryRaster, // this is typically a Byte or int16 raster
    Operation op)
{
    const auto rows = ras.rows();
    const auto cols = ras.cols();
    std::map<int, std::pair<double, int>> cat;

    if (op == Operation::AverageInCat1) {
        cat[1] = std::pair<double, int>(0, 0);
    }

    int countNodata = 0;
    std::set<int> catWithNodata;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int currentCategory = static_cast<int>(categoryRaster(r, c));
            auto currentValue   = ras(r, c);

            if (!categoryRaster.is_nodata(r, c)) {
                if (!ras.is_nodata(r, c)) {
                    auto iter = cat.find(currentCategory);

                    if (iter == cat.end()) {
                        if (op == Operation::Sum || op == Operation::Average || op == Operation::Count || op == Operation::Min || op == Operation::Max) {
                            cat.emplace(currentCategory, std::pair<double, int>(static_cast<double>(currentValue), 1));
                        } else if (op == Operation::CountNonZero) {
                            if (currentValue != 0) {
                                cat.emplace(currentCategory, std::pair<double, int>(static_cast<double>(currentValue), 1));
                            }
                        } else if (op == Operation::MinNonZero) {
                            if (currentValue != 0) {
                                cat.emplace(currentCategory, std::pair<double, int>(static_cast<double>(currentValue), 1));
                            }
                        } else if (op == Operation::AverageInCat1) {
                            if (currentCategory == 1) {
                                cat.emplace(currentCategory, std::pair<double, int>(static_cast<double>(currentValue), 1));
                            }
                        }
                    } else {
                        if (op == Operation::Sum || op == Operation::Average || op == Operation::Count) {
                            iter->second.first += currentValue;
                            ++iter->second.second;
                        } else if (op == Operation::CountNonZero) {
                            if (currentValue != 0) {
                                ++iter->second.second;
                            }
                        } else if (op == Operation::Min) {
                            if (iter->second.first > currentValue) {
                                iter->second.first = currentValue;
                            }
                        } else if (op == Operation::MinNonZero) {
                            if (currentValue != 0) {
                                if (iter->second.first > currentValue) {
                                    iter->second.first = currentValue;
                                }
                            }
                        } else if (op == Operation::Max) {
                            if (iter->second.first < currentValue) {
                                iter->second.first = currentValue;
                            }
                        } else if (op == Operation::AverageInCat1) {
                            if (currentCategory == 1) {
                                iter->second.first += currentValue;
                                ++iter->second.second;
                            }
                        } else {
                            throw RuntimeError("table_row : unsupported operation {}", int(op));
                        }
                    }
                } else if (ras.is_nodata(r, c)) {
                    ++countNodata;
                    catWithNodata.insert(currentCategory);
                    auto iter = cat.find(currentCategory);
                    if (iter == cat.end()) {
                        if (op != Operation::AverageInCat1) {
                            cat.emplace(currentCategory, std::make_pair<double, int>(0.0, 0));
                        }
                    }
                }
            }
        }
    }

    if (countNodata) {
        Log::warn("tablerow(xls,op,A,B): map A contains {} NODATA values where B is a normal nonzero value\nThese values of A are ignored in the output table", countNodata);
    }

    if (op != Operation::AverageInCat1) {
        for (auto& elem : catWithNodata) {
            if (cat.find(elem) == cat.end()) {
                cat.emplace(elem, std::pair<double, int>(std::numeric_limits<double>::quiet_NaN(), 1));
            }
        }
    }

    std::vector<std::string> header;
    if (true) {
        for (auto iter = cat.begin(); iter != cat.end(); ++iter) {
            header.push_back(std::to_string(iter->first));
        }
        if (cat.empty()) {
            header.push_back(""); // print an empty column, that prevents problems with automatic processing those reults
        }
    }

    std::vector<std::string> line;
    for (auto& [catKey, value] : cat) {
        (void)catKey;
        if (op == Operation::AverageInCat1) {
            if (value.second > 0 && !std::isnan(value.first)) {
                line.push_back(std::to_string(value.first / value.second));
            } else {
                line.push_back("");
            }
        } else {
            if (value.second > 0 && !std::isnan(value.first)) {
                if (op != Operation::Count && op != Operation::CountNonZero) {
                    line.push_back(std::to_string((op != Operation::Average ? value.first : value.first / value.second)));
                } else {
                    line.push_back(std::to_string(value.second));
                }
            } else {
                line.push_back("=NA()");
            }
        }
    }
    if (cat.empty()) {
        line.push_back(""); // print an empty column, that prevents problems with automatic processing those reults
    }

    return std::pair(header, line);
}

template <template <typename> typename RasterType, typename T, typename TCat>
void table_row(const RasterType<T>& ras, const RasterType<TCat>& categoryRaster, Operation op, const fs::path& outputPath, const std::string& label, bool append)
{
    auto result = table_row_impl(ras, categoryRaster, op);

    std::ofstream fs(outputPath, append ? std::ios_base::out | std::ios_base::app : std::ios_base::out);
    if (!fs.is_open()) {
        throw RuntimeError("Cannot create file {}", outputPath);
    }

    if (!append) {
        fs << "cat";
        for (auto& value : result.first) {
            fs << "\t" << value.c_str();
        }
        fs << "\n";
    }

    fs << label;
    for (auto& value : result.second) {
        fs << "\t" << value.c_str();
    }
    fs << "\n";
}

}
