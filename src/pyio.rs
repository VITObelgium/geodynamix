use pyo3::prelude::*;

use crate::{
    pyraster::{PythonDenseArray, Raster},
    pyutils,
};

#[pyfunction]
pub fn read(path: std::path::PathBuf) -> PyResult<Raster> {
    Ok(PythonDenseArray::read(&path)?.into())
}

#[pyfunction]
pub fn read_as(dtype: &Bound<'_, PyAny>, path: std::path::PathBuf) -> PyResult<Raster> {
    let dtype = pyutils::convert_data_type(dtype)?;
    Ok(PythonDenseArray::read_as(dtype, &path)?.into())
}

#[pyfunction]
pub fn write(pyraster: &mut Raster, path: std::path::PathBuf) -> PyResult<()> {
    pyraster.write(path)
}
