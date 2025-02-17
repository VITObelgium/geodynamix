use numpy::{PyArray, PyArrayDescr, PyArrayMethods};
use pyo3::{exceptions::PyValueError, prelude::*, types::IntoPyDict};

use geo::{raster::algo, Array, ArrayDataType, ArrayMetadata, ArrayNum, DenseArray};

use crate::{PythonDenseArray, Raster, RasterMetadata};

fn convert_array<'py, T: ArrayNum + numpy::Element>(
    py: Python<'py>,
    arr: &DenseArray<T, RasterMetadata>,
) -> PyResult<Bound<'py, PyAny>> {
    let np_array = PyArray::from_slice(py, arr.as_slice());
    let dimension = [arr.rows().count() as usize, arr.columns().count() as usize];
    Ok(np_array.reshape(dimension)?.into_any())
}

pub fn raster_array<'py>(
    py: Python<'py>,
    raster: &PythonDenseArray,
) -> PyResult<Bound<'py, PyAny>> {
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

pub fn raster_mask<'py>(py: Python<'py>, raster: &PythonDenseArray) -> PyResult<Bound<'py, PyAny>> {
    match raster {
        PythonDenseArray::F32(array) => convert_array(py, &algo::is_nodata(array)),
        PythonDenseArray::F64(array) => convert_array(py, &algo::is_nodata(array)),
        PythonDenseArray::I8(array) => convert_array(py, &algo::is_nodata(array)),
        PythonDenseArray::U8(array) => convert_array(py, &algo::is_nodata(array)),
        PythonDenseArray::I16(array) => convert_array(py, &algo::is_nodata(array)),
        PythonDenseArray::U16(array) => convert_array(py, &algo::is_nodata(array)),
        PythonDenseArray::I32(array) => convert_array(py, &algo::is_nodata(array)),
        PythonDenseArray::U32(array) => convert_array(py, &algo::is_nodata(array)),
        PythonDenseArray::I64(array) => convert_array(py, &algo::is_nodata(array)),
        PythonDenseArray::U64(array) => convert_array(py, &algo::is_nodata(array)),
    }
}

pub fn raster_buffer_array<'py>(
    py: Python<'py>,
    raster: Bound<'py, Raster>,
) -> PyResult<Bound<'py, PyAny>> {
    let np = PyModule::import(py, "numpy")?;

    let shape = [
        raster.borrow().raster.rows().count(),
        raster.borrow().raster.columns().count(),
    ];
    let dtype =
        array_type_to_numpy_dtype(py, raster.borrow().raster.data_type()).into_pyobject(py)?;
    let kwargs = vec![
        ("dtype", dtype.into_pyobject(py)?.into_any()),
        ("buffer", raster.into_pyobject(py)?.into_any()),
    ];

    np.getattr("ndarray")?
        .call((shape.into_pyobject(py)?,), Some(&kwargs.into_py_dict(py)?))
}

pub fn raster_masked_array<'py>(
    py: Python<'py>,
    raster: &PythonDenseArray,
) -> PyResult<Bound<'py, PyAny>> {
    let ma = PyModule::import(py, "numpy.ma")?;
    let data = raster_array(py, raster)?;
    let mask = raster_mask(py, raster)?;

    let nodata_fill_value = raster
        .metadata()
        .nodata()
        .unwrap_or(0.0)
        .into_pyobject(py)?
        .into_any();

    let kwargs = vec![("mask", mask), ("fill_value", nodata_fill_value)];

    ma.getattr("array")?
        .call((data,), Some(&kwargs.into_py_dict(py)?))
}

pub fn convert_data_type(data_type: &Bound<'_, PyAny>) -> PyResult<ArrayDataType> {
    if let Ok(dtype) = data_type.downcast_exact::<PyArrayDescr>() {
        // The provided type is a numpy data type object
        convert_numpy_dtype(dtype)
    } else {
        // The provided type is some python object, try to convert it to a numpy data type (e.g. int, float, 'B', ...)
        convert_numpy_dtype(&PyArrayDescr::new(data_type.py(), data_type)?)
    }
}

pub fn array_type_to_numpy_dtype(
    py: Python<'_>,
    array_type: ArrayDataType,
) -> Bound<'_, PyArrayDescr> {
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

fn convert_numpy_dtype(dtype: &Bound<'_, PyArrayDescr>) -> PyResult<ArrayDataType> {
    match dtype.str()?.to_string_lossy().as_ref() {
        "uint8" | "bool" => Ok(ArrayDataType::Uint8),
        "uint16" => Ok(ArrayDataType::Uint16),
        "uint32" => Ok(ArrayDataType::Uint32),
        "uint64" => Ok(ArrayDataType::Uint64),
        "int16" => Ok(ArrayDataType::Int16),
        "int32" => Ok(ArrayDataType::Int32),
        "int64" => Ok(ArrayDataType::Int64),
        "float32" => Ok(ArrayDataType::Float32),
        "float64" => Ok(ArrayDataType::Float64),
        _ => Err(PyValueError::new_err(format!(
            "Unsupported numpy data type: {}",
            dtype
        ))),
    }
}
