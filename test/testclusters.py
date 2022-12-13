import os
import geodynamix as gdx
import unittest
import numpy as np


class GdxIntegrationTest(unittest.TestCase):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    data_dir = os.path.join(script_dir, 'mapdata', 'clusters')
    reference_dir = os.path.join(script_dir, 'referencedata', 'clusters')

    def map(self, map):
        return gdx.read(os.path.join(self.data_dir, map))

    def map_as(self, dtype, map):
        return gdx.read_as(dtype, os.path.join(self.data_dir, map))

    def ref(self, map):
        return gdx.read(os.path.join(self.reference_dir, map))

    def test_cluster_id(self):
        expected = self.ref('ref_cluster_id.asc')
        clustered = gdx.cluster_id(self.map('in.asc'))
        np.testing.assert_allclose(clustered.array, expected.array)

        expected = self.ref('ref_cluster_id_incldiag.asc')
        clustered = gdx.cluster_id(self.map('in.asc'), include_diagonal=True)
        np.testing.assert_allclose(clustered.array, expected.array)

        expected = self.ref('ref_cluster_id_excldiag.asc')
        clustered = gdx.cluster_id(self.map('in.asc'), include_diagonal=False)
        np.testing.assert_allclose(clustered.array, expected.array)

    def test_fuzzy_cluster_id(self):
        expected = self.ref('ref_fuzzy_cluster_id_142.asc')
        clustered = gdx.fuzzy_cluster_id(self.map('in.asc'), radius=142)
        np.testing.assert_allclose(clustered.array, expected.array)

        expected = self.ref('ref_fuzzy_cluster_id_200.asc')
        clustered = gdx.fuzzy_cluster_id(self.map('in.asc'), radius=200)
        np.testing.assert_array_equal(clustered.array, expected.array)

    def test_cluster_id_with_obstacles(self):
        expected = self.ref('ref_cluster_id_with_obstacles.asc')
        clustered = gdx.cluster_id_with_obstacles(self.map('in.asc'), self.map('obstacle.asc').astype('B'))
        np.testing.assert_array_equal(clustered.array, expected.array)

    def test_csum(self):
        expected = self.ref('ref_csum.asc')
        sum = gdx.csum(self.map('clusters.asc'), self.map('v.asc'))
        np.testing.assert_allclose(sum.array, expected.array, rtol=0.001)

    def test_csum_number_vs_raster(self):
        hungary = self.map_as(np.int32, 'HU_admin2_country_001.tif')
        ids = gdx.cluster_id(hungary, include_diagonal=True)

        ids_sum_number = gdx.csum(ids, 1)
        ids_sum_raster = gdx.csum(ids, gdx.raster(hungary.metadata, np.int32, fill=1))

        self.assertEqual(gdx.raster_sum(ids_sum_number), gdx.raster_sum(ids_sum_raster))
        np.testing.assert_array_equal(ids_sum_number.array, ids_sum_raster.array)

    def test_cmin(self):
        expected = self.ref('ref_cmin.asc')
        min = gdx.cmin(self.map('clusters.asc'), self.map('v.asc'))
        np.testing.assert_allclose(expected.array, min.array)

    def test_cmax(self):
        expected = self.ref('ref_cmax.asc')
        max = gdx.cmax(self.map('clusters.asc'), self.map('v.asc'))
        np.testing.assert_allclose(expected.array, max.array)

    def test_filter_or(self):
        expected = self.ref('ref_filter_or.asc')
        res = gdx.filter_or(self.map('clusters.asc'), self.map("conditie.asc"))
        np.testing.assert_array_equal(expected.array, res.array)

    def test_filter_and(self):
        expected = self.ref('ref_filter_and.asc')
        res = gdx.filter_and(self.map('clusters.asc'), self.map("conditie.asc"))
        np.testing.assert_array_equal(expected.array, res.array)

    def test_filter_not(self):
        expected = self.ref('ref_filter_not.asc')
        res = gdx.filter_not(self.map('clusters.asc'), self.map("conditie.asc"))
        np.testing.assert_array_equal(expected.array, res.array)

    def test_csum_in_buffer(self):
        expected = self.ref('ref_csuminbuffer.asc')
        res = gdx.csum_in_buffer(self.map('clusters.asc'), self.map('v.asc'), 200)
        np.testing.assert_allclose(res.array, expected.array, rtol=0.001)

    def test_sum_in_buffer(self):
        expected = self.ref('ref_suminbuffer_circular.asc')
        res = gdx.sum_in_buffer(self.map('clusters.asc'), 500)
        np.testing.assert_allclose(res.array, expected.array, rtol=0.001)
        res = gdx.sum_in_buffer(self.map('clusters.asc'), 500, buffer_style=gdx.buffer_style.circular)
        np.testing.assert_allclose(res.array, expected.array, rtol=0.001)

        expected = self.ref('ref_suminbuffer_square.asc')
        res = gdx.sum_in_buffer(self.map('clusters.asc'), 500, gdx.buffer_style.square)
        np.testing.assert_allclose(res.array, expected.array, rtol=0.001)


if __name__ == '__main__':
    unittest.main()
