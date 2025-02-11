use numpy::PyArrayDescr;
use pyo3::prelude::*;
use std::sync::Arc;

use geo::{AnyDenseArray, Columns, Error, RasterSize, Rows};

use crate::{
    algoadapters,
    rasterargument::RasterArgument,
    utils::{self, array_type_to_numpy_dtype},
    PythonDenseArray, RasterMetadata,
};

#[pyclass(name = "raster", str)]
#[derive(Clone)]
pub struct Raster {
    pub raster: Arc<PythonDenseArray>,
}

#[pymethods]
impl Raster {
    #[new]
    #[pyo3(signature = (georef = None, /, dtype = None, fill = None, rows = 0, cols = 0))]
    pub fn new(
        georef: Option<&RasterMetadata>,
        dtype: Option<Bound<'_, PyAny>>,
        fill: Option<f64>,
        rows: i32,
        cols: i32,
    ) -> PyResult<Self> {
        let meta = if let Some(georef) = georef {
            georef.to_owned()
        } else {
            let raster_size = RasterSize::with_rows_cols(Rows(rows), Columns(cols));
            RasterMetadata(geo::GeoReference::without_spatial_reference(
                raster_size,
                None,
            ))
        };

        let dtype = match dtype.as_ref() {
            Some(dtype) => utils::convert_data_type(dtype)?,
            None => geo::ArrayDataType::Float32,
        };

        let raster = AnyDenseArray::<RasterMetadata>::filled_with(fill, meta, dtype);

        Ok(Self {
            raster: Arc::new(raster),
        })
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
        utils::raster_array(py, &self.raster)
    }

    #[getter]
    pub fn masked_array<'py>(&self, py: Python<'py>) -> PyResult<Bound<'py, PyAny>> {
        utils::raster_masked_array(py, &self.raster)
    }

    pub fn astype(&mut self, dtype: Bound<'_, PyAny>) -> PyResult<Raster> {
        let dtype = utils::convert_data_type(&dtype)?;
        Ok(self.raster.cast(dtype).into())
    }

    pub fn replace_value(&mut self, value: f64, new_value: f64) -> PyResult<()> {
        let raster = Arc::get_mut(&mut self.raster)
            .ok_or_else(|| Error::Runtime("Failed to get mutable reference".into()))?;
        algoadapters::replace_value(raster, value, new_value);
        Ok(())
    }

    pub fn write(&mut self, path: std::path::PathBuf) -> PyResult<()> {
        let raster = Arc::get_mut(&mut self.raster)
            .ok_or_else(|| Error::Runtime("Failed to get mutable reference".into()))?;
        Ok(raster.write(&path)?)
    }
}

impl From<PythonDenseArray> for Raster {
    fn from(raster: PythonDenseArray) -> Self {
        Self {
            raster: Arc::new(raster),
        }
    }
}

impl std::fmt::Display for Raster {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let meta = self.raster.metadata();
        write!(
            f,
            "<gdx.raster {}x{} dtype={}>",
            meta.get_rows(),
            meta.get_cols(),
            self.raster.data_type()
        )
    }
}

#[pyfunction]
pub fn raster_equal(
    py: Python,
    raster1: Bound<'_, PyAny>,
    raster2: Bound<'_, PyAny>,
) -> PyResult<bool> {
    let mut raster1 = RasterArgument::new(raster1);
    let mut raster2 = RasterArgument::new(raster2);

    let array1 = raster1.raster(py)?;
    let array2 = raster2.raster_compatible_with(array1, py)?;

    Ok(array1 == array2)
}
