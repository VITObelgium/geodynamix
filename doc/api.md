# GDX public api

## GDX core

Contains several raster data structures and arithmetic operations on these data structures.
This allows us to perform 'nodata aware' calculations with rasters.

### API headers
>- gdx/denseraster.h
>- gdx/denserasterio.h
>- gdx/simd.h
>
>Raster type that stores the regular values and nodata values in the same array data structure
Uses simd instructions to speed up calculations

>- gdx/maskedraster.h
>- gdx/maskedrasterio.h
>- gdx/maskedrasteriterator.h
>
>Raster type that stores the regular values and nodata values in separate array data structure

>- gdx/sparseraster.h
>- gdx/sparserasterio.h
>- gdx/sparserasteriterator.h
>
>Raster type optimised for rasters that consist of mainly nodata values

>- gdx/raster.h
>
>Type erased version of the maskedraster type (the type of the raster values is not visible in the raster type)
>
>Needed for creating bindings to languages with dynamic typing (e.g. python)

>- gdx/rasterspan.h
>- gdx/rasterspanio.h
>
>Raster view adapter type to craete a "raster view" on top of regular c++ arrays. This allows a regular array to behave as a denseraster

>- gdx/rasterchecks.h
>- gdx/rastercompare.h
>- gdx/rasterdiff.h
>
>Utilities for comparing rasters for equality

>- gdx/rasterfwd.h
>
>Forward declarations of the templated raster types (allows compilation speedups if the full type info is not needed)

>- gdx/rasteriterator.h
>- gdx/rastercelliterator.h
>- gdx/rasterarea.h
>
>Raster iteration support. e.g. Iterating over a subrectangle or circular area of the raster. 


## GDX algo
Contains algorithms that operate on the gdx raster data structures. The algorithms are generic to accept any of the raster types. Popular algorithms from the c++ standard libraray are provided to work on rasters with awareness of nodata values.

### API headers
>- gdx/algo/accuflux.h
>
>Implementation of the pcraster accuflux functions
(Accumulated material flowing into downstream cell)

>- gdx/algo/addpoints.h
>
> Add points from a vector file to a raster

>- gdx/algo/aggregateandspreadmultiresolution.h
>- gdx/algo/aggregatemultiresolution.h
>- gdx/algo/arealweighteddistribution.h
>- gdx/algo/dasline.h
>- gdx/algo/dasmap.h
>- gdx/algo/dasmaplineairdistribution.h
>- gdx/algo/dasmapmultiresolution.h
>- gdx/algo/distribute.h
>- gdx/algo/multiresolution.h
>- gdx/algo/propdist.h
>- gdx/algo/weighteddistribution.h
>- gdx/algo/weightedproduct.h
>- gdx/algo/weightedpropdist.h
>- gdx/algo/weightedsum.h
>
>Algorithms for spreading values over a raster

>- gdx/algo/blurfilter.h
>
>Blurring of raster values

>- gdx/algo/cast.h
>
>Casting rasters to different data types

>- gdx/algo/choropleth.h
>
>Create a chropleth map of a raster (summing / averaging regions) based on an area raster

>- gdx/algo/category.h
>- gdx/algo/categoryio.h
>
>Category algorithms: operations on cells of the same category

>- gdx/algo/clip.h
>
>Raster clipping (all data outside of the polygon becomes nodata)

>- gdx/algo/clusterid.h
>- gdx/algo/clustersize.h
>- gdx/algo/clusterutils.h
>
>Clustering algorithms. Operation on adjacent cells with the same value (aka clusters)

>- gdx/algo/colormap.h
>
>Apply a colormap to a raster

>- gdx/algo/conditionals.h
>
>Conditional algorithms (if, if_then) 

>- gdx/algo/distance.h
>
>Distance calculations (e.g. min distances to cells with a specific value)

>- gdx/algo/filter.h
>
>Filtering algorithms. Apply sum/average on a rectangular/circular area around each cell

>- gdx/algo/logical.h
>
>all_of, any_of algorithms

>- gdx/algo/majorityfilter.h
>
> Calculating majorities, assigning the majority of the neigbouring cells to the cell value

>- gdx/algo/masking.h
>
>Masking algorithms: Checking if values are in a mask, erasing values outside of a mask

>- gdx/algo/mathoperations.h
>- gdx/algo/maximum.h
>- gdx/algo/minimum.h
>- gdx/algo/sum.h
>
>Math algorithms on rasters: min, max, abs, sin, cos, sum, ...

>- gdx/algo/nodata.h
>
>Algorithms for manipulating nodata: replace_nodata, turn_value_into_nodata, ...

>- gdx/algo/normalise.h
>
>Normalize algorithm

>- gdx/algo/random.h
>
>Fill a raster with random values

>- gdx/algo/rasterize.h
>- gdx/algo/rasterizelineantialiased.h
>- gdx/algo/shape.h
>
>Rasterization of shapefiles into a raster

>- gdx/algo/statistics.h
>
>Calculate statistics of a raster (cell count, nodata count, ...)

>- gdx/algo/reclass.h
>
>Mapping cells from one class to another

>- gdx/algo/suminbuffer.h
>
>Calculate the sum of values in a speficied buffer region

>- gdx/algo/tablerow.h
>
>Operations on values of the same class (count, sum, ...)

>- gdx/algo/voronoi.h
>
>Implementation of the voronoi algorithm

>- gdx/algo/algorithm.h
>
>Generic algorithm utilities for modifying raster data used by many of the algorithms
