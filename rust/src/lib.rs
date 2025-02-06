use pyo3::prelude::*;

mod pyio;
mod pyraster;
pub(crate) mod pyutils;

#[pyfunction]
fn version() -> String {
    env!("CARGO_PKG_VERSION").to_string()
}

/// A Python module implemented in Rust.
#[pymodule]
fn geodynamix(m: &Bound<'_, PyModule>) -> PyResult<()> {
    pyo3_log::init();

    #[cfg(feature = "static_build")]
    {
        let gdx_data_dir = std::env::current_exe()?
            .parent()
            .expect("Unable to get the python env root directory")
            .join("share")
            .join("geodynamix");

        let gdal_config = geo::RuntimeConfiguration::builder().proj_db(&gdx_data_dir).build();
        gdal_config.apply().expect("Failed to configure GDAL");
    }

    m.add_class::<pyraster::RasterMetadata>()?;
    m.add_class::<pyraster::Raster>()?;
    m.add_function(wrap_pyfunction!(version, m)?)?;
    m.add_function(wrap_pyfunction!(pyio::read, m)?)?;
    m.add_function(wrap_pyfunction!(pyio::read_as, m)?)?;
    m.add_function(wrap_pyfunction!(pyio::write, m)?)?;
    Ok(())
}
