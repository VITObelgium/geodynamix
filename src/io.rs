use pyo3::prelude::*;

use crate::{utils, PythonDenseArray, Raster};

#[pyfunction]
pub fn read(path: std::path::PathBuf) -> PyResult<Raster> {
    Ok(PythonDenseArray::read(&path)?.into())
}

#[pyfunction]
pub fn read_as(dtype: &Bound<'_, PyAny>, path: std::path::PathBuf) -> PyResult<Raster> {
    let dtype = utils::convert_data_type(dtype)?;
    Ok(PythonDenseArray::read_as(dtype, &path)?.into())
}

#[pyfunction]
pub fn write(pyraster: &mut Raster, path: std::path::PathBuf) -> PyResult<()> {
    pyraster.write(path)
}
