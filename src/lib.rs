use pyo3::prelude::*;
use std::path::PathBuf;

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
    configure_proj_data()?;

    m.add_class::<pyraster::RasterMetadata>()?;
    m.add_class::<pyraster::Raster>()?;
    m.add_function(wrap_pyfunction!(version, m)?)?;
    m.add_function(wrap_pyfunction!(pyio::read, m)?)?;
    m.add_function(wrap_pyfunction!(pyio::read_as, m)?)?;
    m.add_function(wrap_pyfunction!(pyio::write, m)?)?;
    Ok(())
}

#[cfg(feature = "static_build")]
fn configure_proj_data() -> PyResult<()> {
    // If we are using a static build, we are responsible for shipping the proj.db
    // So make sure gdal can find it
    let python_exe = std::env::current_exe()?;

    let mut gdx_data_dir = python_exe
        .parent()
        .expect("Unable to get the python env root directory");

    // If the python exe is in the bin directory, we need to go up one level to get the root
    // of the python env
    if gdx_data_dir.file_name().unwrap().to_str() == Some("bin") {
        gdx_data_dir = gdx_data_dir
            .parent()
            .expect("Unable to get the python bin dir parent");
    }

    let gdx_data_dir: PathBuf = gdx_data_dir.join("share").join("geodynamix");
    let gdal_config = geo::RuntimeConfiguration::builder()
        .proj_db(&gdx_data_dir)
        .build();
    gdal_config.apply().expect("Failed to configure GDAL");

    Ok(())
}
