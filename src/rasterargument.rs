use geo::{Error, Result};
use numpy::PyArrayDescr;
use pyo3::{
    prelude::*,
    types::{PyFloat, PyInt, PyString},
};
// use pyo3::types::{PyString, PyType};
use std::path::PathBuf;
use std::str::FromStr;

use crate::{io, Nodata, PythonDenseArray, Raster};

/// Wrapper struct that allows raster arguments to be instantiated from:
/// - An actual raster instance
/// - strings: this should be the path of the raster file
/// - numeric values: if there is a context raster provided from which we can determine the size and type
pub struct RasterArgument {
    ob: PyObject,
    raster: Option<Raster>,
}

impl RasterArgument {
    pub fn new(ob: Bound<'_, PyAny>) -> Self {
        RasterArgument {
            ob: ob.unbind(),
            raster: None,
        }
    }

    fn store_raster(&mut self, raster: Raster) -> Result<&PythonDenseArray> {
        self.raster = Some(raster);
        Ok(&self.raster.as_ref().unwrap().raster)
    }

    fn store_bound_raster(&mut self, raster: Bound<'_, Raster>) -> Result<&PythonDenseArray> {
        self.raster = Some(raster.extract()?);
        Ok(&self.raster.as_ref().unwrap().raster)
    }

    pub fn raster(&mut self, py: Python) -> Result<&PythonDenseArray> {
        let ob = self.ob.clone_ref(py).into_bound(py);

        if let Ok(raster) = ob.downcast::<Raster>() {
            self.store_bound_raster(raster.clone())
        } else if let Ok(path_string) = ob.downcast::<PyString>() {
            self.store_raster(RasterArgument::from_string(path_string)?)
        } else {
            return Err(Error::InvalidArgument(
                "Could not create raster from argument".into(),
            ));
        }
    }

    pub fn raster_compatible_with(
        &mut self,
        context: &PythonDenseArray,
        py: Python,
    ) -> Result<&PythonDenseArray> {
        let ob = self.ob.clone_ref(py).into_bound(py);

        if let Ok(raster) = ob.downcast::<Raster>() {
            self.store_bound_raster(raster.clone())
        } else if let Ok(path_string) = ob.downcast::<PyString>() {
            self.store_raster(RasterArgument::from_string(path_string)?)
        } else if let Ok(val) = ob.downcast::<PyFloat>() {
            self.store_raster(RasterArgument::from_float(val, context, py)?)
        } else if let Ok(val) = ob.downcast::<PyInt>() {
            self.store_raster(RasterArgument::from_int(val, context, py)?)
        } else if ob.downcast::<Nodata>().is_ok() {
            self.store_raster(RasterArgument::from_nodata(context, py)?)
        } else {
            return Err(Error::InvalidArgument(
                "Could not create raster from argument".into(),
            ));
        }
    }

    fn from_float(
        val: &Bound<'_, PyFloat>,
        context: &PythonDenseArray,
        py: Python,
    ) -> Result<Raster> {
        let georef = context.metadata();
        let fill_value = val.extract::<f64>()?;
        let dtype = match context {
            PythonDenseArray::U8(_) => Some(PyArrayDescr::of::<u8>(py).into_any()),
            PythonDenseArray::U16(_) => Some(PyArrayDescr::of::<u16>(py).into_any()),
            PythonDenseArray::U32(_) => Some(PyArrayDescr::of::<u32>(py).into_any()),
            PythonDenseArray::U64(_) => Some(PyArrayDescr::of::<u64>(py).into_any()),
            PythonDenseArray::I8(_) => Some(PyArrayDescr::of::<i8>(py).into_any()),
            PythonDenseArray::I16(_) => Some(PyArrayDescr::of::<i16>(py).into_any()),
            PythonDenseArray::I32(_) => Some(PyArrayDescr::of::<i32>(py).into_any()),
            PythonDenseArray::I64(_) => Some(PyArrayDescr::of::<i64>(py).into_any()),
            PythonDenseArray::F32(_) => Some(PyArrayDescr::of::<f32>(py).into_any()),
            PythonDenseArray::F64(_) => Some(PyArrayDescr::of::<f64>(py).into_any()),
        };

        Ok(Raster::new(Some(georef), dtype, Some(fill_value), 0, 0)?)
    }

    fn from_int(val: &Bound<'_, PyInt>, context: &PythonDenseArray, py: Python) -> Result<Raster> {
        let georef = context.metadata();
        let fill_value = val.extract::<i64>()?;
        let dtype = match context {
            PythonDenseArray::U8(_) => Some(PyArrayDescr::of::<u8>(py).into_any()),
            PythonDenseArray::U16(_) => Some(PyArrayDescr::of::<u16>(py).into_any()),
            PythonDenseArray::U32(_) => Some(PyArrayDescr::of::<u32>(py).into_any()),
            PythonDenseArray::U64(_) => Some(PyArrayDescr::of::<u64>(py).into_any()),
            PythonDenseArray::I8(_) => Some(PyArrayDescr::of::<i8>(py).into_any()),
            PythonDenseArray::I16(_) => Some(PyArrayDescr::of::<i16>(py).into_any()),
            PythonDenseArray::I32(_) => Some(PyArrayDescr::of::<i32>(py).into_any()),
            PythonDenseArray::I64(_) => Some(PyArrayDescr::of::<i64>(py).into_any()),
            PythonDenseArray::F32(_) => Some(PyArrayDescr::of::<f32>(py).into_any()),
            PythonDenseArray::F64(_) => Some(PyArrayDescr::of::<f64>(py).into_any()),
        };

        Ok(Raster::new(
            Some(georef),
            dtype,
            Some(fill_value as f64),
            0,
            0,
        )?)
    }

    fn from_nodata(context: &PythonDenseArray, py: Python) -> Result<Raster> {
        let georef = context.metadata();
        let dtype = match context {
            PythonDenseArray::U8(_) => Some(PyArrayDescr::of::<u8>(py).into_any()),
            PythonDenseArray::U16(_) => Some(PyArrayDescr::of::<u16>(py).into_any()),
            PythonDenseArray::U32(_) => Some(PyArrayDescr::of::<u32>(py).into_any()),
            PythonDenseArray::U64(_) => Some(PyArrayDescr::of::<u64>(py).into_any()),
            PythonDenseArray::I8(_) => Some(PyArrayDescr::of::<i8>(py).into_any()),
            PythonDenseArray::I16(_) => Some(PyArrayDescr::of::<i16>(py).into_any()),
            PythonDenseArray::I32(_) => Some(PyArrayDescr::of::<i32>(py).into_any()),
            PythonDenseArray::I64(_) => Some(PyArrayDescr::of::<i64>(py).into_any()),
            PythonDenseArray::F32(_) => Some(PyArrayDescr::of::<f32>(py).into_any()),
            PythonDenseArray::F64(_) => Some(PyArrayDescr::of::<f64>(py).into_any()),
        };

        Ok(Raster::new(Some(georef), dtype, None, 0, 0)?)
    }

    fn from_string(path_string: &Bound<'_, PyString>) -> Result<Raster> {
        let path = PathBuf::from_str(path_string.to_str()?).unwrap();
        if !path.exists() {
            return Err(Error::InvalidArgument(format!(
                "Provided raster path is not valid: {}",
                path.display()
            )));
        }

        Ok(io::read(path)?)
    }
}
