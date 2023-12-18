#include "gdx/algo/clusterid.h"
#include "gdx/test/testbase.h"
#include "testconfig.h"

#include <random>

namespace gdx::test {

using namespace inf;

TEST_CASE_TEMPLATE("cluster_id", TypeParam, UnspecializedRasterTypes)
{
    using FloatRaster = typename TypeParam::template type<float>;
    using ByteRaster  = typename TypeParam::template type<uint8_t>;
    using IntRaster   = typename TypeParam::template type<int32_t>;

    RasterMetadata meta(5, 4);

    SUBCASE("cluster_id")
    {
        FloatRaster raster(meta, std::vector<float>{1, 1, 1, 1, 1, 1, 2, 3, 3, 3, 3, 3, 1, 1, 5, 5, 1, 1, 5, 1});

        const IntRaster expected(meta, std::vector<int32_t>{
                                           1, 1, 1, 1,
                                           1, 1, 2, 3,
                                           3, 3, 3, 3,
                                           4, 4, 5, 5,
                                           4, 4, 5, 6});

        CHECK_RASTER_EQ(expected, cluster_id(raster, ClusterDiagonals::Exclude));
    }

    SUBCASE("cluster_id_border_values")
    {
        const IntRaster ras(meta, std::vector<int32_t>{1, 2, 3, 4, 2, 9, 9, 5, 3, 9, 9, 6, 4, 9, 9, 7, 5, 6, 7, 8});

        const IntRaster expected(meta, std::vector<int32_t>{
                                           1, 2, 3, 4,
                                           5, 6, 6, 7,
                                           8, 6, 6, 9,
                                           10, 6, 6, 11,
                                           12, 13, 14, 15});

        CHECK_RASTER_EQ(expected, cluster_id(ras, ClusterDiagonals::Exclude));
    }

    SUBCASE("fuzzy_cluster_id")
    {
        RasterMetadata meta(10, 10);
        meta.set_cell_size(100.0);

        const FloatRaster ras(meta, std::vector<float>{
                                        1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
                                        1, 1, 0, 1, 0, 0, 1, 0, 1, 0,
                                        1, 0, 0, 1, 0, 0, 0, 1, 0, 0,
                                        1, 0, 1, 1, 0, 0, 1, 0, 1, 0,
                                        1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
                                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
                                        1, 0, 1, 0, 1, 0, 1, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0});

        const IntRaster expected(ras.metadata(), std::vector<int32_t>{
                                                     1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
                                                     1, 1, 0, 1, 0, 0, 2, 0, 2, 0,
                                                     1, 0, 0, 1, 0, 0, 0, 2, 0, 0,
                                                     1, 0, 1, 1, 0, 0, 2, 0, 2, 0,
                                                     1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
                                                     0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
                                                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                     0, 0, 0, 0, 0, 0, 0, 0, 4, 0,
                                                     5, 0, 6, 0, 7, 0, 8, 0, 0, 0,
                                                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0});

        CHECK_RASTER_EQ(expected, fuzzy_cluster_id(ras, 1.42f * static_cast<float>(meta.cellSize.x)));
    }

    SUBCASE("cluster_id_with_obstacles")
    {
        IntRaster categories, expected;
        ByteRaster obstacles;
        gdx::read_raster(file::u8path(TEST_DATA_DIR) / "clusteridwithobstacles_categories.tif", categories);
        gdx::read_raster(file::u8path(TEST_DATA_DIR) / "clusteridwithobstacles_obstacles.tif", obstacles);
        gdx::read_raster(file::u8path(TEST_DATA_DIR) / "reference" / "clusteridwithobstacles.tif", expected);

        auto res = cluster_id_with_obstacles(categories, obstacles);

        CHECK_RASTER_EQ(expected, res);
    }
}
}
