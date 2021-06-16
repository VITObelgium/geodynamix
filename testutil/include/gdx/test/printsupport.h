#pragma once

#include "gdx/config.h"

#ifdef GDX_ENABLE_SIMD
#include "gdx/denseraster.h"
#endif

#include "gdx/maskedraster.h"
#include "infra/cell.h"
#include "infra/test/printsupport.h"

namespace gdx {

#ifdef GDX_ENABLE_SIMD
template <typename T>
void PrintTo(const DenseRaster<T>& ras, std::ostream* o)
{
    if (ras.rows() < 10 && ras.cols() < 10) {
        *o << ras.metadata().to_string() << "\n"
           << ras.to_string();
    } else {
        *o << ras.metadata().to_string();
    }
}
#endif

template <typename T>
void PrintTo(const MaskedRaster<T>& ras, std::ostream* os)
{
    *os << ras.metadata().to_string() << "\n"
        << ras.eigen_const_data();
}
}
