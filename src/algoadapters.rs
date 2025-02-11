use geo::raster::algo;

use crate::PythonDenseArray;

pub fn replace_value(raster: &mut PythonDenseArray, value: f64, new_value: f64) {
    match raster {
        PythonDenseArray::U8(r) => algo::replace_value(r, value as u8, new_value as u8),
        PythonDenseArray::U16(r) => algo::replace_value(r, value as u16, new_value as u16),
        PythonDenseArray::U32(r) => algo::replace_value(r, value as u32, new_value as u32),
        PythonDenseArray::U64(r) => algo::replace_value(r, value as u64, new_value as u64),
        PythonDenseArray::I8(r) => algo::replace_value(r, value as i8, new_value as i8),
        PythonDenseArray::I16(r) => algo::replace_value(r, value as i16, new_value as i16),
        PythonDenseArray::I32(r) => algo::replace_value(r, value as i32, new_value as i32),
        PythonDenseArray::I64(r) => algo::replace_value(r, value as i64, new_value as i64),
        PythonDenseArray::F32(r) => algo::replace_value(r, value as f32, new_value as f32),
        PythonDenseArray::F64(r) => algo::replace_value(r, value, new_value),
    }
}
