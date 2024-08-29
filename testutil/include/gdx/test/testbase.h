#include "gdx/config.h"

#ifdef GDX_ENABLE_SIMD
#include "gdx/denseraster.h"
#include "gdx/denserasterio.h"
#endif
#include "gdx/maskedraster.h"
#include "gdx/maskedrasterio.h"
#include "gdx/sparseraster.h"
#include "gdx/test/platformsupport.h"
#include "gdx/test/printsupport.h"
#include "gdx/test/rasterasserts.h"
#include "infra/test/containerasserts.h"

#include <doctest/doctest.h>

#include <limits>

namespace gdx::test {

using namespace doctest;
namespace gdal = inf::gdal;

static const uint32_t s_rows = 80;
static const uint32_t s_cols = 60;

template <template <typename> typename Raster, typename Value>
struct RasterValuePair
{
    using value_type = Value;
    using raster     = Raster<Value>;
};

template <template <typename> typename Raster>
struct RasterType
{
    template <typename T>
    using type = Raster<T>;
};

void compareMetaData(const RasterMetadata& expected, const RasterMetadata& actual);

#ifdef GDX_ENABLE_SIMD
#define RasterTypes RasterValuePair<MaskedRaster, int32_t>, \
                    RasterValuePair<MaskedRaster, int64_t>, \
                    RasterValuePair<MaskedRaster, float>,   \
                    RasterValuePair<MaskedRaster, double>,  \
                    RasterValuePair<DenseRaster, int32_t>,  \
                    RasterValuePair<DenseRaster, float>

#define RasterIntTypes RasterValuePair<MaskedRaster, int32_t>, \
                       RasterValuePair<MaskedRaster, int64_t>, \
                       RasterValuePair<DenseRaster, int32_t>,  \
                       RasterValuePair<DenseRaster, int64_t>

#define RasterFloatTypes RasterValuePair<MaskedRaster, float>,  \
                         RasterValuePair<MaskedRaster, double>, \
                         RasterValuePair<DenseRaster, float>,   \
                         RasterValuePair<DenseRaster, double>

#define UnspecializedRasterTypes RasterType<MaskedRaster>, \
                                 RasterType<DenseRaster>
#else
#define RasterTypes RasterValuePair<MaskedRaster, int32_t>, \
                    RasterValuePair<MaskedRaster, int64_t>, \
                    RasterValuePair<MaskedRaster, float>,   \
                    RasterValuePair<MaskedRaster, double>

#define RasterIntTypes RasterValuePair<MaskedRaster, int32_t>, \
                       RasterValuePair<MaskedRaster, int64_t>

#define RasterFloatTypes RasterValuePair<MaskedRaster, float>, \
                         RasterValuePair<MaskedRaster, double>

#define UnspecializedRasterTypes RasterType<MaskedRaster>
#endif

}

TYPE_TO_STRING(gdx::test::RasterValuePair<gdx::MaskedRaster, int32_t>);
TYPE_TO_STRING(gdx::test::RasterValuePair<gdx::MaskedRaster, int64_t>);
TYPE_TO_STRING(gdx::test::RasterValuePair<gdx::MaskedRaster, float>);
TYPE_TO_STRING(gdx::test::RasterValuePair<gdx::MaskedRaster, double>);
TYPE_TO_STRING(gdx::test::RasterType<gdx::MaskedRaster>);
#ifdef GDX_ENABLE_SIMD
TYPE_TO_STRING(gdx::test::RasterValuePair<gdx::DenseRaster, int32_t>);
TYPE_TO_STRING(gdx::test::RasterValuePair<gdx::DenseRaster, int64_t>);
TYPE_TO_STRING(gdx::test::RasterValuePair<gdx::DenseRaster, float>);
TYPE_TO_STRING(gdx::test::RasterValuePair<gdx::DenseRaster, double>);
TYPE_TO_STRING(gdx::test::RasterType<gdx::DenseRaster>);
#endif
