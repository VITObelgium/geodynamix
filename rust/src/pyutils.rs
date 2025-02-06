use numpy::{PyArray, PyArrayDescr, PyArrayMethods};
use pyo3::{exceptions::PyValueError, prelude::*, types::PyType};

use geo::{Array, ArrayDataType, ArrayNum, DenseArray};

use crate::pyraster::{PythonDenseArray, RasterMetadata};

fn convert_array<'py, T: ArrayNum<T> + numpy::Element>(
    py: Python<'py>,
    arr: &DenseArray<T, RasterMetadata>,
) -> PyResult<Bound<'py, PyAny>> {
    let np_array = PyArray::from_slice(py, arr.as_slice());
    let dimension = [arr.rows().count() as usize, arr.columns().count() as usize];
    Ok(np_array.reshape(dimension)?.into_any())
}

pub fn raster_array<'py>(py: Python<'py>, raster: &PythonDenseArray) -> PyResult<Bound<'py, PyAny>> {
    match raster {
        PythonDenseArray::F32(array) => convert_array(py, array),
        PythonDenseArray::F64(array) => convert_array(py, array),
        PythonDenseArray::I8(array) => convert_array(py, array),
        PythonDenseArray::U8(array) => convert_array(py, array),
        PythonDenseArray::I16(array) => convert_array(py, array),
        PythonDenseArray::U16(array) => convert_array(py, array),
        PythonDenseArray::I32(array) => convert_array(py, array),
        PythonDenseArray::U32(array) => convert_array(py, array),
        PythonDenseArray::I64(array) => convert_array(py, array),
        PythonDenseArray::U64(array) => convert_array(py, array),
    }
}

pub fn convert_data_type(data_type: &Bound<'_, PyAny>) -> PyResult<ArrayDataType> {
    if let Ok(dtype) = data_type.downcast_exact::<PyArrayDescr>() {
        convert_numpy_dtype(dtype)
    } else if let Ok(pytype) = data_type.downcast_exact::<PyType>() {
        convert_py_type(pytype)
    } else {
        Err(PyValueError::new_err(format!("Invalid data type: {:?}", data_type)))
    }
}

pub fn array_type_to_numpy_dtype(py: Python<'_>, array_type: ArrayDataType) -> Bound<'_, PyArrayDescr> {
    match array_type {
        ArrayDataType::Int8 => PyArrayDescr::of::<i8>(py),
        ArrayDataType::Uint8 => PyArrayDescr::of::<u8>(py),
        ArrayDataType::Int16 => PyArrayDescr::of::<i16>(py),
        ArrayDataType::Uint16 => PyArrayDescr::of::<u16>(py),
        ArrayDataType::Int32 => PyArrayDescr::of::<i32>(py),
        ArrayDataType::Uint32 => PyArrayDescr::of::<u32>(py),
        ArrayDataType::Int64 => PyArrayDescr::of::<i64>(py),
        ArrayDataType::Uint64 => PyArrayDescr::of::<u64>(py),
        ArrayDataType::Float32 => PyArrayDescr::of::<f32>(py),
        ArrayDataType::Float64 => PyArrayDescr::of::<f64>(py),
    }
}

fn convert_py_type(pytype: &Bound<'_, PyType>) -> PyResult<ArrayDataType> {
    match pytype.name()?.to_string_lossy().as_ref() {
        "int" => Ok(ArrayDataType::Int32),
        "float" => Ok(ArrayDataType::Float64),
        _ => Err(PyValueError::new_err(format!("Unsupported python data type: {:?}", pytype.name()))),
    }
}

fn convert_numpy_dtype(dtype: &Bound<'_, PyArrayDescr>) -> PyResult<ArrayDataType> {
    match dtype.str()?.to_string_lossy().as_ref() {
        "uint8" | "bool" => Ok(ArrayDataType::Uint8),
        "int16" => Ok(ArrayDataType::Int16),
        "uint16" => Ok(ArrayDataType::Uint16),
        // when saying int as datatype it is translated to int64 on 64bit linux
        // assume the client simply wants a 32bit integer
        "int32" | "int64" => Ok(ArrayDataType::Int32),
        "uint32" | "uint64" => Ok(ArrayDataType::Uint32),
        "float32" => Ok(ArrayDataType::Float32),
        "float64" => Ok(ArrayDataType::Float64),
        _ => Err(PyValueError::new_err(format!("Unsupported numpy data type: {}", dtype))),
    }
}
