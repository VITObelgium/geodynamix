.. geodynamix documentation master file, created by
   sphinx-quickstart on Fri Feb 14 20:44:15 2025.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

geodynamix
==========

.. toctree::
   :maxdepth: 1

   reference/index

Introduction
____________
Geodynamix is a Rust library with Python bindings that can be used for geospatial data processing.

It's main goal is to make working with raster data more ergonomic.
The main difference to other libraries like ``numpy`` in combination with ``rasterio`` is that it is designed to make calculations with rasters without having to worry about the nodata value.
All operations are nodata aware and handle nodata values in a way that makes sense for raster data.

IO support
----------
Geodynamix supports reading and writing raster data. It uses the ``gdal`` library for reading and writing raster data so it supports all the formats that ``gdal`` supports (See the `gdal drivers <https://gdal.org/en/stable/drivers/raster/index.html>`_ list).

.. code-block:: python
   :caption: Reading and writing raster data

   import geodynamix as gdx
   
   raster = gdx.read("path/to/raster.tif")
   gdx.write("path/to/output.tif", raster)

The data type of the internal raster will match the data type of the input raster. If a different data type is desired, it can be specified with the ``dtype`` parameter using the ``read_as`` function. The numpy data types are supported to specify the desired data type.

.. code-block:: python
   :caption: Reading raster data with data type conversion

   import geodynamix as gdx
   import numpy as np
   
   raster = gdx.read_as(np.int32, "path/to/raster.tif")
   assert raster.dtype == np.int32

Operations
----------
Geodynamix support all the basic raster operations like addition, subtraction, multiplication and division.

.. code-block:: python
   :caption: Basic raster operations

   import geodynamix as gdx
   
   raster1 = gdx.read("path/to/raster1.tif")
   raster2 = gdx.read("path/to/raster2.tif")
   result = raster1 + raster2

If ``raster1`` or ``raster2`` have a nodata value, the ``result`` will also have a nodata value at the same location.
In all of the examples ``#`` represents the nodata value.

.. grid:: 5
   :gutter: 2
   :class-container: custom-grid

   .. grid-item::
      
      .. table:: raster1

         =  =  =
         #  1  2
         3  #  5
         6  7  #
         =  =  =

   .. grid-item::
      :child-align: center

      .. math::

         \Large \textbf{+}


   .. grid-item::

      .. table:: raster2

         =  =  =
         8  7  #
         5  #  3
         #  1  0
         =  =  =

   .. grid-item::
      :child-align: center

      .. math::

         \Large \textbf{=}

   .. grid-item::

      .. table:: result

         =  =  =
         #  8  #
         8  #  8
         #  8  #
         =  =  =

Multiplication and subtraction behave in the same way. Division is a bit different, it also behaves the same as the other operations except division by zero alsp becomes a nodata value.

.. grid:: 5
   :gutter: 2
   :class-container: custom-grid

   .. grid-item::
      
      .. table:: raster1

         =  =  =
         #  2  2
         4  #  4
         6  6  #
         =  =  =

   .. grid-item::
      :child-align: center

      .. math::

         \Large \textbf{/}


   .. grid-item::

      .. table:: raster2

         =  =  =
         2  2  0
         2  0  2
         0  2  2
         =  =  =

   .. grid-item::
      :child-align: center

      .. math::

         \Large \textbf{=}

   .. grid-item::

      .. table:: result

         =  =  =
         #  1  #
         2  #  2
         #  3  #
         =  =  =