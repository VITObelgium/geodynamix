use numpy::PyArrayDescr;
use pyo3::prelude::*;
use std::sync::Arc;

use geo::{AnyDenseArray, CellSize, Columns, Error, Point, RasterSize, Rows};

use crate::pyutils::{self, array_type_to_numpy_dtype};

pub type PythonDenseArray = AnyDenseArray<RasterMetadata>;

#[pyclass(name = "raster_metadata")]
#[derive(Clone, Debug)]
pub struct RasterMetadata(geo::GeoReference);

#[pymethods]
impl RasterMetadata {
    #[new]
    #[pyo3(signature = (rows = 0, cols = 0, nodata = None, cell_size = 1.0, xll = 0.0, yll = 0.0))]
    pub fn new(rows: i32, cols: i32, nodata: Option<f64>, cell_size: f64, xll: f64, yll: f64) -> Self {
        Self(geo::GeoReference::with_origin(
            "",
            RasterSize::with_rows_cols(Rows(rows), Columns(cols)),
            Point::new(xll, yll),
            CellSize::square(cell_size),
            nodata,
        ))
    }

    #[getter]
    pub fn get_projected_epsg(&self) -> Option<u32> {
        self.0.projected_epsg().map(|x| x.into())
    }

    #[getter]
    pub fn get_cols(&self) -> i32 {
        self.0.columns().count()
    }

    #[getter]
    pub fn get_rows(&self) -> i32 {
        self.0.rows().count()
    }

    #[getter]
    pub fn get_cell_size(&self) -> f64 {
        self.0.cell_size_x()
    }

    #[getter]
    pub fn get_xll(&self) -> f64 {
        self.0.bottom_left().x()
    }

    #[getter]
    pub fn get_yll(&self) -> f64 {
        self.0.bottom_left().y()
    }

    #[getter]
    pub fn get_nodata(&self) -> Option<f64> {
        self.0.nodata()
    }

    #[setter]
    pub fn set_nodata(&mut self, nodata: Option<f64>) {
        self.0.set_nodata(nodata);
    }
}

impl geo::ArrayMetadata for RasterMetadata {
    fn size(&self) -> geo::RasterSize {
        self.0.size()
    }

    fn geo_reference(&self) -> geo::GeoReference {
        self.0.clone()
    }

    fn with_size(size: geo::RasterSize) -> Self {
        Self(geo::GeoReference::without_spatial_reference(size, None))
    }

    fn with_rows_cols(rows: geo::Rows, cols: geo::Columns) -> Self {
        Self(geo::GeoReference::without_spatial_reference(
            RasterSize::with_rows_cols(rows, cols),
            None,
        ))
    }

    fn with_geo_reference(georef: geo::GeoReference) -> Self {
        Self(georef)
    }
}

#[pyclass(name = "raster")]
pub struct Raster {
    raster: Arc<PythonDenseArray>,
}

#[pymethods]
impl Raster {
    #[new]
    #[pyo3(signature = (georef = None, /, rows = 0, cols = 0, dtype = None, fill = None))]
    pub fn new(
        georef: Option<&RasterMetadata>,
        rows: i32,
        cols: i32,
        dtype: Option<Bound<'_, PyAny>>,
        fill: Option<f64>,
    ) -> PyResult<Self> {
        let meta = if let Some(georef) = georef {
            georef.to_owned()
        } else {
            let raster_size = RasterSize::with_rows_cols(Rows(rows), Columns(cols));
            RasterMetadata(geo::GeoReference::without_spatial_reference(raster_size, None))
        };

        let dtype = match dtype.as_ref() {
            Some(dtype) => pyutils::convert_data_type(dtype)?,
            None => geo::ArrayDataType::Float32,
        };

        let raster = AnyDenseArray::<RasterMetadata>::filled_with(fill, meta, dtype);

        Ok(Self { raster: Arc::new(raster) })
    }

    #[getter]
    pub fn get_metadata(&self) -> RasterMetadata {
        self.raster.metadata().clone()
    }

    #[getter]
    pub fn get_dtype<'py>(&self, py: Python<'py>) -> Bound<'py, PyArrayDescr> {
        array_type_to_numpy_dtype(py, self.raster.data_type())
    }

    #[getter]
    pub fn array<'py>(&self, py: Python<'py>) -> PyResult<Bound<'py, PyAny>> {
        pyutils::raster_array(py, &self.raster)
    }

    pub fn write(&mut self, path: std::path::PathBuf) -> PyResult<()> {
        let raster = Arc::get_mut(&mut self.raster).ok_or_else(|| Error::Runtime("Failed to get mutable reference".into()))?;
        Ok(raster.write(&path)?)
    }
}

impl From<PythonDenseArray> for Raster {
    fn from(raster: PythonDenseArray) -> Self {
        Self { raster: Arc::new(raster) }
    }
}
