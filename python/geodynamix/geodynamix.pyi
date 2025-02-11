from os import PathLike
from typing import Optional, TypeAlias, Type
import numpy as np
from numpy.typing import DTypeLike

RasterLike: TypeAlias = (
    raster
    # a raster object
    | PathLike
    # the path to a raster file
    | float
    | int
    # a scalar value that will be exapnded to a raster with the same metadata
    | Type[nodata]
    # the nodata value that will be exapnded to a raster with the same metadata
)

class nodata:
    """
    A class representing a nodata value.
    - This class can be used to represent the nodata value for a given raster
    """
    def __init__(self) -> None: ...

class raster_metadata:
    """
    A class containing raster metadata.
    - The spatial extent and projection information
    - The nodata value

    :param rows: the number of rows in the raster
    :param cols: the number of columns in the raster
    :param nodata: the nodata value that is used when that raster is stored to disk
    :param cell_size: the size of a raster cell (can be m or degrees depending on the projection)
    :param xll: the x value of the lower left cell
    :param yll: the y value of the lower left cell
    """
    def __init__(
        self,
        rows: int = 0,
        cols: int = 0,
        nodata: Optional[float] = None,
        cell_size: float = 1.0,
        xll: float = 0.0,
        yll: float = 0.0,
    ) -> None: ...
    @property
    def rows(self) -> int: ...
    @property
    def cols(self) -> int: ...
    @property
    def nodata(self) -> Optional[float]: ...
    @nodata.setter
    def nodata(self, nodata: Optional[float]): ...
    @property
    def xll(self) -> float: ...
    @property
    def yll(self) -> float: ...
    @property
    def cell_size(self) -> float: ...
    @property
    def projected_epsg(self) -> Optional[int]: ...

class raster:
    """
    A class containing a geospatial raster.
    - This a dense raster where all the pixels are stored row by row in contiguous memory
    - The format of the pixel type can be chosen at creation
    - The nodata value is fixed per data Type
        - NAN for floating point types
        - The maximum value of the data type for unsigned types
        - The minimum value of the data type for signed types

    Keyword arguments:
    :param georef: an existing raster_metadata object (when provided rows, cols, nodata, cell_size, xll, yll should be omitted)
    :param dtype: the data type of the raster cells
    :param fill: the initial value of the raster cells (the raster will be filled with nodata if omitted)
    :param rows: the number of rows in the raster
    :param cols: the number of columns in the raster
    :param nodata: the nodata value that is used when that raster is stored to disk
    :param cell_size: the size of a raster cell (can be m or degrees depending on the projection)
    :param xll: the x value of the lower left cell
    :param xll: the y value of the lower left cell
    """
    def __init__(
        self,
        georef: Optional[raster_metadata] = None,
        dtype: DTypeLike = np.float32,
        fill: Optional[int | float] = None,
        rows: int = 0,
        cols: int = 0,
        nodata: Optional[float] = None,
        cell_size: float = 1.0,
        xll: float = 0.0,
        yll: float = 0.0,
    ) -> None: ...
    @property
    def metadata(self) -> raster_metadata: ...
    @property
    def dtype(self) -> np.dtype: ...
    @property
    def array(self) -> np.ndarray:
        """
        Returns the raster as a numpy array. Data is copied so changes to the array will not affect the raster.
        """
        ...
    @property
    def masked_array(self) -> np.ma.masked_array:
        """
        Returns the raster as a numpy masked array for easier nodata handling at the cost of additional memory usage. Data is copied so changes to the array will not affect the raster.
        """
        ...
    def replace_value(self, search: int | float, replace: int | float):
        """
        Replace all values in the raster that are equal to search with replace.
        """
        ...
    def astype(self, dtype: DTypeLike) -> raster:
        """
        Convert the raster to the requested data type.
        """
        ...

def raster_equal(lhs: RasterLike, rhs: RasterLike) -> bool:
    """
    Compare the two provided rasters for equaliy. Both the metadata and the pixel values are compared.
    The data types of the rasters must be the same.

    Parameters
    ----------
    lhs : RasterLike
    The left hand side raster.
    rhs : RasterLike
    The right hand side raster.
    """
    ...

def read(path: PathLike | str) -> raster:
    """
    Read a raster from disk in the data format of the file

    Parameters
    ----------
    path : PathLike | str
    The path to the raster file. Can be anything that is supported by gdal.
    """
    ...

def read_as(dtype: DTypeLike, path: PathLike | str) -> raster:
    """
    Read a raster from disk and convert the data format to the requested data type

    Parameters
    ----------
    dtype : DTypeLike
    The data type to wich the raster data will be converted.
    path : PathLike | str
    The path to the raster file. Can be anything that is supported by gdal.
    """
    ...

def write(raster: raster, path: PathLike | str) -> raster:
    """
    Write the raster raster to disk. The file file format is determined by the file extension.

    Parameters
    ----------
    raster : raster
    The raster to store.
    path : PathLike | str
    The storage location on disk.
    """
    ...

def raster_from_ndarray(array: np.ndarray, metadata: raster_metadata) -> raster:
    """
    Create a raster from the data in the ndarray and spatially reference it using the provided metadata.
    The ndarray will be copied in the raster so further changes to the ndarray will not affect the raster.
    The values in the ndarray that match the nodata value in the metadata will be set to nodata in the raster.

    Parameters
    ----------
    array : np.ndarray
    The numpy array.
    metadata : raster_metadata
    The raster metadata.
    """
    ...
