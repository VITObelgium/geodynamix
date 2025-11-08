use geo::{ArrayInterop as _, ArrayNum, DenseArray};
use numpy::{PyArray2, PyArrayMethods, PyUntypedArrayMethods};
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
    if let Ok(ndarray) = ndarray.downcast::<PyArray2<u8>>() {
        Ok(PythonDenseArray::U8(dense_array_from_ndarray(ndarray, meta)?).into())
    } else if let Ok(ndarray) = ndarray.downcast::<PyArray2<u16>>() {
        Ok(PythonDenseArray::U16(dense_array_from_ndarray(ndarray, meta)?).into())
    } else if let Ok(ndarray) = ndarray.downcast::<PyArray2<u32>>() {
        Ok(PythonDenseArray::U32(dense_array_from_ndarray(ndarray, meta)?).into())
    } else if let Ok(ndarray) = ndarray.downcast::<PyArray2<u64>>() {
        Ok(PythonDenseArray::U64(dense_array_from_ndarray(ndarray, meta)?).into())
    } else if let Ok(ndarray) = ndarray.downcast::<PyArray2<i8>>() {
        Ok(PythonDenseArray::I8(dense_array_from_ndarray(ndarray, meta)?).into())
    } else if let Ok(ndarray) = ndarray.downcast::<PyArray2<i16>>() {
        Ok(PythonDenseArray::I16(dense_array_from_ndarray(ndarray, meta)?).into())
    } else if let Ok(ndarray) = ndarray.downcast::<PyArray2<i32>>() {
        Ok(PythonDenseArray::I32(dense_array_from_ndarray(ndarray, meta)?).into())
    } else if let Ok(ndarray) = ndarray.downcast::<PyArray2<i64>>() {
        Ok(PythonDenseArray::I64(dense_array_from_ndarray(ndarray, meta)?).into())
    } else if let Ok(ndarray) = ndarray.downcast::<PyArray2<f32>>() {
        Ok(PythonDenseArray::F32(dense_array_from_ndarray(ndarray, meta)?).into())
    } else if let Ok(ndarray) = ndarray.downcast::<PyArray2<f64>>() {
        Ok(PythonDenseArray::F64(dense_array_from_ndarray(ndarray, meta)?).into())
    } else if let Ok(ndarray) = ndarray.downcast::<PyArray2<bool>>() {
        let u8_array = ndarray.cast::<u8>(false)?;
        Ok(PythonDenseArray::U8(dense_array_from_ndarray(&u8_array, meta)?).into())
    } else {
        Err(PyValueError::new_err("Invalid numpy array type"))
    }
}

pub fn dense_array_from_ndarray<T: numpy::Element + ArrayNum>(
    ndarray: &Bound<'_, PyArray2<T>>,
    meta: &RasterMetadata,
) -> PyResult<DenseArray<T, RasterMetadata>> {
    if ndarray.shape()[0] != meta.get_rows() as usize
        || ndarray.shape()[1] != meta.get_cols() as usize
    {
        return Err(PyValueError::new_err(
            "Array shape does not match raster metadata",
        ));
    }

    Ok(DenseArray::<T, RasterMetadata>::new_init_nodata(
        meta.clone(),
        ndarray.to_vec()?,
    )?)
}
