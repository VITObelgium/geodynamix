use numpy::PyArrayDescr;
use pyo3::exceptions::PyBufferError;
use pyo3::ffi;
use pyo3::prelude::*;
use std::ffi::c_void;
use std::ffi::CString;
use std::ops::Add;
use std::ops::Div;
use std::ops::Mul;
use std::ops::Sub;

use geo::{AnyDenseArray, Columns, RasterSize, Rows};

use crate::{
    algoadapters,
    rasterargument::RasterArgument,
    utils::{self, array_type_to_numpy_dtype},
    PythonDenseArray, RasterMetadata,
};

#[pyclass(name = "raster", str)]
#[derive(Clone)]
pub struct Raster {
    pub raster: PythonDenseArray,
}

macro_rules! impl_raster_op {
    ( $op:ident, $lhs:expr, $rhs_object:ident ) => {{
        let py = $rhs_object.py();
        let mut rhs = RasterArgument::new($rhs_object);
        let rhs = rhs.raster_compatible_with(&$lhs, py)?;
        Ok((&$lhs).$op(rhs).into())
    }};
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

        Ok(Self { raster })
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
    pub fn array<'py>(slf: Bound<'py, Self>, py: Python<'py>) -> PyResult<Bound<'py, PyAny>> {
        //utils::raster_array(py, &slf.borrow().raster)
        utils::raster_buffer_array(py, slf)
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
        algoadapters::replace_value(&mut self.raster, value, new_value);
        Ok(())
    }

    pub fn write(&mut self, path: std::path::PathBuf) -> PyResult<()> {
        Ok(self.raster.write(&path)?)
    }

    pub fn __add__(&self, rhs_object: Bound<'_, PyAny>) -> PyResult<Raster> {
        impl_raster_op!(add, self.raster, rhs_object)
    }

    pub fn __sub__(&self, rhs_object: Bound<'_, PyAny>) -> PyResult<Raster> {
        impl_raster_op!(sub, self.raster, rhs_object)
    }

    pub fn __mul__(&self, rhs_object: Bound<'_, PyAny>) -> PyResult<Raster> {
        impl_raster_op!(mul, self.raster, rhs_object)
    }

    pub fn __truediv__(&self, rhs_object: Bound<'_, PyAny>) -> PyResult<Raster> {
        impl_raster_op!(div, self.raster, rhs_object)
    }
    /// # Safety
    /// This function is unsafe because it exposes a raw pointer to the Python buffer protocol.
    pub unsafe fn __getbuffer__(
        slf: Bound<'_, Self>,
        view: *mut ffi::Py_buffer,
        flags: std::ffi::c_int,
    ) -> PyResult<()> {
        fill_view_from_data(
            view,
            flags,
            slf.borrow().raster.raw_data_u8_slice(),
            slf.into_any(),
        )
    }

    /// # Safety
    /// This function is unsafe because it exposes a raw pointer to the Python buffer protocol.
    pub unsafe fn __releasebuffer__(&self, _buffer: *mut ffi::Py_buffer) {
        // No need to release, the raster still owns the data
    }
}

impl From<PythonDenseArray> for Raster {
    fn from(raster: PythonDenseArray) -> Self {
        Self { raster }
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

/// # Safety
///
/// `view` must be a valid pointer to `ffi::Py_buffer`, or null
/// `data` must outlive the Python lifetime of `owner` (i.e. data must be owned by owner, or data
/// must be static data)
unsafe fn fill_view_from_data(
    view: *mut ffi::Py_buffer,
    flags: std::ffi::c_int,
    data: &[u8],
    owner: Bound<'_, PyAny>,
) -> PyResult<()> {
    if view.is_null() {
        return Err(PyBufferError::new_err("View is null"));
    }

    (*view).obj = owner.into_ptr();
    (*view).buf = data.as_ptr() as *mut c_void;
    (*view).len = data.len() as isize;
    (*view).readonly = 0;
    (*view).itemsize = 1;

    (*view).format = if (flags & ffi::PyBUF_FORMAT) == ffi::PyBUF_FORMAT {
        let msg = CString::new("B").unwrap();
        msg.into_raw()
    } else {
        std::ptr::null_mut()
    };

    (*view).ndim = 1;
    (*view).shape = if (flags & ffi::PyBUF_ND) == ffi::PyBUF_ND {
        &mut (*view).len
    } else {
        std::ptr::null_mut()
    };

    (*view).strides = if (flags & ffi::PyBUF_STRIDES) == ffi::PyBUF_STRIDES {
        &mut (*view).itemsize
    } else {
        std::ptr::null_mut()
    };

    (*view).suboffsets = std::ptr::null_mut();
    (*view).internal = std::ptr::null_mut();

    Ok(())
}
