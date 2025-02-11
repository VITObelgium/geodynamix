use geo::{Array, ArrayNum, DenseArray};
use numpy::{PyArray2, PyArrayMethods};
use pyo3::{exceptions::PyValueError, prelude::*};

use crate::{utils, PythonDenseArray, Raster, RasterMetadata};

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

#[pyfunction]
pub fn raster_from_ndarray(ndarray: &Bound<'_, PyAny>, meta: &RasterMetadata) -> PyResult<Raster> {
    if let Ok(ndarray) = ndarray.downcast_exact::<PyArray2<f32>>() {
        Ok(PythonDenseArray::F32(dense_array_from_ndarray(ndarray, meta)?).into())
    } else if let Ok(ndarray) = ndarray.downcast_exact::<PyArray2<f64>>() {
        Ok(PythonDenseArray::F64(dense_array_from_ndarray(ndarray, meta)?).into())
    } else {
        Err(PyValueError::new_err("Invalid numpy array type"))
    }
}

pub fn dense_array_from_ndarray<T: numpy::Element + ArrayNum<T>>(
    ndarray: &Bound<'_, PyArray2<T>>,
    meta: &RasterMetadata,
) -> PyResult<DenseArray<T, RasterMetadata>> {
    Ok(DenseArray::<T, RasterMetadata>::new_process_nodata(
        meta.clone(),
        ndarray.to_vec()?,
    ))
}
