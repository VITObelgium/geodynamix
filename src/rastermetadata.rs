use geo::{CellSize, Columns, Point, RasterSize, Rows};
use pyo3::prelude::*;

#[pyclass(name = "raster_metadata", str)]
#[derive(Clone, Debug, PartialEq)]
pub struct RasterMetadata(pub geo::GeoReference);

#[pymethods]
impl RasterMetadata {
    #[new]
    #[pyo3(signature = (rows = 0, cols = 0, nodata = None, cell_size = 1.0, xll = 0.0, yll = 0.0))]
    pub fn new(
        rows: i32,
        cols: i32,
        nodata: Option<f64>,
        cell_size: f64,
        xll: f64,
        yll: f64,
    ) -> Self {
        Self(geo::GeoReference::with_origin(
            "",
            RasterSize::with_rows_cols(Rows(rows), Columns(cols)),
            Point::new(xll, yll),
            CellSize::square(cell_size),
            nodata,
        ))
    }

    #[getter]
    pub fn get_projected_epsg(&self) -> Option<u32> {
        self.0.projected_epsg().map(|x| x.into())
    }

    #[getter]
    pub fn get_cols(&self) -> i32 {
        self.0.columns().count()
    }

    #[getter]
    pub fn get_rows(&self) -> i32 {
        self.0.rows().count()
    }

    #[getter]
    pub fn get_cell_size(&self) -> f64 {
        self.0.cell_size_x()
    }

    #[getter]
    pub fn get_xll(&self) -> f64 {
        self.0.bottom_left().x()
    }

    #[getter]
    pub fn get_yll(&self) -> f64 {
        self.0.bottom_left().y()
    }

    #[getter]
    pub fn get_nodata(&self) -> Option<f64> {
        self.0.nodata()
    }

    #[setter]
    pub fn set_nodata(&mut self, nodata: Option<f64>) {
        self.0.set_nodata(nodata);
    }
}

impl geo::ArrayMetadata for RasterMetadata {
    fn size(&self) -> geo::RasterSize {
        self.0.size()
    }

    fn nodata(&self) -> Option<f64> {
        self.0.nodata()
    }

    fn geo_reference(&self) -> geo::GeoReference {
        self.0.clone()
    }

    fn with_size(size: geo::RasterSize) -> Self {
        Self(geo::GeoReference::without_spatial_reference(size, None))
    }

    fn with_rows_cols(rows: geo::Rows, cols: geo::Columns) -> Self {
        Self(geo::GeoReference::without_spatial_reference(
            RasterSize::with_rows_cols(rows, cols),
            None,
        ))
    }

    fn with_geo_reference(georef: geo::GeoReference) -> Self {
        Self(georef)
    }
}

impl std::fmt::Display for RasterMetadata {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.0)
    }
}
